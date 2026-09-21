#!/usr/bin/env python3
"""Generate and validate fail-closed negative CGR evidence."""

from __future__ import annotations

import argparse
import copy
import importlib.util
import json
import pathlib
import tempfile
from typing import Any


class NegativeError(RuntimeError):
    pass


def fail(message: str) -> NegativeError:
    return NegativeError(message)


def load_validator(path: pathlib.Path):
    specification = importlib.util.spec_from_file_location("cgr_evidence", path)
    if specification is None or specification.loader is None:
        raise fail("CGR evidence validator could not be loaded")
    module = importlib.util.module_from_spec(specification)
    specification.loader.exec_module(module)
    return module


def read_json(path: pathlib.Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, dict):
        raise fail("JSON root must be an object")
    return value


def write_json(path: pathlib.Path, value: dict[str, Any]) -> None:
    path.write_text(
        json.dumps(value, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
        newline="\n",
    )


def expect_rejection(module, profile: dict[str, Any], certificate: dict[str, Any], root: pathlib.Path, identifier: str) -> dict[str, str]:
    path = root / f"{identifier}.json"
    write_json(path, certificate)
    try:
        module.validate_certificate(profile, path)
    except module.EvidenceError:
        return {
            "id": identifier,
            "result": "REJECTED",
            "evidence": "validator_rejection",
        }
    raise fail(f"forged certificate was accepted: {identifier}")


def observed_failure(
    cases: dict[str, Any],
    identifier: str,
    field: str,
    expected: str,
) -> dict[str, str]:
    failures = cases["failure_semantics"]
    if failures.get(field) != expected:
        raise fail(f"certificate failure semantic differs: {field}")
    return {
        "id": identifier,
        "result": "REJECTED",
        "evidence": f"certificate_error:{expected}",
    }


def generate(
    module,
    profile_path: pathlib.Path,
    certificate_path: pathlib.Path,
    output: pathlib.Path,
) -> None:
    profile = module.validate_profile(profile_path)
    module.validate_certificate(profile, certificate_path)
    base = read_json(certificate_path)
    projection = base["scientific_projection"]
    cases = projection["cases"]

    outcomes: list[dict[str, str]] = []

    with tempfile.TemporaryDirectory(prefix="apmesh-cgr-negatives-") as temporary:
        root = pathlib.Path(temporary)

        forged = copy.deepcopy(base)
        inventory = forged["scientific_projection"]["case_inventory"]
        forged["scientific_projection"]["case_inventory"] = inventory + [inventory[0]]
        outcomes.append(
            expect_rejection(module, profile, forged, root, "duplicate_case")
        )

        forged = copy.deepcopy(base)
        forged["scientific_projection"]["cases"]["regularity_regular"]["result"] = "degenerate"
        outcomes.append(
            expect_rejection(
                module,
                profile,
                forged,
                root,
                "forged_categorical_result",
            )
        )

        forged = copy.deepcopy(base)
        forged["scientific_projection"]["cases"]["total_length_parabola"]["lower"] = 100.0
        forged["scientific_projection"]["cases"]["total_length_parabola"]["upper"] = 101.0
        outcomes.append(
            expect_rejection(
                module,
                profile,
                forged,
                root,
                "forged_reference_containment",
            )
        )

        forged = copy.deepcopy(base)
        forged["scientific_projection"]["cases"]["inverse_line"]["lower_parameter"] = 0.75
        forged["scientific_projection"]["cases"]["inverse_line"]["upper_parameter"] = 0.25
        outcomes.append(
            expect_rejection(
                module,
                profile,
                forged,
                root,
                "forged_inverse_bracket_order",
            )
        )

        forged = copy.deepcopy(base)
        forged["scientific_projection"]["declared_claims"] = ["physical_sampling"]
        outcomes.append(
            expect_rejection(
                module,
                profile,
                forged,
                root,
                "undeclared_semantic_claim",
            )
        )

    outcomes.extend(
        [
            observed_failure(
                cases,
                "invalid_regularity_policy",
                "invalid_regularity_policy",
                "invalid_policy",
            ),
            observed_failure(
                cases,
                "invalid_length_policy",
                "invalid_length_policy",
                "invalid_policy",
            ),
            observed_failure(
                cases,
                "invalid_inverse_policy",
                "invalid_inverse_policy",
                "invalid_policy",
            ),
            observed_failure(
                cases,
                "non_finite_parameter",
                "non_finite_parameter",
                "non_finite_parameter",
            ),
            observed_failure(
                cases,
                "non_finite_inverse_target",
                "non_finite_inverse_target",
                "non_finite_target",
            ),
            observed_failure(
                cases,
                "inverse_target_outside_domain",
                "inverse_target_outside_domain",
                "target_out_of_domain",
            ),
            observed_failure(
                cases,
                "inverse_target_domain_indeterminate",
                "inverse_target_domain_indeterminate",
                "target_domain_indeterminate",
            ),
            observed_failure(
                cases,
                "inverse_regularity_not_certified",
                "inverse_regularity_not_certified",
                "regularity_not_certified",
            ),
        ]
    )

    expected_ids = profile["certificate_negative_cases"]
    if [item["id"] for item in outcomes] != expected_ids:
        raise fail("negative evidence inventory differs")

    write_json(
        output,
        {
            "schema_version": 1,
            "kind": "continuous-curve-geometry-negative-evidence",
            "certificate": certificate_path.name,
            "outcomes": outcomes,
        },
    )


def validate(profile_path: pathlib.Path, path: pathlib.Path, module) -> None:
    profile = module.validate_profile(profile_path)
    value = read_json(path)
    if set(value) != {"schema_version", "kind", "certificate", "outcomes"}:
        raise fail("negative evidence schema differs")
    if (
        value["schema_version"] != 1
        or value["kind"] != "continuous-curve-geometry-negative-evidence"
    ):
        raise fail("negative evidence identity differs")
    if not isinstance(value["certificate"], str) or not value["certificate"]:
        raise fail("negative evidence certificate identity differs")
    outcomes = value["outcomes"]
    if not isinstance(outcomes, list):
        raise fail("negative evidence outcomes differ")
    expected_ids = profile["certificate_negative_cases"]
    if [item.get("id") for item in outcomes] != expected_ids:
        raise fail("negative evidence inventory differs")
    for item in outcomes:
        if set(item) != {"id", "result", "evidence"}:
            raise fail("negative evidence outcome schema differs")
        if item["result"] != "REJECTED":
            raise fail("negative evidence contains a non-rejected result")
        if not isinstance(item["evidence"], str) or not item["evidence"]:
            raise fail("negative evidence proof differs")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--validator", required=True)
    parser.add_argument("--profile", required=True)
    commands = parser.add_subparsers(dest="command", required=True)

    generate_parser = commands.add_parser("generate")
    generate_parser.add_argument("--certificate", required=True)
    generate_parser.add_argument("--output", required=True)

    validate_parser = commands.add_parser("validate")
    validate_parser.add_argument("--outcomes", required=True)

    arguments = parser.parse_args()

    try:
        module = load_validator(pathlib.Path(arguments.validator))
        profile = pathlib.Path(arguments.profile)
        if arguments.command == "generate":
            generate(
                module,
                profile,
                pathlib.Path(arguments.certificate),
                pathlib.Path(arguments.output),
            )
        else:
            validate(profile, pathlib.Path(arguments.outcomes), module)
        return 0
    except (
        NegativeError,
        OSError,
        ValueError,
        json.JSONDecodeError,
    ) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    import sys

    raise SystemExit(main())
