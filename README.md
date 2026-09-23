# context-forge — static documentation site

Static, dependency-free documentation for [TsukiNi22/context-forge](https://github.com/TsukiNi22/context-forge) (v1.0.0).

## Pages

| File | Content |
| --- | --- |
| `index.html` | Overview, how it works, quick start, vocabulary |
| `install.html` | **User guide** — packages (Fedora/Debian), build from source, what gets installed where, bash/zsh completion setup, systemd user daemon, Ollama |
| `usage.html` | **User guide** — every sub-command, flag and environment variable; the exec/server pipeline; recipes; troubleshooting |
| `rules.html` | **User guide** — `.cfg` rule-file reference (trigger, ansi, block, ln, drop, dup, replace, insert) |
| `plugins.html` | **Developer guide** — plugin ABI, interfaces, lifecycle, complete worked examples for rule/trigger/pre-rule plugins, CMake wiring, out-of-tree builds, tests |
| `graph.html` | **Interactive 2D / 3D architecture graph** — files, classes, methods, free functions, tests, build targets, external libraries and their dependencies |

## Hosting

Everything is relative and self-contained (no CDN, no build step required): open `index.html` directly from disk, or serve the folder with any static host (GitHub Pages, nginx, `python3 -m http.server`).

- `assets/style.css`, `assets/site.js` — shared look & behaviour (light/dark theme, copy buttons, tabs).
- `assets/graph.js`, `assets/graph.css`, `assets/graph-data.js` — the architecture graph page.
- `assets/vendor/` — [force-graph](https://github.com/vasturiano/force-graph) (2D canvas) and [3d-force-graph](https://github.com/vasturiano/3d-force-graph) (WebGL, bundles three.js), MIT licensed.

## Regenerating the graph data

`tools/extract_graph.py` parses `include/`, `src/`, `tests/` and `CMakeLists.txt` of a checkout and writes the graph JSON:

```bash
python3 tools/extract_graph.py /path/to/context-forge assets/graph-data.json
python3 - <<'EOF'
import json
d = json.load(open('assets/graph-data.json'))
open('assets/graph-data.js', 'w').write('window.CF_GRAPH = ' + json.dumps(d, separators=(',', ':')) + ';\n')
EOF
```

The extractor is regex-based (no compiler needed); it recognises classes and their methods in headers, method definitions and `static` helpers in sources, `extern "C"` plugin entry points, GoogleTest `TEST*` macros, `#include` relationships and CMake targets.
