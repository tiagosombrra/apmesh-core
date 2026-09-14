#!/usr/bin/env python3
"""Exercise the actual retained failure verifier and candidate subprocess."""
import argparse
import copy
import pathlib
import shutil
import subprocess
import sys
import tempfile

def main():
    p=argparse.ArgumentParser();p.add_argument("--runner",required=True);a=p.parse_args()
    tools=pathlib.Path(a.runner).resolve().parent;sys.path.insert(0,str(tools))
    import run_cartesian_frames_qualification as r
    def rejects(fn):
        try:fn()
        except (r.RuntimeErrorEvidence,KeyError,TypeError,ValueError,OSError):return
        raise AssertionError("negative accepted")
    # A minimal disposable repository exercises detached verification with its own committed verifier.
    # It is a failed runner contract, never evidence of a scientific CF gate.
    with tempfile.TemporaryDirectory(prefix="cf-retention-contract-") as temp:
        root=pathlib.Path(temp);source=root/"source"
        subprocess.run(["git","clone","--shared","--no-hardlinks",str(tools.parent),str(source)],check=True,capture_output=True)
        for f in ("run_cartesian_frames_qualification.py","cartesian_frames_evidence.py","experiment_runtime.py"):
            shutil.copyfile(tools/f,source/"tools"/f)
        original=tools.parent
        profile=r.validate_profile(original/r.PROFILE)
        # Retained failure payload revalidation is semantic; it cannot depend on the original path.
        shutil.copyfile(original/r.PROFILE,source/r.PROFILE)
        for command in (["config","user.name","Contract Test"],["config","user.email","contract@example.invalid"],["add","tools","experiments/profiles/cartesian_frames.json"],["commit","-m","test fixture"]):
            r.git(source,*command)
        candidate=r.clean_candidate(source)
        control=root/"control";control.mkdir();evidence=root/"evidence"
        env={"tools":{n:{"path":"/usr/bin/"+n,"version":"test"} for n in ("cmake","ctest","ninja","python3","g++-13","clang++-18","ldd","git")}}
        plan={"cells":r.command_plan(profile,source,env)}
        m={"schema_version":3,"state":"PREPARED","execution_requested":False,"candidate":candidate,
           "working_directory":str(source),"control_root":str(control),"evidence_root":str(evidence),
           "environment":env,"plan":plan,"source_checks":r.source_checks(source),
           "inputs":r.input_identity(r.input_paths(source)),"limitations":profile["limitations"]}
        for name,value in (("prepared-manifest.json",m),("plan.json",plan),("source-checks.json",m["source_checks"]),
                           ("planned-inventory.json",r.planned_inventory(plan["cells"]))):r.write_json(control/name,value)
        shutil.copyfile(source/r.PROFILE,control/"profile.json")
        r.write_inputs(profile,control/"case-inputs.txt")
        r.write_state(control,"PREPARED",{"candidate_commit":candidate["commit"],"execution_requested":False});r.seal_preparation(control)
        r.execution_claim(control,evidence,m)
        r.terminal(control,evidence,m,"BLOCKED",[],[],{"stage":"before-first-command","type":"RuntimeErrorEvidence","message":"development failure","retry_attempted":False})
        r.seal_output(control,evidence,source)
        stage=evidence/"retained"
        def reseal():
            files=sorted(f for f in stage.rglob("*") if f.is_file() and f.name!="retention-manifest.json")
            r.write_json(stage/"retention-manifest.json",r.retention_manifest(stage,[(f,f) for f in files],candidate["commit"],None))
        r.verify_retention(stage)
        for path in ("evidence/failure.json","evidence/terminal-manifest.json","control/plan.json","detached-verification.json"):
            file=stage/path;old=file.read_bytes();file.write_bytes(old+b"x")
            rejects(lambda:r.verify_retention(stage));file.write_bytes(old)
        missing=stage/"evidence/failure.json";old=missing.read_bytes();missing.unlink()
        rejects(lambda:r.verify_retention(stage));missing.write_bytes(old)
        # Rehashing a semantically forged terminal still cannot turn this failure into success.
        t=stage/"evidence/terminal-manifest.json";old=t.read_bytes();v=r.read_json(t);v["state"]="EXECUTED_PENDING_AUDIT";r.write_json(t,v);reseal()
        rejects(lambda:r.verify_retention(stage));t.write_bytes(old);reseal()
        (stage/"unlisted.json").write_text("{}")
        rejects(lambda:r.verify_retention(stage));(stage/"unlisted.json").unlink()
        relocated=root/"relocated";shutil.copytree(stage,relocated);r.verify_retention(relocated)
        # An integrity failure in detached verification never publishes the staging tree.
        control2=root/"bad-control";shutil.copytree(control,control2);evidence2=root/"bad-evidence"
        for f in ("state.json","state-history.jsonl","preparation-seal.json"):(control2/f).unlink()
        m2=copy.deepcopy(m);m2["control_root"]=str(control2);m2["evidence_root"]=str(evidence2)
        m2["source_checks"]["focused"]["sha256"]="0"*64
        r.write_json(control2/"source-checks.json",m2["source_checks"])
        r.write_json(control2/"prepared-manifest.json",m2)
        r.write_state(control2,"PREPARED",{"candidate_commit":candidate["commit"],"execution_requested":False});r.seal_preparation(control2)
        r.execution_claim(control2,evidence2,m2)
        r.terminal(control2,evidence2,m2,"BLOCKED",[],[],{"stage":"test-failure","retry_attempted":False})
        rejects(lambda:r.seal_output(control2,evidence2,source))
        assert not (evidence2/"retained").exists()
        assert (evidence2/".retaining").is_dir() and r.read_json(evidence2/"retention-error.json")["retry_attempted"] is False
    print("PASS: real candidate verifier, failure retention, hash/semantic mutations and relocation")
    return 0
if __name__=="__main__":raise SystemExit(main())
