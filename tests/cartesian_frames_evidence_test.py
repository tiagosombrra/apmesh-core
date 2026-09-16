#!/usr/bin/env python3
"""Focused real-exporter checks; these observations do not qualify four toolchains."""
import argparse
import copy
import pathlib
import sys
import tempfile

def main():
    p=argparse.ArgumentParser()
    for n in ("exporter","profile","tool"):p.add_argument("--"+n,required=True)
    a=p.parse_args();sys.path.insert(0,str(pathlib.Path(a.tool).parent))
    import cartesian_frames_evidence as e
    from experiment_runtime import run_command
    profile=e.validate_profile(pathlib.Path(a.profile))
    def rejected(fn):
        try:fn()
        except (e.EvidenceError,KeyError,TypeError,ValueError):return
        raise AssertionError("negative unexpectedly accepted")
    with tempfile.TemporaryDirectory(prefix="cf-focused-evidence-") as temp:
        root=pathlib.Path(temp);inputs=root/"inputs.txt";e.write_inputs(profile,inputs)
        index={"candidate_commit":"1"*40,"certificates":[]}
        for cell in e.CELLS:
            for repeat in range(1,4):
                cert=root/f"{cell}-{repeat}.json"
                record=run_command([a.exporter,str(inputs),str(cert)],root,root/"logs",f"{cell}-certificate-{repeat}",30)
                e.command_ok(record);e.validate_certificate(profile,cert)
                index["certificates"].append({"cell":cell,"repetition":repeat,"path":cert.name,"sha256":e.sha256(cert),
                    "command":record,"candidate_commit":"1"*40,"executable_sha256":e.sha256(pathlib.Path(a.exporter))})
        path=root/"index.json";e.write_json(path,index)
        report=e.compare(profile,path,None);assert report["equivalent"]
        first=root/index["certificates"][0]["path"]
        e.negative_self_check(profile,first,root/"negatives.json");e.validate_negatives(profile,first,root/"negatives.json")
        for mode in ("alias","pid","binary","slot","commit"):
            bad=copy.deepcopy(index)
            if mode=="alias":bad["certificates"][1]["path"]=bad["certificates"][0]["path"]
            elif mode=="pid":bad["certificates"][0]["command"]["pid"]+=10000
            elif mode=="binary":bad["certificates"][0]["executable_sha256"]="bad"
            elif mode=="slot":bad["certificates"][0]["repetition"]=2
            elif mode=="commit":bad["certificates"][0]["candidate_commit"]="2"*40
            e.write_json(path,bad);rejected(lambda:e.compare(profile,path,None))
        # Independent oracle sanity checks guard against two implementations agreeing on zero.
        for d in (2,3):
            reflection=next(c for c in profile["cases"] if c["id"]==f"d{d}_reflection_vector_world")
            assert float.fromhex(e.oracle(reflection)["values"][0])==-1
            signed=next(c for c in profile["cases"] if c["id"]==f"d{d}_signed_zero")
            assert "-0x0p+0" in signed["inputs"]["basis"]
            assert e.oracle(signed)["kind"]=="value"
            for tag,expected in (("matrix_nan","non_finite_input"),("duplicate","invalid_frame"),
                                 ("boundary_-1024","scale_out_of_range")):
                case=next(c for c in profile["cases"] if c["id"]==f"d{d}_{tag}")
                assert e.oracle(case)["error"]==expected
        bad=copy.deepcopy(profile);bad["cases"].pop()
        e.write_json(root/"bad-profile.json",bad);rejected(lambda:e.validate_profile(root/"bad-profile.json"))
        bad=copy.deepcopy(profile);bad["limitations"][0]="Report-only infrastructure only; no manifest is prepared and no formal qualification is executed"
        e.write_json(root/"bad-profile-lifecycle.json",bad);rejected(lambda:e.validate_profile(root/"bad-profile-lifecycle.json"))
    print("PASS: 142 input-bound cases, 12 independent focused exporter processes, oracle and mutation checks")
    return 0
if __name__=="__main__":raise SystemExit(main())
