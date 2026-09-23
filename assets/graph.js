/* Architecture graph — 2D (canvas, force-graph) and 3D (WebGL, 3d-force-graph) views over assets/graph-data.js */
(function () {
  "use strict";
  const DATA = window.CF_GRAPH;
  if (!DATA) { document.getElementById("hint").textContent = "graph-data.js missing"; return; }

  // ------------------------------------------------------------------ palettes
  const KINDS = {
    "executable":   { label: "executable",       color: "#d37295", desc: "binary produced by CMake" },
    "plugin":       { label: "plugin (.so)",      color: "#499894", desc: "shared object plugin target" },
    "external":     { label: "external library", color: "#79706e", desc: "third-party dependency" },
    "header":       { label: "header (.hpp)",    color: "#4e79a7", desc: "" },
    "source":       { label: "source (.cpp)",    color: "#76b7b2", desc: "" },
    "test-file":    { label: "test file",        color: "#9c755f", desc: "" },
    "class":        { label: "class",            color: "#e15759", desc: "" },
    "interface":    { label: "interface",        color: "#f28e2b", desc: "pure virtual class" },
    "abstract":     { label: "abstract class",   color: "#ff9da7", desc: "" },
    "mock":         { label: "mock (tests)",     color: "#bab0ac", desc: "test double" },
    "method":       { label: "method",           color: "#59a14f", desc: "" },
    "pure-virtual": { label: "pure virtual",     color: "#8cd17d", desc: "" },
    "function":     { label: "free function",    color: "#b6992d", desc: "static / file-local helper" },
    "c-entry":      { label: "extern \"C\" entry", color: "#edc948", desc: "plugin ABI symbol" },
    "test":         { label: "test case",        color: "#b07aa1", desc: "GoogleTest TEST/TEST_F/TEST_P" },
  };
  const GROUPS = {
    "core": "#4e79a7", "rules-core": "#e15759", "plugin:trigger": "#7a3fb0", "plugin:pre-rule": "#1f7a8c",
    "plugin:rule": "#f28e2b", "tests": "#b07aa1", "external": "#79706e", "build": "#499894",
  };
  const LINKS = {
    "includes":      { label: "includes",      color: "#8a8a8a" },
    "defines":       { label: "defines",       color: "#b0b0b0" },
    "member":        { label: "member of",     color: "#59a14f" },
    "implements":    { label: "implements",    color: "#8cd17d" },
    "inherits":      { label: "inherits",      color: "#e15759" },
    "instantiates":  { label: "instantiates",  color: "#edc948" },
    "compiled-into": { label: "compiled into", color: "#4e79a7" },
    "links":         { label: "links",         color: "#9c755f" },
    "tests":         { label: "tests",         color: "#b07aa1" },
    "uses":          { label: "uses",          color: "#d4a6c8" },
  };
  const PRESETS = {
    overview: { nodes: ["executable", "plugin", "external", "header", "source", "test-file", "class", "interface", "abstract"], links: ["includes", "inherits", "compiled-into", "links", "defines"] },
    classes:  { nodes: ["class", "interface", "abstract", "mock", "method", "pure-virtual", "c-entry"], links: ["member", "inherits", "instantiates"] },
    plugins:  { nodes: ["plugin", "header", "source", "class", "interface", "abstract", "c-entry"], links: ["includes", "inherits", "instantiates", "compiled-into", "defines"], groupFilter: ["plugin:trigger", "plugin:pre-rule", "plugin:rule", "rules-core", "build"] },
    tests:    { nodes: ["test", "test-file", "mock", "class", "interface", "abstract", "executable", "external"], links: ["tests", "uses", "defines", "inherits", "compiled-into", "links"] },
    build:    { nodes: ["executable", "plugin", "external", "header", "source", "test-file"], links: ["compiled-into", "links", "includes"] },
    all:      { nodes: Object.keys(KINDS), links: Object.keys(LINKS) },
  };

  // ------------------------------------------------------------------ state
  const state = {
    mode: "2d",
    nodeKinds: new Set(PRESETS.overview.nodes),
    linkKinds: new Set(PRESETS.overview.links),
    groupFilter: null,
    search: "",
    labels: true,
    particles: false,
    colorGroup: false,
    isolate: false,
    selected: null,
    hover: null,
    frozen: false,
  };
  DATA.nodes.forEach((n) => { if (!n.group) n.group = n.kind === "external" ? "external" : "build"; });
  const byId = new Map(DATA.nodes.map((n) => [n.id, n]));
  const neighbors = new Map(DATA.nodes.map((n) => [n.id, new Set()]));
  DATA.links.forEach((l) => { neighbors.get(l.source).add(l.target); neighbors.get(l.target).add(l.source); });

  // ------------------------------------------------------------------ helpers
  const $ = (s) => document.querySelector(s);
  const isDark = () => (window.cfTheme ? window.cfTheme() : "light") === "dark";
  const bg = () => isDark() ? "#15140f" : "#f2efe8";
  const textColor = () => isDark() ? "#ece8de" : "#1e1c18";
  const nodeColor = (n) => state.colorGroup ? (GROUPS[n.group] || "#999") : (KINDS[n.kind] ? KINDS[n.kind].color : "#999");
  const nodeVal = (n) => 1 + Math.sqrt(n.degree || 1) * 1.6;
  const withAlpha = (hex, a) => {
    const v = parseInt(hex.slice(1), 16);
    return `rgba(${(v >> 16) & 255},${(v >> 8) & 255},${v & 255},${a})`;
  };
  const dirOf = (n) => { const parts = (n.path || "").split("/"); return parts.length > 1 ? parts[parts.length - 2] : ""; };
  const shortLabel = (n) => {
    if (n.kind === "method" || n.kind === "pure-virtual") return n.fq.split("::").slice(-2).join("::");
    if (n.kind === "function" || n.kind === "c-entry") { const d = dirOf(n); return d && d !== "forge" ? `${d} · ${n.label}` : n.label; }
    return n.label;
  };
  const tooltip = (n) => {
    const k = KINDS[n.kind] ? KINDS[n.kind].label : n.kind;
    const extra = n.signature ? `<div style="font-family:monospace;font-size:11px;opacity:.8">${esc(n.signature)}</div>` : "";
    const path = n.path ? `<div style="font-size:11px;opacity:.7">${esc(n.path)}</div>` : "";
    return `<div style="max-width:320px"><div style="font-size:10px;text-transform:uppercase;letter-spacing:.08em;color:${nodeColor(n)};font-weight:700">${k}</div><div style="font-weight:600">${esc(n.fq || n.label)}</div>${extra}${path}</div>`;
  };
  const esc = (s) => String(s).replace(/[&<>"]/g, (c) => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;" }[c]));

  // ------------------------------------------------------------------ filtering
  function visibleData() {
    const q = state.search.trim().toLowerCase();
    let nodes = DATA.nodes.filter((n) => state.nodeKinds.has(n.kind));
    if (state.groupFilter) nodes = nodes.filter((n) => state.groupFilter.includes(n.group) || n.kind === "external");
    if (q) {
      const pathy = /[\/.]/.test(q);
      const matched = new Set(nodes.filter((n) => (n.fq || n.label).toLowerCase().includes(q) || (pathy && (n.path || "").toLowerCase().includes(q))).map((n) => n.id));
      // keep matches + their direct neighbours for context
      nodes = nodes.filter((n) => matched.has(n.id) || [...neighbors.get(n.id)].some((m) => matched.has(m)));
      nodes.forEach((n) => (n._match = matched.has(n.id)));
    } else nodes.forEach((n) => (n._match = false));
    if (state.isolate && state.selected) {
      const keep = new Set([state.selected.id, ...neighbors.get(state.selected.id)]);
      nodes = nodes.filter((n) => keep.has(n.id));
    }
    const ids = new Set(nodes.map((n) => n.id));
    const links = DATA.links.filter((l) => state.linkKinds.has(l.kind) && ids.has(l.source) && ids.has(l.target))
      .map((l) => ({ source: l.source, target: l.target, kind: l.kind }));
    return { nodes, links };
  }

  // highlight set for hover/selection
  function focusSet() {
    const f = state.hover || state.selected;
    if (!f) {
      if (state.search.trim()) { const m = new Set(); DATA.nodes.forEach((n) => n._match && m.add(n.id)); return m.size ? m : null; }
      return null;
    }
    const s = new Set([f.id]); neighbors.get(f.id).forEach((m) => s.add(m));
    return s;
  }

  // ------------------------------------------------------------------ graph construction
  let graph = null;
  let visibleCount = 0;
  const container = $("#graph");

  function destroy() {
    if (graph && graph._destructor) graph._destructor();
    container.innerHTML = "";
    graph = null;
  }

  function build() {
    destroy();
    const data = visibleData();
    const idOf = (x) => (typeof x === "object" ? x.id : x);
    const common = (g) => g
      .graphData(data)
      .nodeId("id")
      .nodeLabel(tooltip)
      .nodeVal(nodeVal)
      .backgroundColor(bg())
      .linkDirectionalArrowLength(state.mode === "2d" ? 3.5 : 3)
      .linkDirectionalArrowRelPos(1)
      .linkDirectionalParticles(() => (state.particles ? 2 : 0))
      .linkDirectionalParticleWidth(state.mode === "2d" ? 2 : 1.2)
      .linkDirectionalParticleSpeed(0.006)
      .onNodeClick((n) => select(n))
      .onNodeHover((n) => { state.hover = n || null; container.style.cursor = n ? "pointer" : ""; })
      .onBackgroundClick(() => select(null));

    if (state.mode === "2d") {
      graph = ForceGraph()(container);
      common(graph)
        .width(container.clientWidth).height(container.clientHeight)
        .nodeColor((n) => { const f = focusSet(); const c = nodeColor(n); return f && !f.has(n.id) ? withAlpha(c, 0.15) : c; })
        .linkColor((l) => {
          const f = focusSet(); const c = LINKS[l.kind].color;
          if (!f) return withAlpha(c, isDark() ? 0.55 : 0.6);
          return f.has(idOf(l.source)) && f.has(idOf(l.target)) ? c : withAlpha(c, 0.06);
        })
        .linkWidth((l) => { const f = focusSet(); return f && f.has(idOf(l.source)) && f.has(idOf(l.target)) ? 2 : 1; })
        .linkDirectionalParticleColor((l) => LINKS[l.kind].color)
        .nodeCanvasObjectMode(() => "after")
        .nodeCanvasObject((n, ctx, scale) => {
          const f = focusSet();
          const dim = f && !f.has(n.id);
          const important = n._match || (f && f.has(n.id)) || n.kind === "class" || n.kind === "interface" || n.kind === "executable" || n.kind === "plugin" || n.kind === "external";
          if (!state.labels || dim) return;
          if (!important && scale < 1.4 && visibleCount > 100) return;
          const label = shortLabel(n);
          const fontSize = Math.max(10 / scale, 2.2);
          ctx.font = `${important ? "600 " : ""}${fontSize}px ui-sans-serif, system-ui, sans-serif`;
          ctx.textAlign = "center"; ctx.textBaseline = "top";
          const r = Math.sqrt(nodeVal(n)) * 4;
          const w = ctx.measureText(label).width;
          ctx.fillStyle = withAlpha(isDark() ? "#15140f" : "#f2efe8", 0.7);
          ctx.fillRect(n.x - w / 2 - 1, n.y + r + 1, w + 2, fontSize + 1);
          ctx.fillStyle = n._match ? "#e8734a" : textColor();
          ctx.fillText(label, n.x, n.y + r + 1.5);
          if (state.selected && state.selected.id === n.id) {
            ctx.beginPath(); ctx.arc(n.x, n.y, r + 2.5, 0, 2 * Math.PI); ctx.strokeStyle = "#e8734a"; ctx.lineWidth = 1.5 / scale; ctx.stroke();
          }
        })
        .d3Force("charge").strength(-90);
      graph.d3Force("link").distance((l) => (l.kind === "member" ? 18 : l.kind === "defines" ? 26 : 42));
      let fitted = false;
      graph.onEngineStop(() => { if (!fitted) { fitted = true; graph.zoomToFit(500, 60); } });
      setTimeout(() => graph.zoomToFit(600, 60), 700);
    } else {
      graph = ForceGraph3D()(container);
      common(graph)
        .width(container.clientWidth).height(container.clientHeight)
        .nodeColor((n) => { const f = focusSet(); const c = nodeColor(n); return f && !f.has(n.id) ? withAlpha(c, 0.12) : c; })
        .nodeOpacity(0.95)
        .linkColor((l) => {
          const f = focusSet(); const c = LINKS[l.kind].color;
          if (!f) return c;
          return f.has(idOf(l.source)) && f.has(idOf(l.target)) ? c : withAlpha(c, 0.04);
        })
        .linkOpacity(isDark() ? 0.45 : 0.4)
        .linkWidth((l) => { const f = focusSet(); return f && f.has(idOf(l.source)) && f.has(idOf(l.target)) ? 1.2 : 0; })
        .linkDirectionalParticleColor((l) => LINKS[l.kind].color)
        .showNavInfo(false);
      graph.d3Force("charge").strength(-120);
      graph.d3Force("link").distance((l) => (l.kind === "member" ? 22 : l.kind === "defines" ? 32 : 55));
      setTimeout(() => graph.zoomToFit(800, 40), 900);
    }
    if (state.frozen) freeze(true);
    updateCounts(data);
    $("#hint").textContent = state.mode === "2d"
      ? "Scroll to zoom · drag background to pan · drag nodes to pin · click a node for details"
      : "Drag to orbit · scroll to zoom · right-drag to pan · click a node for details";
  }

  function refresh() { if (!graph) return; graph.graphData(visibleData()); updateCounts(visibleData()); }
  function repaint() { if (!graph) return; if (state.mode === "2d") graph.nodeColor(graph.nodeColor()); else { graph.nodeColor(graph.nodeColor()); graph.linkColor(graph.linkColor()); graph.linkWidth(graph.linkWidth()); } }
  function freeze(on) {
    state.frozen = on; $("#pause").classList.toggle("active", on); $("#pause").textContent = on ? "Resume" : "Freeze";
    if (!graph) return;
    // stop the simulation ticks but keep rendering (pan/zoom/hover still work)
    if (on) graph.cooldownTicks(0);
    else { graph.cooldownTicks(Infinity); graph.d3ReheatSimulation(); }
  }
  function updateCounts(data) {
    visibleCount = data.nodes.length;
    $("#nodeCount").textContent = `${data.nodes.length} / ${DATA.nodes.length}`;
    $("#linkCount").textContent = `${data.links.length} / ${DATA.links.length}`;
    // per-kind counts
    const nc = {}; data.nodes.forEach((n) => (nc[n.kind] = (nc[n.kind] || 0) + 1));
    document.querySelectorAll("#nodeKinds .cnt").forEach((el) => (el.textContent = nc[el.dataset.kind] || 0));
    const lc = {}; data.links.forEach((l) => (lc[l.kind] = (lc[l.kind] || 0) + 1));
    document.querySelectorAll("#linkKinds .cnt").forEach((el) => (el.textContent = lc[el.dataset.kind] || 0));
  }

  // ------------------------------------------------------------------ selection / details
  function select(n) {
    state.selected = n; state.hover = null;
    const d = $("#details");
    if (!n) { d.hidden = true; if (state.isolate) refresh(); repaint(); return; }
    d.hidden = false;
    const k = KINDS[n.kind] || { label: n.kind, color: "#999" };
    $("#dKind").innerHTML = `<span class="sw" style="background:${nodeColor(n)}"></span>${esc(k.label)}${n.group ? ` · <span style="font-weight:400;text-transform:none;letter-spacing:0">${esc(n.group)}</span>` : ""}`;
    $("#dTitle").textContent = n.fq || n.label;
    const meta = [];
    if (n.signature) meta.push(`<div><code>${esc(n.signature)}</code></div>`);
    if (n.path) meta.push(`<div>📄 <a href="https://github.com/TsukiNi22/context-forge/blob/v1.0.0/${esc(n.path)}" target="_blank" rel="noopener">${esc(n.path)}</a>${n.loc ? ` · ${n.loc} lines` : ""}</div>`);
    if (n.description) meta.push(`<div>${esc(n.description)}</div>`);
    if (n.url) meta.push(`<div>🔗 <a href="${esc(n.url)}" target="_blank" rel="noopener">${esc(n.url)}</a></div>`);
    if (n.macro) meta.push(`<div>${esc(n.macro)} · suite <code>${esc(n.suite)}</code></div>`);
    meta.push(`<div>${n.degree} connection${n.degree === 1 ? "" : "s"}</div>`);
    $("#dMeta").innerHTML = meta.join("");
    const out = DATA.links.filter((l) => l.source === n.id), inn = DATA.links.filter((l) => l.target === n.id);
    const row = (l, other) => { const o = byId.get(other); return `<a href="#" data-id="${esc(other)}"><span class="rel">${esc(LINKS[l.kind].label)}</span>${esc(o.fq || o.label)}</a>`; };
    $("#dLinks").innerHTML =
      (out.length ? `<h5>Outgoing (${out.length})</h5>` + out.map((l) => row(l, l.target)).join("") : "") +
      (inn.length ? `<h5>Incoming (${inn.length})</h5>` + inn.map((l) => row(l, l.source)).join("") : "");
    $("#dLinks").querySelectorAll("a").forEach((a) => a.addEventListener("click", (e) => {
      e.preventDefault(); const t = byId.get(a.dataset.id);
      if (!state.nodeKinds.has(t.kind)) { state.nodeKinds.add(t.kind); syncLegend(); refresh(); }
      select(t); centerOn(t);
    }));
    if (state.isolate) refresh();
    repaint();
  }
  function centerOn(n) {
    if (!graph) return;
    const live = graph.graphData().nodes.find((x) => x.id === n.id);
    if (!live) return;
    if (state.mode === "2d") { graph.centerAt(live.x, live.y, 600); graph.zoom(Math.max(graph.zoom(), 2.2), 600); }
    else { const d = 120, r = 1 + d / Math.hypot(live.x, live.y, live.z); graph.cameraPosition({ x: live.x * r, y: live.y * r, z: live.z * r }, live, 900); }
  }

  // ------------------------------------------------------------------ legend / controls
  function legend(el, dict, set, isLink) {
    el.innerHTML = Object.entries(dict).map(([k, v]) => `<label data-kind="${k}" class="${set.has(k) ? "" : "off"}" title="${esc(v.desc || v.label)}"><input type="checkbox" ${set.has(k) ? "checked" : ""}><span class="sw ${isLink ? "line" : ""}" style="background:${v.color}"></span>${esc(v.label)}<span class="cnt" data-kind="${k}"></span></label>`).join("");
    el.querySelectorAll("label").forEach((lab) => lab.addEventListener("click", (e) => {
      e.preventDefault(); const k = lab.dataset.kind;
      if (e.altKey) { set.clear(); set.add(k); } else if (set.has(k)) set.delete(k); else set.add(k);
      state.groupFilter = null; syncLegend(); refresh(); clearPreset();
    }));
  }
  function syncLegend() {
    document.querySelectorAll("#nodeKinds label").forEach((l) => l.classList.toggle("off", !state.nodeKinds.has(l.dataset.kind)));
    document.querySelectorAll("#linkKinds label").forEach((l) => l.classList.toggle("off", !state.linkKinds.has(l.dataset.kind)));
  }
  function clearPreset() { document.querySelectorAll("#presets button").forEach((b) => b.classList.remove("active")); }
  function applyPreset(name) {
    const p = PRESETS[name]; state.nodeKinds = new Set(p.nodes); state.linkKinds = new Set(p.links); state.groupFilter = p.groupFilter || null;
    clearPreset(); const b = document.querySelector(`#presets [data-preset="${name}"]`); if (b) b.classList.add("active");
    syncLegend(); refresh(); if (graph) setTimeout(() => graph.zoomToFit(600, 60), 500);
  }

  legend($("#nodeKinds"), KINDS, state.nodeKinds, false);
  legend($("#linkKinds"), LINKS, state.linkKinds, true);
  document.querySelectorAll("#presets button").forEach((b) => b.addEventListener("click", () => applyPreset(b.dataset.preset)));
  document.querySelector('#presets [data-preset="overview"]').classList.add("active");

  $("#mode2d").addEventListener("click", () => { if (state.mode !== "2d") { state.mode = "2d"; $("#mode2d").classList.add("active"); $("#mode3d").classList.remove("active"); build(); } });
  $("#mode3d").addEventListener("click", () => { if (state.mode !== "3d") { state.mode = "3d"; $("#mode3d").classList.add("active"); $("#mode2d").classList.remove("active"); build(); } });
  $("#fit").addEventListener("click", () => graph && graph.zoomToFit(600, 60));
  $("#pause").addEventListener("click", () => freeze(!state.frozen));
  let searchTimer;
  $("#search").addEventListener("input", (e) => { clearTimeout(searchTimer); searchTimer = setTimeout(() => { state.search = e.target.value; refresh(); repaint(); }, 180); });
  $("#labels").addEventListener("change", (e) => { state.labels = e.target.checked; repaint(); });
  $("#particles").addEventListener("change", (e) => { state.particles = e.target.checked; if (graph) graph.linkDirectionalParticles(graph.linkDirectionalParticles()); });
  $("#colorGroup").addEventListener("change", (e) => { state.colorGroup = e.target.checked; repaint(); if (state.selected) select(state.selected); });
  $("#isolate").addEventListener("change", (e) => { state.isolate = e.target.checked; refresh(); if (graph) setTimeout(() => graph.zoomToFit(500, 60), 400); });
  $("#detailsClose").addEventListener("click", () => select(null));
  $("#panelToggle").addEventListener("click", () => $("#panel").classList.toggle("open"));
  document.addEventListener("cf-theme-change", () => { if (graph) graph.backgroundColor(bg()); repaint(); });
  window.addEventListener("resize", () => { if (graph) graph.width(container.clientWidth).height(container.clientHeight); });
  document.addEventListener("keydown", (e) => { if (e.key === "Escape") select(null); if (e.key === "/" && document.activeElement !== $("#search")) { e.preventDefault(); $("#search").focus(); } });

  // deep link: graph.html#node=class:forge::Forge
  const h = new URLSearchParams(location.hash.slice(1)).get("node");
  build();
  if (h && byId.has(h)) setTimeout(() => { const n = byId.get(h); if (!state.nodeKinds.has(n.kind)) { state.nodeKinds.add(n.kind); syncLegend(); refresh(); } select(n); setTimeout(() => centerOn(n), 900); }, 1200);
})();
