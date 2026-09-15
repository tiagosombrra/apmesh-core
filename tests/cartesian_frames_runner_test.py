#!/usr/bin/env python3
"""Focused lifecycle/selection contracts with disposable control data only."""
import argparse
import copy
import pathlib
import sys
import tempfile

def main():
    p=argparse.ArgumentParser()
    for n in ("runner","source-root","profile","protocol","validator","exporter"):p.add_argument("--"+n,required=True)
    a=p.parse_args();sys.path.insert(0,str(pathlib.Path(a.runner).parent))
    import run_cartesian_frames_qualification as r
    from cartesian_frames_evidence import write_inputs
    source=pathlib.Path(a.source_root).resolve();profile=r.validate_profile(pathlib.Path(a.profile))
    r.source_checks(source)
    def rejects(fn):
        try:fn()
        except (r.RuntimeErrorEvidence,KeyError,TypeError,ValueError,FileExistsError):return
        raise AssertionError("negative accepted")
    env={"tools":{n:{"path":"/usr/bin/"+n,"version":"development-test"} for n in ("cmake","ctest","ninja","python3","g++-13","clang++-18","ldd","git")}}
    plan={"cells":r.command_plan(profile,source,env)}
    assert len(plan["cells"])==4 and sum(c["stage"].startswith("certificate-") for cell in plan["cells"] for c in cell["commands"])==12
    good=r.json.dumps({"tests":[{"name":n,"command":["/usr/bin/true"]} for n in profile["exact_prerequisite_tests"]]})
    r.discovery(good,profile["exact_prerequisite_tests"])
    for tests in ([],profile["exact_prerequisite_tests"][:-1],profile["exact_prerequisite_tests"]+["extra"]):
        rejects(lambda:r.discovery(r.json.dumps({"tests":[{"name":n,"command":["true"]} for n in tests]}),profile["exact_prerequisite_tests"]))
    build=pathlib.Path(a.exporter).resolve().parent
    actual_env=r.environment_identity()
    cache,commands=r.validate_cache(build,build.name,actual_env,source)
    def compiler_cache(cache_type, value=None):
        rows=[]; replaced=False
        for row in cache.splitlines():
            if row.startswith("CMAKE_CXX_COMPILER:"):
                rows.append(f"CMAKE_CXX_COMPILER:{cache_type}="+(row.split("=",1)[1] if value is None else value))
                replaced=True
            else:rows.append(row)
        assert replaced
        return "\n".join(rows)+"\n"
    for cache_type in ("FILEPATH","STRING","UNINITIALIZED"):
        r.validate_build_metadata(compiler_cache(cache_type),commands,build,build.name,actual_env,source)
    rejects(lambda:r.validate_build_metadata(compiler_cache("STRING","/compiler/path-mismatch"),
        commands,build,build.name,actual_env,source))
    rejects(lambda:r.validate_build_metadata(cache.replace("CMAKE_BUILD_TYPE:STRING=","INVALID_BUILD_TYPE:STRING="),
        commands,build,build.name,actual_env,source))
    bad_commands=copy.deepcopy(commands);bad_commands[0]["file"]="/unrelated/source.cpp"
    rejects(lambda:r.validate_build_metadata(cache,bad_commands,build,build.name,actual_env,source))
    import subprocess
    # Discovery in a separate directory avoids overwriting the enclosing CTest's logs.
    with tempfile.TemporaryDirectory(prefix="cf-discovery-contract-") as discovery_dir:
        r.shutil.copyfile(build/"CTestTestfile.cmake",pathlib.Path(discovery_dir)/"CTestTestfile.cmake")
        selected=subprocess.run(["/usr/bin/ctest","--test-dir",discovery_dir,"--show-only=json-v1","-R",r.ctest_regex(profile["exact_prerequisite_tests"])],check=True,capture_output=True,text=True)
        observed=r.json.loads(selected.stdout)
        assert sorted(t["name"] for t in observed["tests"])==sorted(profile["exact_prerequisite_tests"])
        if all(t.get("command") for t in observed["tests"]):
            r.discovery(selected.stdout,profile["exact_prerequisite_tests"])
        else:
            # A focused build need not produce every prerequisite binary. The formal
            # runner builds all targets and must reject this incomplete discovery.
            rejects(lambda:r.discovery(selected.stdout,profile["exact_prerequisite_tests"]))
            print("Verified rejection of prerequisite discovery before full build")
    deps=subprocess.run(["/usr/bin/ninja","-C",str(build),"-t","deps"],check=True,capture_output=True,text=True)
    r.dependency_check(deps.stdout)
    rejects(lambda:r.dependency_check(deps.stdout.replace("/apmesh/math/linear_algebra.hpp","/missing.hpp")))
    runtime=subprocess.run(["/usr/bin/ldd",a.exporter],check=True,capture_output=True,text=True)
    r.runtime_paths(runtime.stdout,build.name)
    rejects(lambda:r.runtime_paths(runtime.stdout+"\nlibunexpected.so => /tmp/libunexpected.so (0x0)",build.name))
    with tempfile.TemporaryDirectory(prefix="cf-lifecycle-contract-") as temp:
        root=pathlib.Path(temp);control=root/"control";control.mkdir();evidence=root/"evidence"
        checks=r.source_checks(source)
        m={"schema_version":3,"state":"PREPARED","execution_requested":False,"candidate":{"commit":"1"*40},
           "working_directory":str(source),"control_root":str(control),"evidence_root":str(evidence),"environment":env,
           "plan":plan,"source_checks":checks}
        for name,obj in (("prepared-manifest.json",m),("plan.json",plan),("profile.json",profile),
                         ("source-checks.json",checks),("planned-inventory.json",r.planned_inventory(plan["cells"]))):
            r.write_json(control/name,obj)
        write_inputs(profile,control/"case-inputs.txt")
        r.write_state(control,"PREPARED",{"candidate_commit":"1"*40,"execution_requested":False});r.seal_preparation(control)
        r.validate_prepared(control)
        for name in (*r.CONTROL_FILES,"state.json","state-history.jsonl"):
            file=control/name;original=file.read_bytes()
            file.write_bytes(original+b"\n ")
            # JSON whitespace also changes sealed files; state is bound by parsed history.
            if name=="state.json":
                changed=r.read_json(file);changed["detail"]["candidate_commit"]="2"*40;r.write_json(file,changed)
            rejects(lambda:r.validate_prepared(control));file.write_bytes(original)
        r.execution_claim(control,evidence,m)
        rejects(lambda:r.execution_claim(control,evidence,m));rejects(lambda:r.validate_prepared(control))
        r.validate_prepared(control,unused=False)
        # A real failed child is retained, rather than replacing failure with a success mock.
        record=r.run_command([sys.executable,"-c","raise SystemExit(7)"],root,evidence/"logs","failing-child",10)
        assert record["exit_code"]==7;rejects(lambda:r.command_ok(record))
        obs=r.capture_record(record,evidence)
        r.terminal(control,evidence,m,"BLOCKED",[record],[obs],{"stage":"failing-child","retry_attempted":False})
        assert r.read_json(evidence/"terminal-manifest.json")["state"]=="BLOCKED"
        rejects(lambda:r.validate_prepared(control))
    print("PASS: fixed plan, discovery, immutable control/history, exclusive claim and real child failure")
    return 0
if __name__=="__main__":raise SystemExit(main())
