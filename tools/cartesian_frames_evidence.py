#!/usr/bin/env python3
"""Input-bound Cartesian-frame observations, rational oracle and exact comparison."""
from __future__ import annotations
import argparse
import copy
import datetime as dt
import json
import math
import pathlib
import re
import sys
from fractions import Fraction as Q
from typing import Any
from experiment_runtime import (RuntimeErrorEvidence, read_json, write_json, sha256_file,
                                require_exact_keys)
EvidenceError = RuntimeErrorEvidence
sha256 = sha256_file
NON_CLAIMS = ["orientation", "handedness", "topology", "general_transform"]
CASE_KEYS = {"id","dimension","operation","operand_kind","direction","claim_category","inputs","non_claims"}
OPERATIONS = {"identity","map","translation","roundtrip","origin","basis","exponent",
              "construct","affine","difference","dot","norm","type_separation"}
HEX = re.compile(r"-?0x[0-9a-f]+(?:\.[0-9a-f]*)?p[+-][0-9]+")
CELLS = ["gcc-debug","gcc-release","clang-debug","clang-release"]
LIFECYCLE_LIMITATION = (
    "Report-only infrastructure may prepare and execute evidence; only an independent "
    "CF0-CF7 audit can qualify Cartesian Frames"
)

def require(ok: bool, reason: str) -> None:
    if not ok: raise EvidenceError(reason)

def encoded(x: str, *, nonfinite: bool = False, raw_zero: bool = False) -> str:
    require(isinstance(x,str), "hex value must be text")
    if x in {"nan","inf","-inf"}:
        require(nonfinite, "nonfinite claim result")
        return x
    require(HEX.fullmatch(x) is not None, "invalid hexadecimal input")
    n = float.fromhex(x)
    require(math.isfinite(n), "nonfinite hexadecimal input")
    if n == 0:
        return "-0x0p+0" if raw_zero and math.copysign(1,n)<0 else "0x0p+0"
    return n.hex()

def specification(case: dict[str,Any]) -> dict[str,Any]:
    require_exact_keys(case, CASE_KEYS, "case")
    c=copy.deepcopy(case);d=c["dimension"]
    require(type(d) is int and d in (2,3), "dimension differs")
    require(isinstance(c["id"],str) and re.fullmatch(r"[a-z0-9_-]+",c["id"]) is not None, "case id")
    require(c["operation"] in OPERATIONS and c["claim_category"]==c["operation"], "operation/category differs")
    require(c["operand_kind"] in ("point","vector") and c["direction"] in ("world","local"), "operand/direction differs")
    require(c["non_claims"]==NON_CLAIMS, "non-claims differ")
    require_exact_keys(c["inputs"], {"origin","basis","exponent","operand","auxiliary"}, "inputs")
    i=c["inputs"]
    require(type(i["exponent"]) is int and -1075<=i["exponent"]<=1024, "exponent schema")
    for key in ("origin","basis","operand","auxiliary"):
        require(isinstance(i[key],list) and len(i[key])==(d*d if key=="basis" else d), "input dimension differs")
        i[key]=[encoded(x,nonfinite=key=="basis",raw_zero=True) for x in i[key]]
    return c

def validate_profile(path: pathlib.Path) -> dict[str,Any]:
    p=read_json(path)
    require_exact_keys(p,{"schema_version","kind","status","repetitions_per_cell","cells","gates","cases","negative_cases","scale_exponents","exact_prerequisite_tests","equivalence","limitations","certificate_fields","baselines"},"profile")
    require(p["schema_version"]==2 and p["kind"]=="cartesian-frames-qualification-profile", "profile schema")
    require(p["status"]=="report_only_infrastructure" and p["repetitions_per_cell"]==3, "profile lifecycle")
    require(p["limitations"][0]==LIFECYCLE_LIMITATION, "profile lifecycle limitation")
    require(p["cells"]==[{"id":c,"compiler":"GCC 13" if c.startswith("gcc") else "Clang 18",
        "library":"libstdc++" if c.startswith("gcc") else "libc++",
        "build_type":"Debug" if c.endswith("debug") else "Release"} for c in CELLS], "matrix differs")
    require(p["gates"]==[f"CF{i}" for i in range(8)], "gates differ")
    require(p["scale_exponents"]==[-8,-1,0,1,8],"scale envelope differs")
    require(p["baselines"]=={"production_commit":"a32cf67b63a287596d9afbdd6f4b5fe0a80b7a59",
        "focused_commit":"e4b80982be59e8ef2abe614b47c376c2b583c588",
        "production_paths":["include/apmesh/core/geometry.hpp","src/core/geometry.cpp"],
        "focused_path":"tests/coordinate_frames.cpp"},"reviewed baseline authorities differ")
    tests=p["exact_prerequisite_tests"]
    require(isinstance(tests,list) and len(tests)==len(set(tests)) and bool(tests)
            and all(isinstance(n,str) and re.fullmatch(r"apmesh_core\.[a-z_]+",n) for n in tests),"prerequisite allowlist schema")
    require(p["equivalence"]=={"rule":"exact_hex","signed_zero_policy":"normalize_to_positive"}, "equivalence differs")
    require(p["certificate_fields"]==["schema_version","id","dimension","operation","operand_kind","direction","claim_category","inputs","non_claims","observed"], "claim fields differ")
    cases=[specification(c) for c in p["cases"]]
    require(len({c["id"] for c in cases})==len(cases), "duplicate profile case")
    # Enumerated IDs guard against silently removing a required analytic family.
    required=set()
    for d in (2,3):
        for kind in ("point","vector"):
            for direction in ("world","local"):
                required.update(f"d{d}_{name}_{kind}_{direction}" for name in ("identity","translation","reflection","roundtrip","overflow"))
                required.update(f"d{d}_permutation_{kind}_{direction}_{a}" for a in range(d))
                required.update(f"d{d}_scale_{kind}_{direction}_{k}" for k in (-8,-1,0,1,8))
        required.update(f"d{d}_{op}" for op in ("origin","basis","exponent","construct","affine","difference","dot","norm","type_separation","signed_zero","missing","duplicate","nonunit","matrix_nan","matrix_inf","matrix_-inf"))
        required.update(f"d{d}_boundary_{k}" for k in (-1075,-1024,-1023,1023,1024))
    require({c["id"] for c in cases}==required, "analytic coverage differs")
    require(p["negative_cases"]==NEGATIVES, "negative case set differs")
    p["cases"]=cases
    return p

def oracle(case: dict[str,Any]) -> dict[str,Any]:
    c=specification(case); i=c["inputs"];d=c["dimension"];k=i["exponent"];op=c["operation"]
    def err(e): return {"kind":"error","error":e,"values":None}
    if any(x in {"nan","inf","-inf"} for x in i["basis"]): return err("non_finite_input")
    B=[Q(float.fromhex(x)) for x in i["basis"]]
    if any(x not in (-1,0,1) for x in B) or any(sum(B[r*d+j]!=0 for j in range(d))!=1 for r in range(d)) or any(sum(B[r*d+j]!=0 for r in range(d))!=1 for j in range(d)):
        return err("invalid_frame")
    # IEEE binary64 positive finite scale AND reciprocal, derived from representability.
    def power(n): return Q(2**n) if n>=0 else Q(1,2**(-n))
    scale,inverse=power(k),power(-k)
    maximum=Q(float.fromhex("0x1.fffffffffffffp+1023"));minimum=Q(1,2**1074)
    if any(x<minimum or x>maximum for x in (scale,inverse)): return err("scale_out_of_range")
    O=[Q(float.fromhex(x)) for x in i["origin"]]
    x=[Q(float.fromhex(v)) for v in i["operand"]];y=[Q(float.fromhex(v)) for v in i["auxiliary"]]
    def add(a,b):return [u+v for u,v in zip(a,b)]
    def sub(a,b):return [u-v for u,v in zip(a,b)]
    def mapped(v,world,point,origin=None):
        origin=O if origin is None else origin
        v=sub(v,origin) if point and not world else v
        m=B if world else [B[j*d+r] for r in range(d) for j in range(d)]
        result=[sum(m[r*d+j]*v[j] for j in range(d))*(scale if world else inverse) for r in range(d)]
        return add(result,origin) if point and world else result
    world=c["direction"]=="world";point=c["operand_kind"]=="point"
    if op=="construct":result=O+B+[Q(k)]
    elif op=="origin":result=O
    elif op=="basis":result=B
    elif op=="exponent":result=[Q(k)]
    elif op=="type_separation":result=[Q(1)]
    elif op=="identity":result=x
    elif op=="map":result=mapped(x,world,point)
    elif op=="translation":result=mapped(x,world,point)+mapped(x,world,point,[Q(0)]*d)
    elif op=="roundtrip":
        middle=mapped(x,world,point)
        if any(abs(v)>maximum for v in middle):return err("non_finite_result")
        result=mapped(middle,not world,point)
    elif op=="affine":result=mapped(add(x,y),True,True)+add(mapped(x,True,True),mapped(y,True,False))
    elif op=="difference":result=mapped(sub(x,y),True,False)+sub(mapped(x,True,True),mapped(y,True,True))
    elif op=="dot":
        a,b=mapped(x,True,False),mapped(y,True,False)
        result=[sum(u*v for u,v in zip(a,b)),sum(u*v for u,v in zip(x,y))*scale*scale]
    elif op=="norm":
        def exact_sqrt(v):
            n,m=math.isqrt(v.numerator),math.isqrt(v.denominator)
            require(n*n==v.numerator and m*m==v.denominator,"fixture norm is not exact")
            return Q(n,m)
        a=mapped(x,True,False)
        result=[exact_sqrt(sum(v*v for v in a)),exact_sqrt(sum(v*v for v in x))*scale]
    else:raise EvidenceError("unsupported operation")
    if any(abs(v)>maximum for v in result):return err("non_finite_result")
    # No rounded fixture can enter this oracle unnoticed.
    for v in result:require(Q(float(v))==v,"oracle result requires rounding")
    return {"kind":"value","error":None,"values":[encoded(float(v).hex()) for v in result]}

def write_inputs(profile: dict[str,Any], path: pathlib.Path) -> None:
    lines=[]
    for c in profile["cases"]:
        i=c["inputs"]
        lines.append(" ".join([c["id"],str(c["dimension"]),c["operation"],c["operand_kind"],c["direction"],c["claim_category"],str(i["exponent"]),*i["origin"],*i["basis"],*i["operand"],*i["auxiliary"]]))
    path.parent.mkdir(parents=True,exist_ok=True)
    path.write_text("\n".join(lines)+"\n",encoding="ascii")

def validate_certificate(profile: dict[str,Any], path: pathlib.Path) -> dict[str,Any]:
    cert=read_json(path)
    require_exact_keys(cert,{"schema_version","kind","signed_zero_policy","pid","cases"},"certificate")
    require(cert["schema_version"]==2 and cert["kind"]=="cartesian-frames-certificate","certificate schema")
    require(type(cert["pid"]) is int and cert["pid"]>0,"certificate child PID")
    require(cert["signed_zero_policy"]=="normalize_to_positive","signed-zero policy")
    require(isinstance(cert["cases"],list) and len(cert["cases"])==len(profile["cases"]),"certificate coverage")
    out=[]
    for raw,planned in zip(cert["cases"],profile["cases"]):
        require_exact_keys(raw,CASE_KEYS|{"schema_version","observed"},"record")
        require(raw["schema_version"]==2,"record schema")
        c=specification({key:raw[key] for key in CASE_KEYS})
        require(c==specification(planned),"recorded inputs or case metadata differ from profile")
        obs=copy.deepcopy(raw["observed"]);require_exact_keys(obs,{"kind","error","values"},"observed")
        if obs["values"] is not None:
            require(isinstance(obs["values"],list),"observed values schema")
            obs["values"]=[encoded(v) for v in obs["values"]]
        expected=oracle(c)
        require(obs==expected,f"{c['id']}: independent formula disagrees")
        canonical=copy.deepcopy(c)
        for key in ("basis","origin","operand","auxiliary"):
            canonical["inputs"][key]=[encoded(v,nonfinite=key=="basis") for v in canonical["inputs"][key]]
        out.append({"schema_version":2,**canonical,"expected":expected,"observed":obs,
                    "comparison":{"rule":"exact_hex","signed_zero_policy":"normalize_to_positive","exact_match":True}})
    return {"schema_version":2,"kind":"cartesian-frames-semantic-certificate","cases":out}

def command_ok(record: dict[str,Any]) -> None:
    require(record.get("exit_code")==0 and record.get("timed_out") is False and record.get("launch_error") is None,"failed command")
    require(type(record.get("pid")) is int and record["pid"]>0,"command child PID")
    start=dt.datetime.fromisoformat(record["started_utc"]);end=dt.datetime.fromisoformat(record["ended_utc"])
    require(start.tzinfo is not None and end.tzinfo is not None and end>=start,"command timestamps")
    require(isinstance(record["elapsed_seconds"],(int,float)) and math.isfinite(record["elapsed_seconds"]) and record["elapsed_seconds"]>=0 and abs((end-start).total_seconds()-record["elapsed_seconds"])<1.01,"command duration")

def compare(profile: dict[str,Any], index_path: pathlib.Path, report_path: pathlib.Path | None) -> dict[str,Any]:
    index=read_json(index_path);require_exact_keys(index,{"candidate_commit","certificates"},"index")
    require(re.fullmatch("[0-9a-f]{40}",index["candidate_commit"]) is not None,"comparison revision")
    entries=index["certificates"]
    slots=[(c["id"],r) for c in profile["cells"] for r in range(1,4)]
    require([(e["cell"],e["repetition"]) for e in entries]==slots,"comparison fixed 4x3 slots")
    root=index_path.parent;projections=[];identities=set();paths=set()
    for e in entries:
        require_exact_keys(e,{"cell","repetition","path","sha256","command","executable_sha256","candidate_commit"},"slot")
        require(e["candidate_commit"]==index["candidate_commit"],"slot revision")
        path=(root/e["path"]).resolve()
        require(path.is_relative_to(root.resolve()) and str(path) not in paths,"certificate path alias/escape")
        paths.add(str(path));require(sha256(path)==e["sha256"],"certificate hash")
        record=e["command"];command_ok(record);cert=read_json(path)
        require(cert["pid"]==record["pid"],"certificate PID differs from executed process")
        identity=(record["pid"],record["started_utc"],record["id"])
        require(identity not in identities and record["id"]==f"{e['cell']}-certificate-{e['repetition']}","duplicate process slot")
        identities.add(identity)
        require(len(record["argv"])==3 and pathlib.Path(record["argv"][2]).name==path.name,"certificate command")
        require(re.fullmatch("[0-9a-f]{64}",e["executable_sha256"]) is not None,"executable identity")
        projections.append(validate_certificate(profile,path))
    require(all(p==projections[0] for p in projections),"cross-cell semantic difference")
    report={"schema_version":2,"kind":"cartesian-frames-comparison","status":"EVIDENCE_COLLECTED_PENDING_AUDIT","candidate_commit":index["candidate_commit"],"certificate_count":12,"equivalent":True,"within_cell":{c:True for c in CELLS},"semantic_projection":projections[0]}
    if report_path is not None:write_json(report_path,report)
    return report

NEGATIVES=["duplicate_case","forged_value","forged_error","missing_case","wrong_signed_zero_policy","mutated_input","mutated_dimension","extra_field","nonfinite_result"]
def mutated_certificate(original,name):
    c=copy.deepcopy(original)
    if name=="duplicate_case":c["cases"].append(copy.deepcopy(c["cases"][0]))
    elif name=="forged_value":c["cases"][0]["observed"]["values"][0]="0x1p+6"
    elif name=="forged_error":c["cases"][0]["observed"]["error"]="invalid_frame"
    elif name=="missing_case":c["cases"].pop()
    elif name=="wrong_signed_zero_policy":c["signed_zero_policy"]="preserve"
    elif name=="mutated_input":c["cases"][0]["inputs"]["origin"][0]="0x1p+0"
    elif name=="mutated_dimension":c["cases"][0]["dimension"]=3
    elif name=="extra_field":c["cases"][0]["extra"]=True
    elif name=="nonfinite_result":c["cases"][0]["observed"]["values"][0]="nan"
    else:raise EvidenceError("unknown negative")
    return c

def negative_self_check(profile: dict[str,Any], certificate_path: pathlib.Path, output: pathlib.Path) -> dict[str,Any]:
    original=read_json(certificate_path);validate_certificate(profile,certificate_path)
    rows=[]
    for name in NEGATIVES:
        c=mutated_certificate(original,name)
        path=output.parent/(name+".json");write_json(path,c)
        try:validate_certificate(profile,path)
        except EvidenceError as error:rows.append({"case":name,"rejected":True,"reason":str(error),"path":path.name,"sha256":sha256(path)})
        else:raise EvidenceError(f"negative accepted: {name}")
    result={"schema_version":2,"kind":"cartesian-frames-negative-outcomes","baseline_sha256":sha256(certificate_path),"validator_sha256":sha256(pathlib.Path(__file__)),"cases":rows}
    write_json(output,result);return result

def validate_negatives(profile,certificate,path):
    report=read_json(path);require(report["baseline_sha256"]==sha256(certificate),"negative baseline hash")
    require(report["validator_sha256"]==sha256(pathlib.Path(__file__)),"negative validator identity")
    require([r["case"] for r in report["cases"]]==NEGATIVES,"negative coverage")
    for r in report["cases"]:
        file=path.parent/r["path"];require(file.resolve().is_relative_to(path.parent.resolve()),"negative path escape")
        require(sha256(file)==r["sha256"] and r["rejected"] is True,"negative mutation hash")
        require(read_json(file)==mutated_certificate(read_json(certificate),r["case"]),"negative is not the declared baseline mutation")
        try:validate_certificate(profile,file)
        except EvidenceError as e:require(str(e)==r["reason"],"negative rejection reason")
        else:raise EvidenceError("retained negative accepted")

def main():
    p=argparse.ArgumentParser();p.add_argument("command",choices=["validate-profile","write-inputs","validate-certificate","compare","negative-self-check"])
    p.add_argument("--profile",type=pathlib.Path,required=True);p.add_argument("--certificate",type=pathlib.Path);p.add_argument("--output",type=pathlib.Path);p.add_argument("--index",type=pathlib.Path);p.add_argument("--report",type=pathlib.Path)
    a=p.parse_args()
    try:
        profile=validate_profile(a.profile)
        if a.command=="write-inputs":write_inputs(profile,a.output)
        elif a.command=="validate-certificate":
            result=validate_certificate(profile,a.certificate)
            if a.output:write_json(a.output,result)
        elif a.command=="compare":compare(profile,a.index,a.report)
        elif a.command=="negative-self-check":negative_self_check(profile,a.certificate,a.output)
    except (EvidenceError,KeyError,TypeError,ValueError,OverflowError) as e:print(f"Cartesian evidence: {e}",file=sys.stderr);return 1
    return 0
if __name__=="__main__":raise SystemExit(main())
