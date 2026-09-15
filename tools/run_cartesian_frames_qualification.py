#!/usr/bin/env python3
"""CF0-CF7 revision-bound lifecycle using the shared experiment runtime."""
from __future__ import annotations
import argparse
import copy
import hashlib
import json
import os
import pathlib
import platform
import re
import shutil
import shlex
import subprocess
import sys
import tempfile
from typing import Any
from experiment_runtime import (RuntimeErrorEvidence, canonical_json, clean_candidate, input_identity,
    read_json, retention_manifest, run_command, sha256_file, tool_version, utc_now,
    verify_input_identity, write_json, write_state)
from cartesian_frames_evidence import (validate_profile, write_inputs, validate_certificate,
    compare, negative_self_check, validate_negatives, command_ok, require, CELLS)
CONFIGURATIONS={c:("g++-13" if c.startswith("gcc") else "clang++-18", c.startswith("clang")) for c in CELLS}
REQUIRED_FOCUSED=["apmesh_core.coordinate_frames","apmesh_core.cartesian_frames_evidence",
                  "apmesh_core.cartesian_frames_runner","apmesh_core.cartesian_frames_retention"]
BASE="a32cf67b63a287596d9afbdd6f4b5fe0a80b7a59"
FOCUSED_BASE="e4b80982be59e8ef2abe614b47c376c2b583c588"
PROFILE="experiments/profiles/cartesian_frames.json"
PROTOCOL="docs/decisions/GEOMETRY_CARTESIAN_FRAMES_QUALIFICATION_PROTOCOL.md"
ENTRY="docs/decisions/GEOMETRY_TRANSFORMATIONS_COORDINATE_FRAMES_ENTRY_DECISION.md"
RUNTIME_EXECUTABLES=["apmesh_core_cartesian_frames_export","apmesh_core.coordinate_frames","apmesh_core_bootstrap_smoke","apmesh_core_numeric_contract","apmesh_core.geometry_primitives","apmesh_core.minimal_small_linear_algebra"]
CONTROL_FILES=("prepared-manifest.json","plan.json","planned-inventory.json","profile.json","source-checks.json","case-inputs.txt")
SUCCESS_FILES={"certificate-index.json","cross-cell-comparison.json","gate-summary.json","observed-inventories.json"}
ENV_KEYS=("PATH","CC","CXX","CXXFLAGS","CFLAGS","LDFLAGS","LD_LIBRARY_PATH","CPATH","CPLUS_INCLUDE_PATH","CMAKE_PREFIX_PATH","LANG","LC_ALL")

def git(source,*argv):
    r=subprocess.run(["git",*argv],cwd=source,capture_output=True,check=False)
    require(r.returncode==0,"Git command failed: "+" ".join(argv))
    return r.stdout

def source_checks(source: pathlib.Path) -> dict[str,Any]:
    rows=[]
    paths=git(source,"ls-tree","-r","--name-only",BASE,"--","include","src").decode().splitlines()
    require(paths==git(source,"ls-files","include","src").decode().splitlines(),"production inventory differs from reviewed scope")
    for path in paths:
        expected=hashlib.sha256(git(source,"show",f"{BASE}:{path}")).hexdigest()
        require(sha256_file(source/path)==expected,f"production baseline differs: {path}")
        rows.append({"path":path,"sha256":expected,"baseline":BASE})
    p="tests/coordinate_frames.cpp";expected=hashlib.sha256(git(source,"show",f"{FOCUSED_BASE}:{p}")).hexdigest()
    require(sha256_file(source/p)==expected,"focused contract differs from reviewed baseline")
    # Exact preservation is deliberately stronger than merely retaining the old case names.
    return {"production":rows,"focused":{"path":p,"sha256":expected,"baseline":FOCUSED_BASE}}

def input_paths(source: pathlib.Path) -> dict[str,pathlib.Path]:
    tracked=git(source,"ls-files").decode().splitlines()
    fixed=[PROFILE,PROTOCOL,ENTRY,"CMakeLists.txt","CMakePresets.json",
           "tools/cartesian_frames_evidence.py","tools/run_cartesian_frames_qualification.py",
           "tools/experiment_runtime.py","experiments/cartesian_frames_export.cpp",
           "tests/coordinate_frames.cpp","tests/cartesian_frames_evidence_test.py",
           "tests/cartesian_frames_runner_test.py","tests/cartesian_frames_retention_test.py",
           "docs/APMESH_CORE_STATE.md","docs/APMESH_CORE_ROADMAP.md"]
    fixed+= [p for p in tracked if p.startswith(("include/","src/","cmake/","docs/contracts/","docs/decisions/"))]
    return {p:source/p for p in sorted(set(fixed))}

def environment_identity():
    require(sys.platform=="linux","formal environment must be Linux/WSL")
    release=pathlib.Path("/etc/os-release").read_text()
    require('ID=ubuntu' in release and 'VERSION_ID="24.04"' in release,"formal OS must be Ubuntu 24.04")
    tools={n:tool_version(n) for n in ("cmake","ctest","ninja","python3","g++-13","clang++-18","ldd","git")}
    require(re.search(r"\b13\.",tools["g++-13"]["version"]) is not None,"GCC major differs")
    require(re.search(r"\b18\.",tools["clang++-18"]["version"]) is not None,"Clang major differs")
    return {"tools":tools,"os_release":release,"platform":platform.platform(),
            "environment":{k:os.environ.get(k) for k in ENV_KEYS}}

def ctest_regex(names):return "^("+"|".join(re.escape(n) for n in names)+")$"

def command_plan(profile,source,environment):
    t=environment["tools"];plans=[]
    for c in profile["cells"]:
        compiler,libcxx=CONFIGURATIONS[c["id"]];b="@EVIDENCE_ROOT@/cells/"+c["id"]+"/build"
        p={"cell":c["id"],"build_directory":b,"compile_commands":b+"/compile_commands.json",
           "cache":b+"/CMakeCache.txt","exporter":b+"/apmesh_core_cartesian_frames_export",
           "runtime_executables":RUNTIME_EXECUTABLES,"commands":[]}
        commands=[
            ("configure",[t["cmake"]["path"],"--preset",c["id"],"-S",str(source),"-B",b,
                          "-DCMAKE_CXX_COMPILER="+t[compiler]["path"]]),
            ("build",[t["cmake"]["path"],"--build",b]),
            ("focused-discovery",[t["ctest"]["path"],"--test-dir",b,"--show-only=json-v1","-R",ctest_regex(REQUIRED_FOCUSED)]),
            ("prerequisite-discovery",[t["ctest"]["path"],"--test-dir",b,"--show-only=json-v1","-R",ctest_regex(profile["exact_prerequisite_tests"])]),
            ("focused-ctest",[t["ctest"]["path"],"--test-dir",b,"--output-on-failure","--no-tests=error","-R",ctest_regex(REQUIRED_FOCUSED)]),
            ("prerequisite-ctest",[t["ctest"]["path"],"--test-dir",b,"--output-on-failure","--no-tests=error","-R",ctest_regex(profile["exact_prerequisite_tests"])]),
            ("object-dependencies",[t["ninja"]["path"],"-C",b,"-t","deps"])]
        commands += [("ldd-"+exe,[t["ldd"]["path"],b+"/"+exe]) for exe in RUNTIME_EXECUTABLES]
        commands += [(f"certificate-{r}",[p["exporter"],"@CONTROL_ROOT@/case-inputs.txt",f"@EVIDENCE_ROOT@/certificates/{c['id']}-{r}.json"]) for r in range(1,4)]
        p["commands"]=[{"id":c["id"]+"-"+stage,"stage":stage,"argv":argv,"cwd":str(source),
                        "timeout_seconds":600 if stage in ("configure","build") else 120} for stage,argv in commands]
        plans.append(p)
    return plans

def planned_inventory(plan):
    rows=[{"root":"control","path":p,"required_when":"always"} for p in (*CONTROL_FILES,"preparation-seal.json","state.json","state-history.jsonl")]
    for p in ("execution-claim.json","command-records.json","command-observations.json","terminal-manifest.json","artifact-snapshots.json"):
        rows.append({"root":"evidence","path":p,"required_when":"started"})
    for p in sorted(SUCCESS_FILES|{"limitations.json"}):
        if p not in [r["path"] for r in rows]:rows.append({"root":"evidence","path":p,"required_when":"success"})
    for c in plan:
        for r in range(1,4):
            rows += [{"root":"evidence","path":f"{kind}/{c['cell']}-{r}.json","required_when":"success"} for kind in ("certificates","semantic")]
        rows += [{"root":"evidence","path":f"cells/{c['cell']}/{p}","required_when":"success"} for p in ("compile_commands.json","cache.txt","dependencies.json","prerequisites.json","comparison.json","negative-outcomes.json")]
        from cartesian_frames_evidence import NEGATIVES
        rows += [{"root":"evidence","path":f"cells/{c['cell']}/{n}.json","required_when":"success"} for n in NEGATIVES]
    rows += [{"root":"evidence","path":p,"required_when":"failure"} for p in ("failure.json",)]
    rows += [{"root":"retained","path":p,"required_when":"sealed"} for p in ("retention-manifest.json","detached-verification.json")]
    return {"schema_version":1,"artifacts":rows}

def validate_roots(source,control,evidence):
    require(not control.exists() and not evidence.exists(),"roots must be new and nonexistent")
    require(not any(a.is_relative_to(b) for a,b in ((control,source),(evidence,source),(control,evidence),(evidence,control))),"roots overlap source or each other")
    require(control.parent.is_dir() and evidence.parent.is_dir(),"root parents must exist")

def seal_preparation(root):
    state=read_json(root/"state.json")
    value={"files":{p:sha256_file(root/p) for p in CONTROL_FILES},
           "initial_state":state,"initial_history_sha256":sha256_file(root/"state-history.jsonl")}
    write_json(root/"preparation-seal.json",value)

def validate_prepared(root,unused=True):
    m=read_json(root/"prepared-manifest.json");seal=read_json(root/"preparation-seal.json");state=read_json(root/"state.json")
    require(m["schema_version"]==3 and m["state"]=="PREPARED" and m["execution_requested"] is False,"PREPARED schema")
    require(seal["files"]=={p:sha256_file(root/p) for p in CONTROL_FILES},"preparation seal mismatch")
    history=(root/"state-history.jsonl").read_bytes().splitlines(keepends=True)
    require(bool(history) and hashlib.sha256(history[0]).hexdigest()==seal["initial_history_sha256"],"initial history seal")
    require(json.loads(history[0])==seal["initial_state"] and seal["initial_state"]["state"]=="PREPARED","initial lifecycle")
    require(json.loads(history[-1])==state and all(json.loads(h)["detail"]["candidate_commit"]==m["candidate"]["commit"] for h in history),"state/history identity")
    transitions={"PREPARED":{"RUNNING","BLOCKED"},"RUNNING":{"EXECUTED_PENDING_AUDIT","BLOCKED"},"EXECUTED_PENDING_AUDIT":{"BLOCKED"},"BLOCKED":set()}
    for a,b in zip(history,history[1:]):require(json.loads(b)["state"] in transitions[json.loads(a)["state"]],"history transition")
    if unused:
        require(len(history)==1 and state==seal["initial_state"] and not pathlib.Path(m["evidence_root"]).exists(),"attempt already consumed")
    p=validate_profile(root/"profile.json")
    require(read_json(root/"plan.json")==m["plan"]=={"cells":command_plan(p,pathlib.Path(m["working_directory"]),m["environment"])},"plan differs from canonical commands")
    require(read_json(root/"planned-inventory.json")==planned_inventory(m["plan"]["cells"]),"planned inventory differs")
    require(m["source_checks"]==read_json(root/"source-checks.json"),"source checks differ")
    if "inputs" in m:
        require(sha256_file(root/"profile.json")==m["inputs"][PROFILE]["sha256"],"prepared profile authority hash")
        for path,entry in m["inputs"].items():
            require({"path":path,"sha256":entry["sha256"]} in m["candidate"]["source_inventory"],"authority not in candidate inventory")
    expected_lines=[]
    for c in p["cases"]:
        i=c["inputs"]
        expected_lines.append(" ".join([c["id"],str(c["dimension"]),c["operation"],c["operand_kind"],c["direction"],c["claim_category"],str(i["exponent"]),*i["origin"],*i["basis"],*i["operand"],*i["auxiliary"]]))
    require((root/"case-inputs.txt").read_text(encoding="ascii")=="\n".join(expected_lines)+"\n","exporter input table differs from profile")
    return m

def prepare(args):
    source=pathlib.Path(args.source_root).resolve();control=pathlib.Path(args.output_root).resolve();evidence=pathlib.Path(args.evidence_root).resolve()
    validate_roots(source,control,evidence)
    candidate=clean_candidate(source)
    require(git(source,"rev-parse","@{upstream}").decode().strip()==candidate["commit"],"candidate upstream differs")
    # Query the actual remote ref; a stale tracking ref is not publication evidence.
    remote=git(source,"config","--get","branch."+git(source,"branch","--show-current").decode().strip()+".remote").decode().strip()
    branch=git(source,"config","--get","branch."+git(source,"branch","--show-current").decode().strip()+".merge").decode().strip()
    require(git(source,"ls-remote",remote,branch).decode().split()[0]==candidate["commit"],"published remote differs")
    checks=source_checks(source);profile=validate_profile(source/PROFILE);env=environment_identity()
    plan={"cells":command_plan(profile,source,env)}
    m={"schema_version":3,"kind":"cartesian-frames-prepared-manifest","state":"PREPARED","execution_requested":False,
       "candidate":candidate,"working_directory":str(source),"control_root":str(control),"evidence_root":str(evidence),
       "environment":env,"inputs":input_identity(input_paths(source)),"source_checks":checks,"plan":plan,
       "prepared_utc":utc_now(),"limitations":profile["limitations"]}
    control.mkdir()
    write_json(control/"prepared-manifest.json",m);write_json(control/"plan.json",plan)
    shutil.copyfile(source/PROFILE,control/"profile.json")
    write_json(control/"source-checks.json",checks);write_json(control/"planned-inventory.json",planned_inventory(plan["cells"]))
    write_inputs(profile,control/"case-inputs.txt")
    write_state(control,"PREPARED",{"candidate_commit":candidate["commit"],"execution_requested":False})
    seal_preparation(control);validate_prepared(control)

def expand(argv,control,evidence):
    return [s.replace("@CONTROL_ROOT@",str(control)).replace("@EVIDENCE_ROOT@",str(evidence)) for s in argv]

def discovery(text,expected):
    data=json.loads(text)
    names=[t["name"] for t in data["tests"]]
    require(len(names)==len(set(names)) and set(names)==set(expected),"discovered CTest selection differs")
    require(all(t.get("command") for t in data["tests"]),"selected CTest has no executable")
    return data

def dependency_check(text):
    blocks=text.split("\n\n")
    def block(path):
        found=[b for b in blocks if b.startswith(f"CMakeFiles/apmesh_core.dir/{path}.o:")]
        require(len(found)==1 and "deps not found" not in found[0],"missing observed object dependencies")
        return found[0]
    mathblock=block("src/math/linear_algebra.cpp");geom=block("src/core/geometry.cpp")
    require("apmesh/math/linear_algebra.hpp" in mathblock and "apmesh/math/linear_algebra.hpp" in geom,"math dependency absent")
    require("apmesh/core/geometry.hpp" not in mathblock,"math depends on geometry")
    return {"math_to_geometry":False,"geometry_to_math":True}

def capture_record(record,root):
    data={}
    for s in ("stdout","stderr"):
        p=root/record[s]["path"];require(sha256_file(p)==record[s]["sha256"],"command output changed")
        # UTF-8 text is retained as evidence; transient log files are not retained.
        text=p.read_bytes().decode("utf-8")
        data[s]={"sha256":record[s]["sha256"],"text":text}
    return {"id":record["id"],**data}

def validate_observations(records,observations):
    require([r["id"] for r in records]==[o["id"] for o in observations],"command observation inventory")
    require(len({r["id"] for r in records})==len(records),"duplicate command records")
    for r,o in zip(records,observations):
        for stream in ("stdout","stderr"):
            require(hashlib.sha256(o[stream]["text"].encode()).hexdigest()==o[stream]["sha256"]==r[stream]["sha256"],"observed stream hash")

def validate_build_metadata(cache,commands,build,cell,env,source):
    compiler,libcxx=CONFIGURATIONS[cell]
    required={"CMAKE_BUILD_TYPE:STRING":"Debug" if cell.endswith("debug") else "Release",
              "CMAKE_HOME_DIRECTORY:INTERNAL":str(source),"APMESH_USE_LIBCXX:BOOL":"ON" if libcxx else "OFF"}
    entries=dict(line.split("=",1) for line in cache.splitlines() if "=" in line and not line.startswith("//"))
    compiler_values=[entries[f"CMAKE_CXX_COMPILER:{kind}"] for kind in ("FILEPATH","STRING","UNINITIALIZED")
                     if f"CMAKE_CXX_COMPILER:{kind}" in entries]
    require(compiler_values and all(pathlib.Path(value).resolve()==pathlib.Path(env["tools"][compiler]["path"]).resolve()
                                    for value in compiler_values),"cache compiler identity differs")
    for key,v in required.items():
        actual=entries.get(key)
        require(actual is not None and actual==v,"cache identity differs: "+key)
    require(any(pathlib.Path(row["file"]).resolve()==source/"src/core/geometry.cpp" for row in commands),"geometry compile command absent")
    require(all(pathlib.Path(row["file"]).resolve().is_relative_to(source) for row in commands),"compile source escapes revision")
    require(all(pathlib.Path(row["directory"]).resolve()==build for row in commands),"compile build directory differs")
    for row in commands:
        command=row.get("command"," ".join(row.get("arguments",[])))
        require(pathlib.Path(shlex.split(command)[0]).resolve()==pathlib.Path(env["tools"][compiler]["path"]).resolve(),"compile compiler differs")
        require(("-stdlib=libc++" in command)==libcxx,"compile runtime differs")
    return cache,commands

def validate_cache(build,cell,env,source):
    return validate_build_metadata((build/"CMakeCache.txt").read_text(),
        json.loads((build/"compile_commands.json").read_text()),build,cell,env,source)

def runtime_paths(text,cell):
    require("not found" not in text and text.strip(),"missing runtime dependency")
    require(("libc++.so" in text and "libstdc++.so" not in text) if cell.startswith("clang")
            else ("libstdc++.so" in text and "libc++.so" not in text),"runtime library differs")
    paths=re.findall(r"(?:=>\s*)?(/[^\s()]+)",text)
    allowed=re.compile(r"(lib(?:c|m|gcc_s|stdc\+\+|c\+\+|c\+\+abi|unwind)\.so(?:\.[0-9]+)*|ld-linux[^/]*\.so(?:\.[0-9]+)*)$")
    require(paths and all(allowed.fullmatch(pathlib.Path(p).name) for p in paths),"unexpected runtime dependency")
    return paths

def execution_claim(control,evidence,manifest):
    evidence.mkdir() # exclusive creation prevents two executions of the same plan
    claim={"prepared_sha256":sha256_file(control/"prepared-manifest.json"),"candidate_commit":manifest["candidate"]["commit"],"pid":os.getpid(),"started_utc":utc_now(),"execution_requested":True}
    with (evidence/"execution-claim.json").open("xb") as f:f.write(canonical_json(claim))
    write_state(control,"RUNNING",{"candidate_commit":manifest["candidate"]["commit"]})
    return claim

def artifact_snapshot(root):
    excluded={"execution-claim.json","command-records.json","command-observations.json","terminal-manifest.json",
              "artifact-snapshots.json","failure.json","retention-error.json"}
    return [{"path":p.relative_to(root).as_posix(),"sha256":sha256_file(p),"size":p.stat().st_size}
            for p in sorted(root.rglob("*")) if p.is_file() and p.relative_to(root).parts[0] not in {"logs","retained",".retaining"}
            and "build" not in p.relative_to(root).parts and p.relative_to(root).as_posix() not in excluded]

def terminal(control,evidence,m,state,records,observations,failure=None):
    write_json(evidence/"command-records.json",{"records":records})
    write_json(evidence/"command-observations.json",{"observations":observations})
    if failure:write_json(evidence/"failure.json",failure)
    write_json(evidence/"artifact-snapshots.json",{"files":artifact_snapshot(evidence)})
    write_json(evidence/"terminal-manifest.json",{"schema_version":3,"state":state,
        "candidate_commit":m["candidate"]["commit"],"prepared_sha256":sha256_file(control/"prepared-manifest.json"),
        "execution_claim_sha256":sha256_file(evidence/"execution-claim.json"),
        "records_sha256":sha256_file(evidence/"command-records.json"),
        "artifacts_sha256":sha256_file(evidence/"artifact-snapshots.json"),"failure":failure,"ended_utc":utc_now()})
    if read_json(control/"state.json")["state"]!=state:
        write_state(control,state,{"candidate_commit":m["candidate"]["commit"],"closure_failure":bool(failure)})

def execute(args):
    control=pathlib.Path(args.output_root).resolve();m=validate_prepared(control)
    source=pathlib.Path(m["working_directory"]);root=pathlib.Path(m["evidence_root"])
    require(clean_candidate(source)==m["candidate"],"candidate changed after preparation")
    require(environment_identity()==m["environment"],"execution environment changed")
    verify_input_identity(m["inputs"],input_paths(source));require(source_checks(source)==m["source_checks"],"reviewed source changed")
    profile=validate_profile(control/"profile.json")
    execution_claim(control,root,m);records=[];observations=[];index={"candidate_commit":m["candidate"]["commit"],"certificates":[]};inventories=[]
    stage="initialization"
    try:
        for c in m["plan"]["cells"]:
            cell=c["cell"];build=root/"cells"/cell/"build";cellroot=build.parent;dependencies=[];prereqs={}
            for item in c["commands"]:
                stage=item["id"]
                before=clean_candidate(source);require(before==m["candidate"],"revision changed during execution")
                r=run_command(expand(item["argv"],control,root),source,root/"logs",stage,item["timeout_seconds"])
                r["source_identity"]={"commit":before["commit"],"inventory_sha256":hashlib.sha256(canonical_json(before["source_inventory"])).hexdigest()}
                records.append(r);obs=capture_record(r,root);observations.append(obs)
                write_json(root/"command-records.json",{"records":records})
                write_json(root/"command-observations.json",{"observations":observations})
                command_ok(r)
                if item["stage"]=="build":
                    cache,commands=validate_cache(build,cell,m["environment"],source)
                    (cellroot/"cache.txt").write_text(cache,encoding="utf-8")
                    write_json(cellroot/"compile_commands.json",{"commands":commands})
                if item["stage"].endswith("discovery"):
                    expected=REQUIRED_FOCUSED if item["stage"]=="focused-discovery" else profile["exact_prerequisite_tests"]
                    prereqs[item["stage"]]=discovery(obs["stdout"]["text"],expected)
                if item["stage"]=="object-dependencies":prereqs["dependency_direction"]=dependency_check(obs["stdout"]["text"])
                if item["stage"].startswith("ldd-"):
                    exe=build/item["stage"][4:];text=obs["stdout"]["text"]
                    deps=[]
                    for absolute in runtime_paths(text,cell):
                        path=pathlib.Path(absolute);require(path.is_file(),"ldd dependency file absent")
                        deps.append({"path":str(path),"resolved_path":str(path.resolve()),"sha256":sha256_file(path)})
                    dependencies.append({"executable":exe.name,"sha256":sha256_file(exe),"command_id":r["id"],"libraries":deps})
                if item["stage"].startswith("certificate-"):
                    repeat=int(item["stage"].split("-")[-1]);cert=root/"certificates"/f"{cell}-{repeat}.json"
                    semantic=validate_certificate(profile,cert)
                    require(read_json(cert)["pid"]==r["pid"],"exporter PID mismatch")
                    write_json(root/"semantic"/cert.name,semantic)
                    index["certificates"].append({"cell":cell,"repetition":repeat,"path":str(cert.relative_to(root)),
                        "sha256":sha256_file(cert),"candidate_commit":m["candidate"]["commit"],"command":r,
                        "executable_sha256":sha256_file(build/"apmesh_core_cartesian_frames_export")})
            write_json(cellroot/"dependencies.json",{"executables":dependencies})
            write_json(cellroot/"prerequisites.json",prereqs)
            projections=[read_json(root/"semantic"/f"{cell}-{i}.json") for i in range(1,4)]
            require(all(p==projections[0] for p in projections),"within-cell difference")
            write_json(cellroot/"comparison.json",{"cell":cell,"equivalent":True,"certificates":[sha256_file(root/"certificates"/f"{cell}-{i}.json") for i in range(1,4)]})
            negative_self_check(profile,root/"certificates"/f"{cell}-1.json",cellroot/"negative-outcomes.json")
            inventories.append({"cell":cell,"files":{name:sha256_file(cellroot/name) for name in ("cache.txt","compile_commands.json","dependencies.json","prerequisites.json","comparison.json","negative-outcomes.json")}})
        write_json(root/"certificate-index.json",index);compare(profile,root/"certificate-index.json",root/"cross-cell-comparison.json")
        write_json(root/"observed-inventories.json",{"cells":inventories})
        write_json(root/"limitations.json",{"limitations":m["limitations"]})
        write_json(root/"gate-summary.json",{"status":"EVIDENCE_COLLECTED_PENDING_AUDIT","gates":{g:"PENDING_SCIENTIFIC_AUDIT" for g in profile["gates"]}})
        terminal(control,root,m,"EXECUTED_PENDING_AUDIT",records,observations)
        seal_output(control,root,source)
    except Exception as error:
        terminal(control,root,m,"BLOCKED",records,observations,{"stage":stage,"type":type(error).__name__,"message":str(error),"retry_attempted":False})
        if not (root/"retention-error.json").exists():
            try:seal_output(control,root,source)
            except Exception as retention_error:write_json(root/"retention-error.json",{"message":str(retention_error),"retry_attempted":False})
        raise

def payload_digest(package):
    rows=[{"path":p.relative_to(package).as_posix(),"sha256":sha256_file(p)}
          for folder in ("control","evidence") for p in sorted((package/folder).rglob("*")) if p.is_file()]
    return hashlib.sha256(canonical_json(rows)).hexdigest()

def verify_payload(package,verification_source=None):
    control=package/"control";m=validate_prepared(control,unused=False);root=package/"evidence"
    t=read_json(root/"terminal-manifest.json");state=read_json(control/"state.json")
    require(t["candidate_commit"]==m["candidate"]["commit"] and t["state"]==state["state"],"terminal/state revision")
    require(t["state"] in {"BLOCKED","EXECUTED_PENDING_AUDIT"},"nonterminal package")
    require(t["prepared_sha256"]==sha256_file(control/"prepared-manifest.json"),"terminal prepared hash")
    claim=read_json(root/"execution-claim.json")
    require(t["execution_claim_sha256"]==sha256_file(root/"execution-claim.json") and claim["prepared_sha256"]==t["prepared_sha256"] and claim["candidate_commit"]==t["candidate_commit"] and claim["execution_requested"] is True,"execution claim identity")
    require(t["records_sha256"]==sha256_file(root/"command-records.json"),"command hash")
    require(t["artifacts_sha256"]==sha256_file(root/"artifact-snapshots.json")
            and read_json(root/"artifact-snapshots.json")=={"files":artifact_snapshot(root)},"produced/retained artifact snapshot differs")
    records=read_json(root/"command-records.json")["records"];obs=read_json(root/"command-observations.json")["observations"]
    validate_observations(records,obs)
    expected=[item for c in m["plan"]["cells"] for item in c["commands"]]
    require(len(records)<=len(expected),"extra commands")
    if t["state"]=="EXECUTED_PENDING_AUDIT":require(len(records)==len(expected),"missing executed commands")
    for n,(r,item) in enumerate(zip(records,expected)):
        require(r["id"]==item["id"] and r["argv"]==expand(item["argv"],pathlib.Path(m["control_root"]),pathlib.Path(m["evidence_root"])) and r["cwd"]==m["working_directory"] and r["timeout_seconds"]==item["timeout_seconds"],"observed command differs from plan")
        if n<len(records)-1 or t["state"]=="EXECUTED_PENDING_AUDIT":command_ok(r)
        require(r["source_identity"]=={"commit":m["candidate"]["commit"],
            "inventory_sha256":hashlib.sha256(canonical_json(m["candidate"]["source_inventory"])).hexdigest()},"executed source identity differs")
    profile=validate_profile(control/"profile.json")
    if verification_source is not None:
        observed=clean_candidate(verification_source)
        require(observed["commit"]==m["candidate"]["commit"] and observed["source_inventory"]==m["candidate"]["source_inventory"],"detached source differs")
        for relative,identity in m["inputs"].items():require(sha256_file(verification_source/relative)==identity["sha256"],"detached authority hash")
        require(source_checks(verification_source)==m["source_checks"],"detached baseline check")
    if t["state"]=="BLOCKED":
        require(t["failure"]==read_json(root/"failure.json") and t["failure"]["retry_attempted"] is False,"failure evidence")
        return {"status":"BLOCKED","candidate_commit":t["candidate_commit"],"payload_sha256":payload_digest(package)}
    # Recompute all successful report-only evidence from retained data.
    result=compare(profile,root/"certificate-index.json",None)
    require(result==read_json(root/"cross-cell-comparison.json"),"retained comparison differs")
    entries=read_json(root/"certificate-index.json")["certificates"]
    lookup={r["id"]:(r,o) for r,o in zip(records,obs)}
    for e in entries:
        r,_=lookup[e["command"]["id"]];require(r==e["command"],"certificate process record differs")
        require(validate_certificate(profile,root/e["path"])==read_json(root/"semantic"/pathlib.Path(e["path"]).name),"semantic certificate differs")
    inventories=read_json(root/"observed-inventories.json")
    require([c["cell"] for c in inventories["cells"]]==CELLS,"observed cell inventory")
    for cellrow in inventories["cells"]:
        cell=cellrow["cell"];cellroot=root/"cells"/cell
        require(set(cellrow["files"])=={"cache.txt","compile_commands.json","dependencies.json","prerequisites.json","comparison.json","negative-outcomes.json"},"cell artifact inventory")
        for p,h in cellrow["files"].items():require(sha256_file(cellroot/p)==h,"observed artifact hash")
        validate_build_metadata((cellroot/"cache.txt").read_text(),
            read_json(cellroot/"compile_commands.json")["commands"],
            pathlib.Path(m["evidence_root"])/"cells"/cell/"build",cell,m["environment"],pathlib.Path(m["working_directory"]))
        prereqs=read_json(cellroot/"prerequisites.json")
        for key,selection in (("focused-discovery",REQUIRED_FOCUSED),("prerequisite-discovery",profile["exact_prerequisite_tests"])):
            _,o=lookup[cell+"-"+key];require(discovery(o["stdout"]["text"],selection)==prereqs[key],"discovery projection differs")
        _,o=lookup[cell+"-object-dependencies"];require(dependency_check(o["stdout"]["text"])==prereqs["dependency_direction"],"dependency direction differs")
        deps=read_json(cellroot/"dependencies.json")["executables"]
        require([d["executable"] for d in deps]==RUNTIME_EXECUTABLES,"runtime executable inventory")
        for d in deps:
            r,o=lookup[d["command_id"]];command_ok(r)
            require(pathlib.Path(r["argv"][-1]).name==d["executable"],"runtime command link")
            require([x["path"] for x in d["libraries"]]==runtime_paths(o["stdout"]["text"],cell),"runtime library inventory")
            require(all(re.fullmatch("[0-9a-f]{64}",x["sha256"]) for x in d["libraries"]),"runtime library provenance")
        exporter=next(d for d in deps if d["executable"]=="apmesh_core_cartesian_frames_export")
        require(all(e["executable_sha256"]==exporter["sha256"] for e in entries if e["cell"]==cell),"certificate binary differs")
        validate_negatives(profile,root/"certificates"/f"{cell}-1.json",cellroot/"negative-outcomes.json")
        require(read_json(cellroot/"comparison.json")=={"cell":cell,"equivalent":True,"certificates":[sha256_file(root/"certificates"/f"{cell}-{i}.json") for i in range(1,4)]},"within-cell comparison differs")
    require(read_json(root/"limitations.json")=={"limitations":m["limitations"]},"limitations differ")
    require(read_json(root/"gate-summary.json")=={"status":"EVIDENCE_COLLECTED_PENDING_AUDIT","gates":{g:"PENDING_SCIENTIFIC_AUDIT" for g in profile["gates"]}},"scientific outcome assigned by tooling")
    return {"status":"EVIDENCE_COLLECTED_PENDING_AUDIT","candidate_commit":t["candidate_commit"],"payload_sha256":payload_digest(package)}

def retain_files(control,root,stage):
    (stage/"control").mkdir(parents=True);(stage/"evidence").mkdir()
    for p in control.iterdir():
        require(p.is_file(),"unexpected control directory")
        shutil.copyfile(p,stage/"control"/p.name)
    for p in root.rglob("*"):
        if not p.is_file():continue
        rel=p.relative_to(root)
        if "build" in rel.parts or rel.parts[0] in {"logs","retained",".retaining"} or p.name=="retention-error.json":continue
        require(not p.is_symlink(),"symlink in retained inputs")
        dest=stage/"evidence"/rel;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(p,dest)

def detached_verify(source,package,candidate):
    with tempfile.TemporaryDirectory(prefix="cf-detached-") as temp:
        checkout=pathlib.Path(temp)/"candidate"
        git(source,"worktree","add","--detach",str(checkout),candidate)
        try:
            # Execute the candidate's verifier, not an import from the caller's checkout.
            r=run_command([sys.executable,str(checkout/"tools/run_cartesian_frames_qualification.py"),
                "verify-payload","--package",str(package),"--verification-source",str(checkout)],
                checkout,pathlib.Path(temp)/"logs","detached-verification",120)
            command_ok(r)
            return {"candidate_commit":candidate,"status":"VERIFIED","command":r,"observations":capture_record(r,pathlib.Path(temp))}
        finally:git(source,"worktree","remove",str(checkout))

def seal_output(control,root,source):
    stage=root/".retaining";dest=root/"retained";require(not stage.exists() and not dest.exists(),"retention destination already exists")
    m=read_json(control/"prepared-manifest.json")
    stage.mkdir()
    try:
        retain_files(control,root,stage);verify_payload(stage)
        detached=detached_verify(source,stage,m["candidate"]["commit"])
        write_json(stage/"detached-verification.json",detached)
        files=sorted(p for p in stage.rglob("*") if p.is_file())
        inv=retention_manifest(stage,[(p,p) for p in files],m["candidate"]["commit"],None)
        write_json(stage/"retention-manifest.json",inv)
        verify_retention(stage);stage.rename(dest)
    except Exception as e:
        write_json(root/"retention-error.json",{"stage":str(stage),"error_type":type(e).__name__,"message":str(e),"retry_attempted":False})
        raise

def verify_retention(package):
    inv=read_json(package/"retention-manifest.json")
    listed=[r["retained_path"] for r in inv["files"]]
    actual=sorted(str(p.relative_to(package).as_posix()) for p in package.rglob("*") if p.is_file() and p!=package/"retention-manifest.json")
    require(sorted(listed)==actual and len(listed)==len(set(listed)),"retained inventory bijection")
    for row in inv["files"]:
        p=(package/row["retained_path"]).resolve()
        require(p.is_relative_to(package.resolve()) and sha256_file(p)==row["sha256"] and p.stat().st_size==row["size"],"retained hash or path")
    result=verify_payload(package)
    detached=read_json(package/"detached-verification.json")
    require(inv["candidate_commit"]==result["candidate_commit"]==detached["candidate_commit"] and detached["status"]=="VERIFIED","detached retained identity")
    command_ok(detached["command"]);validate_observations([detached["command"]],[detached["observations"]])
    require(json.loads(detached["observations"]["stdout"]["text"])==result,"detached semantic result differs")
    argv=detached["command"]["argv"]
    require(len(argv)==7 and argv[2]=="verify-payload" and argv[3]=="--package" and argv[5]=="--verification-source","detached verifier command differs")
    require(pathlib.Path(argv[1])==pathlib.Path(argv[6])/"tools/run_cartesian_frames_qualification.py" and detached["command"]["cwd"]==argv[6],"detached verifier source differs")
    return result

def self_check(args):
    source=pathlib.Path(args.source_root).resolve();p=validate_profile(source/PROFILE);source_checks(source)
    # Build no manifest and run no formal commands here.
    require(len(p["cases"])==142 and len(p["cells"])==4,"fixed profile")
    record={"status":"pass","formal_manifest":None,"execution_requested":False}
    if args.output_root:write_json(pathlib.Path(args.output_root),record)
    return record

def main():
    parser=argparse.ArgumentParser(allow_abbrev=False)
    parser.add_argument("command",choices=["self-check","prepare","execute","verify-retention","verify-payload"])
    for name in ("source-root","output-root","evidence-root","package","verification-source","profile","protocol","validator","exporter"):
        parser.add_argument("--"+name)
    args=parser.parse_args()
    try:
        if args.source_root:
            source=pathlib.Path(args.source_root).resolve()
            for key,relative in (("profile",PROFILE),("protocol",PROTOCOL),("validator","tools/cartesian_frames_evidence.py")):
                supplied=getattr(args,key)
                require(supplied is None or pathlib.Path(supplied).resolve()==source/relative,"noncanonical supplied "+key)
        if args.command=="self-check":self_check(args)
        elif args.command=="prepare":prepare(args)
        elif args.command=="execute":execute(args)
        elif args.command=="verify-retention":print(json.dumps(verify_retention(pathlib.Path(args.package))))
        else:print(json.dumps(verify_payload(pathlib.Path(args.package),pathlib.Path(args.verification_source) if args.verification_source else None)))
    except Exception as e:print(f"Cartesian qualification: {type(e).__name__}: {e}",file=sys.stderr);return 2
    return 0
if __name__=="__main__":raise SystemExit(main())
