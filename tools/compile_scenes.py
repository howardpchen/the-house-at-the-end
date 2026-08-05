#!/usr/bin/env python3
"""Compile human-readable campaign scenes into bounded watch resources."""

from __future__ import annotations

import argparse
import json
import struct
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "content" / "scenes.json"
OUTPUT = ROOT / "resources" / "generated"

OPCODES = {
    "text": 1,
    "choice": 2,
    "if_flag": 3,
    "if_resource": 4,
    "cost": 5,
    "reward": 6,
    "set_flag": 7,
    "clear_flag": 8,
    "trust": 9,
    "goto": 10,
    "end": 11,
}


class ContentError(ValueError):
    pass


def op_size(op: dict) -> int:
    kind = op["op"]
    if kind == "text":
        return 3
    if kind == "choice":
        return 2 + 4 * len(op["options"])
    if kind == "if_flag":
        return 4
    if kind == "if_resource":
        return 6
    if kind in ("cost", "reward"):
        return 4
    if kind in ("set_flag", "clear_flag"):
        return 2
    if kind == "trust":
        return 3
    if kind == "goto":
        return 3
    if kind == "end":
        return 2
    raise ContentError(f"unknown opcode {kind!r}")


def compile_content(source: dict) -> tuple[bytes, bytes, str]:
    resources = source["resources"]
    guests = source["guests"]
    flags = source["flags"]
    strings: list[str] = []
    string_ids: dict[str, int] = {}

    def string_id(text: str, choice: bool = False) -> int:
        encoded = text.encode("utf-8")
        limit = 20 if choice else 80
        if len(text) > limit:
            raise ContentError(f"text exceeds {limit} characters: {text!r}")
        if len(encoded) > 255:
            raise ContentError(f"encoded text is too long: {text!r}")
        if text not in string_ids:
            string_ids[text] = len(strings)
            strings.append(text)
        return string_ids[text]

    scene_payloads: list[tuple[int, str, bytes]] = []
    seen_scene_ids: set[int] = set()
    for scene in source["scenes"]:
        scene_id = int(scene["id"])
        if scene_id in seen_scene_ids or not 0 < scene_id < 256:
            raise ContentError(f"invalid or duplicate scene id {scene_id}")
        seen_scene_ids.add(scene_id)
        labels: dict[str, int] = {}
        label_indices: dict[str, int] = {}
        offset = 0
        for op_index, op in enumerate(scene["ops"]):
            label = op.get("label")
            if label:
                if label in labels:
                    raise ContentError(f"duplicate label {label!r} in scene {scene_id}")
                labels[label] = offset
                label_indices[label] = op_index
            offset += op_size(op)
        if offset > 65535:
            raise ContentError(f"scene {scene_id} exceeds 16-bit offsets")

        operations = scene["ops"]

        def successors(index: int) -> list[int]:
            op = operations[index]
            kind = op["op"]
            if kind == "end":
                return []
            if kind == "choice":
                targets = []
                for option in op["options"]:
                    if option["target"] not in label_indices:
                        raise ContentError(f"missing target {option['target']!r}")
                    targets.append(label_indices[option["target"]])
                return targets
            if kind == "goto":
                if op["target"] not in label_indices:
                    raise ContentError(f"missing target {op['target']!r}")
                return [label_indices[op["target"]]]
            result = []
            if kind in ("if_flag", "if_resource"):
                if op["target"] not in label_indices:
                    raise ContentError(f"missing target {op['target']!r}")
                result.append(label_indices[op["target"]])
            if index + 1 >= len(operations):
                raise ContentError(f"scene {scene_id} falls off its final opcode")
            result.append(index + 1)
            return result

        visit_state = [0] * len(operations)

        def visit(index: int) -> None:
            if visit_state[index] == 1:
                raise ContentError(f"scene {scene_id} contains a loop")
            if visit_state[index] == 2:
                return
            visit_state[index] = 1
            for target_index in successors(index):
                visit(target_index)
            visit_state[index] = 2

        visit(0)
        unreachable = [index for index, state in enumerate(visit_state) if state == 0]
        if unreachable:
            raise ContentError(
                f"scene {scene_id} has unreachable op indexes {unreachable}"
            )

        code = bytearray()
        has_end = False
        for op in scene["ops"]:
            kind = op["op"]
            code.append(OPCODES[kind])
            if kind == "text":
                code += struct.pack("<H", string_id(op["text"]))
            elif kind == "choice":
                options = op["options"]
                if not 1 <= len(options) <= 3:
                    raise ContentError(f"scene {scene_id} choice count is invalid")
                code.append(len(options))
                for option in options:
                    target = labels.get(option["target"])
                    if target is None:
                        raise ContentError(f"missing target {option['target']!r}")
                    code += struct.pack("<HH", string_id(option["text"], True), target)
            elif kind == "if_flag":
                target = labels.get(op["target"])
                if target is None:
                    raise ContentError(f"missing target {op['target']!r}")
                code += struct.pack("<BH", flags[op["flag"]], target)
            elif kind == "if_resource":
                target = labels.get(op["target"])
                if target is None:
                    raise ContentError(f"missing target {op['target']!r}")
                code += struct.pack(
                    "<BHH", resources[op["resource"]], int(op["amount"]), target
                )
            elif kind in ("cost", "reward"):
                code += struct.pack(
                    "<BH", resources[op["resource"]], int(op["amount"])
                )
            elif kind in ("set_flag", "clear_flag"):
                code.append(flags[op["flag"]])
            elif kind == "trust":
                delta = int(op["delta"])
                if not -128 <= delta <= 127:
                    raise ContentError("trust delta exceeds one signed byte")
                code += struct.pack("<Bb", guests[op["guest"]], delta)
            elif kind == "goto":
                target = labels.get(op["target"])
                if target is None:
                    raise ContentError(f"missing target {op['target']!r}")
                code += struct.pack("<H", target)
            elif kind == "end":
                code.append(int(op["result"]))
                has_end = True
        if not has_end:
            raise ContentError(f"scene {scene_id} has no END")
        scene_payloads.append((scene_id, scene["name"], bytes(code)))

    scene_header_size = 5 + 5 * len(scene_payloads)
    scenes_blob = bytearray(b"HSC1" + bytes([len(scene_payloads)]))
    scene_offset = scene_header_size
    for scene_id, _name, code in scene_payloads:
        scenes_blob += struct.pack("<BHH", scene_id, scene_offset, len(code))
        scene_offset += len(code)
    for _scene_id, _name, code in scene_payloads:
        scenes_blob += code

    strings_header_size = 6 + 3 * len(strings)
    strings_blob = bytearray(b"HST1" + struct.pack("<H", len(strings)))
    string_offset = strings_header_size
    encoded_strings = [value.encode("utf-8") for value in strings]
    for encoded in encoded_strings:
        strings_blob += struct.pack("<HB", string_offset, len(encoded))
        string_offset += len(encoded)
    for encoded in encoded_strings:
        strings_blob += encoded

    report_lines = [
        "# Generated scene report",
        "",
        "Generated from `content/scenes.json`. Do not hand-edit binary outputs.",
        "",
        f"- Scenes: {len(scene_payloads)}",
        f"- Strings: {len(strings)}",
        f"- Scene bytes: {len(scenes_blob)}",
        f"- String bytes: {len(strings_blob)}",
        f"- Longest page: {max(map(len, strings), default=0)} characters",
        "",
        "## Scene table",
        "",
    ]
    report_lines.extend(
        f"- {scene_id}: {name} ({len(code)} bytecode bytes)"
        for scene_id, name, code in scene_payloads
    )
    return bytes(scenes_blob), bytes(strings_blob), "\n".join(report_lines) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true", help="fail if outputs are stale")
    args = parser.parse_args()
    source = json.loads(SOURCE.read_text(encoding="utf-8"))
    scenes, strings, report = compile_content(source)
    outputs = {
        OUTPUT / "scenes.bin": scenes,
        OUTPUT / "strings.bin": strings,
        OUTPUT / "REPORT.md": report.encode("utf-8"),
    }
    if args.check:
        stale = [str(path.relative_to(ROOT)) for path, value in outputs.items()
                 if not path.exists() or path.read_bytes() != value]
        if stale:
            raise SystemExit("stale generated content: " + ", ".join(stale))
        return 0
    OUTPUT.mkdir(parents=True, exist_ok=True)
    for path, value in outputs.items():
        path.write_bytes(value)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
