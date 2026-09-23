/* context-forge docs — small shared behaviours (theme toggle, copy buttons, tabs, sidebar, anchors) */
(function () {
  // ----- theme
  const root = document.documentElement;
  try {
    const saved = localStorage.getItem("cf-theme");
    if (saved === "dark" || saved === "light") root.setAttribute("data-theme", saved);
  } catch (e) {}
  function currentTheme() {
    const t = root.getAttribute("data-theme");
    if (t) return t;
    return window.matchMedia && window.matchMedia("(prefers-color-scheme: dark)").matches ? "dark" : "light";
  }
  const themeBtn = document.querySelector("[data-theme-toggle]");
  if (themeBtn) {
    themeBtn.addEventListener("click", () => {
      const next = currentTheme() === "dark" ? "light" : "dark";
      root.setAttribute("data-theme", next);
      try { localStorage.setItem("cf-theme", next); } catch (e) {}
      document.dispatchEvent(new CustomEvent("cf-theme-change", { detail: next }));
    });
  }
  window.cfTheme = currentTheme;

  // ----- sidebar (mobile)
  const menuBtn = document.querySelector(".menu-btn");
  const sidebar = document.querySelector(".sidebar");
  if (menuBtn && sidebar) {
    menuBtn.addEventListener("click", () => sidebar.classList.toggle("open"));
    document.addEventListener("click", (e) => {
      if (!sidebar.contains(e.target) && !menuBtn.contains(e.target)) sidebar.classList.remove("open");
    });
  }

  // ----- copy buttons on <pre>
  document.querySelectorAll("pre").forEach((pre) => {
    if (pre.dataset.nocopy !== undefined) return;
    const btn = document.createElement("button");
    btn.className = "copy"; btn.type = "button"; btn.textContent = "Copy";
    btn.addEventListener("click", async () => {
      const code = pre.querySelector("code") || pre;
      try { await navigator.clipboard.writeText(code.innerText.replace(/\n$/, "")); btn.textContent = "Copied"; }
      catch (e) { btn.textContent = "Select & copy"; }
      setTimeout(() => (btn.textContent = "Copy"), 1400);
    });
    pre.appendChild(btn);
  });

  // ----- tabs
  document.querySelectorAll(".tabs").forEach((tabs) => {
    const group = tabs.dataset.tabs;
    const buttons = tabs.querySelectorAll("button");
    const panels = document.querySelectorAll(`.tab-panel[data-tabs="${group}"]`);
    function activate(name) {
      buttons.forEach((b) => b.classList.toggle("active", b.dataset.tab === name));
      panels.forEach((p) => p.classList.toggle("active", p.dataset.tab === name));
      try { localStorage.setItem("cf-tab-" + group, name); } catch (e) {}
    }
    buttons.forEach((b) => b.addEventListener("click", () => activate(b.dataset.tab)));
    let initial = buttons[0] && buttons[0].dataset.tab;
    try { const s = localStorage.getItem("cf-tab-" + group); if (s && [...buttons].some((b) => b.dataset.tab === s)) initial = s; } catch (e) {}
    if (initial) activate(initial);
  });

  // ----- heading anchors + active sidebar link on scroll
  const headings = [...document.querySelectorAll("main h2[id], main h3[id]")];
  headings.forEach((h) => {
    const a = document.createElement("a");
    a.className = "anchor"; a.href = "#" + h.id; a.textContent = "#"; a.setAttribute("aria-label", "Link to this section");
    h.appendChild(a);
  });
  const sideLinks = [...document.querySelectorAll(".sidebar a[href^='#']")];
  if (sideLinks.length && "IntersectionObserver" in window) {
    const map = new Map(sideLinks.map((a) => [a.getAttribute("href").slice(1), a]));
    let current = null;
    const obs = new IntersectionObserver((entries) => {
      entries.forEach((en) => {
        if (en.isIntersecting && map.has(en.target.id)) {
          if (current) current.classList.remove("active");
          current = map.get(en.target.id); current.classList.add("active");
        }
      });
    }, { rootMargin: "-10% 0px -75% 0px", threshold: 0 });
    headings.forEach((h) => map.has(h.id) && obs.observe(h));
  }
})();
