#!/usr/bin/env python3
"""Focused durable-retention package contract without a formal campaign."""

from __future__ import annotations

import argparse
import importlib.util
import pathlib
import json
import sys
import tempfile


def load_tool(path: pathlib.Path):
    sys.path.insert(0, str(path.parent))
    spec = importlib.util.spec_from_file_location("rec_retention", path)
    if spec is None or spec.loader is None:
        raise RuntimeError("cannot load retention tool")
    module = importlib.util.module_from_spec(spec); sys.modules[spec.name] = module; spec.loader.exec_module(module)
    return module


def main() -> int:
    parser = argparse.ArgumentParser(); parser.add_argument("--tool", required=True)
    arguments = parser.parse_args(); tool = load_tool(pathlib.Path(arguments.tool))
    with tempfile.TemporaryDirectory() as temporary_directory:
        root = pathlib.Path(temporary_directory); campaign, destination = root / "campaign", root / "evidence" / "foundation" / "rec" / "synthetic"
        (campaign / "cells" / "one").mkdir(parents=True); (campaign / "cells" / "one" / "summary.json").write_text("{}\n", encoding="utf-8")
        (campaign / "build" / "CMakeFiles").mkdir(parents=True); (campaign / "build" / "CMakeFiles" / "object.o").write_bytes(b"object")
        tool.assemble(campaign, destination, "candidate-sha", "archive-sha")
        tool.verify(destination)
        retained = json.loads((destination / "retention-manifest.json").read_text(encoding="utf-8"))
        if not retained["files"][0]["source_path"].startswith(str(campaign)):
            raise RuntimeError("retention manifest did not preserve the external source path")
        if (destination / "build").exists() or not (destination / "cells" / "one" / "summary.json").is_file():
            raise RuntimeError("retention package did not filter scratch or retain evidence")
        (destination / "cells" / "one" / "summary.json").write_text("tampered\n", encoding="utf-8")
        try:
            tool.verify(destination)
        except tool.RuntimeErrorEvidence:
            pass
        else:
            raise RuntimeError("retention verifier accepted tampered evidence")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
