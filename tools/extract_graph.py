#!/usr/bin/env python3
"""Extract an architecture graph (files, classes, functions, tests, targets, deps)
from the context-forge repository into a JSON file consumed by the docs graph page."""
import json, os, re, sys
from pathlib import Path

ROOT = Path(sys.argv[1]).resolve()
OUT = Path(sys.argv[2])

nodes = {}   # id -> node
edges = []   # {source, target, kind}

def add_node(nid, kind, label, **meta):
    if nid not in nodes:
        nodes[nid] = {"id": nid, "kind": kind, "label": label, **meta}
    else:
        nodes[nid].update({k: v for k, v in meta.items() if v is not None})
    return nodes[nid]

def add_edge(s, t, kind):
    if s == t:
        return
    key = (s, t, kind)
    if key not in {(e["source"], e["target"], e["kind"]) for e in edges}:
        edges.append({"source": s, "target": t, "kind": kind})

def strip_banner(text):
    return "\n".join(l for l in text.splitlines() if not re.match(r"^\s*[█╚╔║╝═╗]+", l))

# ---------------------------------------------------------------- files
SRC_DIRS = ["include", "src", "tests"]
files = []
for d in SRC_DIRS:
    for p in sorted((ROOT / d).rglob("*")):
        if p.suffix in (".hpp", ".cpp") and p.is_file():
            files.append(p)

def rel(p): return p.relative_to(ROOT).as_posix()

def file_group(r):
    if r.startswith("tests/"): return "tests"
    if "/rules/triggers/" in r: return "plugin:trigger"
    if "/rules/pre-rules/" in r: return "plugin:pre-rule"
    if "/rules/rules/" in r: return "plugin:rule"
    if "/rules/" in r: return "rules-core"
    return "core"

for p in files:
    r = rel(p)
    kind = "header" if p.suffix == ".hpp" else ("test-file" if r.startswith("tests/") else "source")
    add_node("file:" + r, kind, p.name, path=r, group=file_group(r),
             loc=sum(1 for _ in p.open(encoding="utf-8", errors="ignore")))

# resolve includes
by_name = {}
for p in files:
    by_name.setdefault(p.name, []).append(p)

EXTERNAL = {
    "utils/": ("ext:libutils", "libutils", "https://github.com/TsukiNi22/libutils"),
    "httplib.h": ("ext:cpp-httplib", "cpp-httplib", "https://github.com/yhirose/cpp-httplib"),
    "libconfig.h++": ("ext:libconfig++", "libconfig++", "https://hyperrealm.github.io/libconfig/"),
    "gtest/": ("ext:gtest", "GoogleTest", "https://github.com/google/googletest"),
    "nlohmann/": ("ext:nlohmann-json", "nlohmann/json", "https://github.com/nlohmann/json"),
}
for eid, label, url in EXTERNAL.values():
    add_node(eid, "external", label, url=url)

def resolve_include(from_path, inc):
    # quoted includes: relative to including file, then to include/, tests/
    cands = [from_path.parent / inc, ROOT / "include" / inc, ROOT / "tests" / inc]
    for c in cands:
        try:
            c = c.resolve()
        except FileNotFoundError:
            continue
        if c.exists() and c.is_file() and ROOT in c.parents:
            return c
    return None

for p in files:
    text = strip_banner(p.read_text(encoding="utf-8", errors="ignore"))
    for m in re.finditer(r'^\s*#\s*include\s+([<"])([^>"]+)[>"]', text, re.M):
        q, inc = m.group(1), m.group(2)
        target = resolve_include(p, inc) if q == '"' else None
        if target is None and q == "<":
            # try external match
            for prefix, (eid, _, _) in EXTERNAL.items():
                if inc.startswith(prefix):
                    add_edge("file:" + rel(p), eid, "includes")
                    break
            continue
        if target is not None:
            add_edge("file:" + rel(p), "file:" + rel(target), "includes")

# ---------------------------------------------------------------- classes (headers)
CLASS_RE = re.compile(r'^\s*class\s+(\w+)\s*(?::\s*([^{]+))?\{', re.M)
METHOD_RE = re.compile(
    r'^\s*(?:_\w+\s+)*'                                   # attributes _hot _nodiscard
    r'(?:virtual\s+|static\s+|inline\s+|explicit\s+)*'
    r'(?:[\w:<>,\s\*&]+?\s+)?'                             # return type (optional for ctors)
    r'([~]?\w+)\s*\(([^;{]*?)\)\s*(const)?\s*(?:final|override)?\s*(?:=\s*(0|default|delete))?\s*[;{:]',
    re.M)

class_of_file = {}
for p in files:
    if p.suffix != ".hpp":
        continue
    r = rel(p)
    text = strip_banner(p.read_text(encoding="utf-8", errors="ignore"))
    # namespace
    ns_m = re.search(r'namespace\s+([\w:]+)\s*\{', text)
    ns = ns_m.group(1) if ns_m else ""
    for cm in CLASS_RE.finditer(text):
        cname = cm.group(1)
        bases = cm.group(2) or ""
        fq = f"{ns}::{cname}" if ns else cname
        cid = "class:" + fq
        is_iface = cname.startswith("I") and cname[1:2].isupper()
        is_abs = cname.startswith("A") and cname[1:2].isupper() and cname != "AInstruction" or cname == "AInstruction"
        ckind = "interface" if is_iface else ("abstract" if cname.startswith("A") and cname[1:2].isupper() else "class")
        if r.startswith("tests/"):
            ckind = "mock"
        add_node(cid, ckind, cname, fq=fq, path=r, group=file_group(r))
        add_edge("file:" + r, cid, "defines")
        class_of_file.setdefault(r, []).append((cname, fq, cid))
        for b in re.split(r',', bases):
            b = b.strip()
            b = re.sub(r'^(public|private|protected)\s+', '', b).strip()
            if not b:
                continue
            bname = b.split("<")[0].split("::")[-1]
            if "utils::" in b:
                add_edge(cid, "ext:libutils", "inherits")
                continue
            # resolve base class node later
            edges.append({"source": cid, "target": "classname:" + bname, "kind": "inherits"})
        # methods inside class body
        body_start = cm.end()
        depth = 1
        i = body_start
        while i < len(text) and depth > 0:
            if text[i] == "{": depth += 1
            elif text[i] == "}": depth -= 1
            i += 1
        body = text[body_start:i]
        for mm in METHOD_RE.finditer(body):
            name, params, const, spec = mm.group(1), mm.group(2), mm.group(3), mm.group(4)
            if name in ("operator", "class", "return", "if", "for", "while", "switch", "static_cast", "throw"):
                continue
            if spec == "delete":
                continue
            if name == cname or name == "~" + cname:
                continue  # ctors/dtors: skip to keep graph readable
            if "operator" in mm.group(0):
                continue
            sig = f"{name}({' '.join(params.split())})"
            mid = f"method:{fq}::{name}"
            mkind = "pure-virtual" if spec == "0" else "method"
            add_node(mid, mkind, name, fq=f"{fq}::{name}", path=r, group=file_group(r), signature=sig)
            add_edge(cid, mid, "member")

# resolve classname: placeholders
name_to_id = {}
for nid, n in nodes.items():
    if nid.startswith("class:"):
        name_to_id.setdefault(n["label"], nid)
for e in edges:
    if e["target"].startswith("classname:"):
        nm = e["target"].split(":", 1)[1]
        e["target"] = name_to_id.get(nm, "ext:libutils")

# ---------------------------------------------------------------- free functions & method definitions (.cpp)
FUNC_DEF_RE = re.compile(
    r'^(?:_\w+\s+)*(?:static\s+|inline\s+)*(?:[\w:<>,\*&\s]+?\s+)?([\w:~]+)\s*\(([^;{]*?)\)\s*(?:const)?\s*\{', re.M)
CFUNC_RE = re.compile(r'^\s*([\w\*: ]+?)\s+(\w+)\s*\(\s*(?:void)?\s*\)\s*\{', re.M)

for p in files:
    if p.suffix != ".cpp":
        continue
    r = rel(p)
    text = strip_banner(p.read_text(encoding="utf-8", errors="ignore"))
    fid = "file:" + r
    # extern "C" plugin entry points
    ext_c = re.search(r'extern\s+"C"\s*\{(.*?)\n\}', text, re.S)
    if ext_c:
        for m in CFUNC_RE.finditer(ext_c.group(1)):
            fname = m.group(2)
            nid = f"cfunc:{r}::{fname}"
            add_node(nid, "c-entry", fname + "()", path=r, group=file_group(r), signature=f'extern "C" {m.group(1).strip()} {fname}()')
            add_edge(fid, nid, "defines")
            # factory returns a class
            ret = m.group(1)
            if "factory" == fname:
                for cname, fq, cid in [c for cs in class_of_file.values() for c in cs]:
                    if re.search(r'new\s+' + re.escape(fq), ext_c.group(1)):
                        add_edge(nid, cid, "instantiates")
    for m in FUNC_DEF_RE.finditer(text):
        name, params = m.group(1), m.group(2)
        if name.startswith("if") or name in ("if", "for", "while", "switch", "catch", "else"):
            continue
        if name.startswith(("TEST", "INSTANTIATE")) or "this->" in params or name == "argv":
            continue
        if name in ("main",):
            nid = "func:main"
            add_node(nid, "function", "main", path=r, group=file_group(r), signature="int main(int argc, const char* argv[])")
            add_edge(fid, nid, "defines")
            continue
        if "::" in name:
            # method definition -> link to method node
            fq = name
            parts = fq.split("::")
            cls_fq = "::".join(parts[:-1])
            mname = parts[-1]
            mid = f"method:{cls_fq}::{mname}"
            if mid in nodes:
                add_edge(fid, mid, "implements")
            elif "class:" + cls_fq in nodes:
                add_node(mid, "method", mname, fq=fq, path=r, group=file_group(r), signature=f"{mname}({' '.join(params.split())})")
                add_edge("class:" + cls_fq, mid, "member")
                add_edge(fid, mid, "implements")
        else:
            if ext_c and m.start() > ext_c.start() and m.start() < ext_c.end():
                continue
            nid = f"func:{r}::{name}"
            add_node(nid, "function", name, path=r, group=file_group(r), signature=f"static {name}({' '.join(params.split())})", static=True)
            add_edge(fid, nid, "defines")

# ---------------------------------------------------------------- tests
TEST_RE = re.compile(r'^\s*(TEST|TEST_F|TEST_P)\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)', re.M)
for p in files:
    r = rel(p)
    if not r.startswith("tests/") or p.suffix != ".cpp":
        continue
    text = strip_banner(p.read_text(encoding="utf-8", errors="ignore"))
    fid = "file:" + r
    for m in TEST_RE.finditer(text):
        macro, suite, name = m.groups()
        tid = f"test:{suite}.{name}"
        add_node(tid, "test", f"{suite}.{name}", path=r, group="tests", macro=macro, suite=suite)
        add_edge(fid, tid, "defines")
        # link to the class under test by suite name heuristics
        base = re.sub(r'Test$', '', suite)
        best = max((cn for cn in name_to_id if base.startswith(cn) and nodes[name_to_id[cn]]["kind"] != "mock"), key=len, default=None)
        if best:
            add_edge(tid, name_to_id[best], "tests")
            base = best
        # explicit class mentions in body
        body_end = text.find("\n}", m.end())
        body = text[m.end():body_end]
        for cid_name, cid in name_to_id.items():
            if nodes[cid]["kind"] == "mock":
                continue
            if re.search(r'\b' + re.escape(cid_name) + r'\b', body) and cid_name != base:
                add_edge(tid, cid, "uses")

# ---------------------------------------------------------------- cmake targets
cm = (ROOT / "CMakeLists.txt").read_text()
def cm_list(var):
    m = re.search(r'set\(' + var + r'\s*\n(.*?)\)', cm, re.S)
    if not m: return []
    return [l.strip() for l in m.group(1).splitlines() if l.strip() and not l.strip().startswith("#")]

add_node("target:context-forge", "executable", "context-forge", description="Main executable (client + server + CLI)")
for s in cm_list("SRC"):
    add_edge("file:" + s, "target:context-forge", "compiled-into")
for lib in ["ext:libutils", "ext:cpp-httplib", "ext:libconfig++"]:
    add_edge("target:context-forge", lib, "links")

for m in re.finditer(r'add_library\((\w[\w-]*)\s+SHARED\s+([^)]*)\)', cm):
    tname = m.group(1)
    vars_ = re.findall(r'\$\{(\w+)\}', m.group(2))
    outdir = re.search(r'set_target_properties\(\s*(?:[\w-]+\s+)*' + re.escape(tname) + r'\b.*?LIBRARY_OUTPUT_DIRECTORY\s+\$\{CMAKE_SOURCE_DIR\}/([\w/-]+)', cm, re.S)
    add_node(f"target:{tname}", "plugin", tname + ".so", description=f"Shared object plugin ({outdir.group(1) if outdir else 'plugins'})",
             group="plugin:" + ("trigger" if tname.startswith("trigger") else "pre-rule" if tname.startswith("pre-rule") else "rule"))
    for v in vars_:
        for s in cm_list(v):
            add_edge("file:" + s, f"target:{tname}", "compiled-into")
    for lib in ["ext:libutils", "ext:cpp-httplib", "ext:libconfig++"]:
        add_edge(f"target:{tname}", lib, "links")

# tests target
tcm = (ROOT / "tests/CMakeLists.txt").read_text()
add_node("target:unit_tests", "executable", "unit_tests", description="GoogleTest binary (ctest)", group="tests")
for s in re.findall(r'^\s*(Forge\.cpp|Ollama\.cpp|rules/\w+\.cpp)\s*$', tcm, re.M):
    add_edge("file:tests/" + s, "target:unit_tests", "compiled-into")
for s in re.findall(r'\$\{CMAKE_SOURCE_DIR\}/(src/[\w/\-]+\.cpp)', tcm):
    add_edge("file:" + s, "target:unit_tests", "compiled-into")
for s in cm_list("SRC"):
    if not s.endswith("main.cpp"):
        add_edge("file:" + s, "target:unit_tests", "compiled-into")
for lib in ["ext:libutils", "ext:cpp-httplib", "ext:libconfig++", "ext:gtest", "ext:nlohmann-json"]:
    add_edge("target:unit_tests", lib, "links")

# drop dangling edges
valid = set(nodes)
edges = [e for e in edges if e["source"] in valid and e["target"] in valid]

# dedupe edges
seen = set(); uniq = []
for e in edges:
    k = (e["source"], e["target"], e["kind"])
    if k in seen: continue
    seen.add(k); uniq.append(e)
edges = uniq

# degree for sizing
deg = {n: 0 for n in nodes}
for e in edges:
    deg[e["source"]] += 1; deg[e["target"]] += 1
for n in nodes.values():
    n["degree"] = deg[n["id"]]

out = {"nodes": list(nodes.values()), "links": edges,
       "meta": {"repo": "https://github.com/TsukiNi22/context-forge", "generated_from": "main @ v1.0.0"}}
OUT.write_text(json.dumps(out, indent=1))
from collections import Counter
print("nodes:", len(nodes), Counter(n["kind"] for n in nodes.values()))
print("edges:", len(edges), Counter(e["kind"] for e in edges))
