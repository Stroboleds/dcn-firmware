// DCN / Cell-Mesh: modo claro/oscuro + paleta de acento (por defecto azul eléctrico)
(function () {
  const MODE_KEY = "dcn_theme_mode";
  const PALETTE_KEY = "dcn_palette";
  const LEGACY_KEY = "meshtacore_theme";

  const PALETTES = {
    "electric-blue": {
      label: "Azul eléctrico",
      light: {
        "--accent": "#0077FF",
        "--accent-hover": "#005ECC",
        "--accent-light": "rgba(0, 119, 255, 0.12)",
        "--input-focus": "#0077FF",
      },
      dark: {
        "--accent": "#3D9BFF",
        "--accent-hover": "#6BB3FF",
        "--accent-light": "rgba(61, 155, 255, 0.18)",
        "--input-focus": "#3D9BFF",
      },
    },
    "dcn-teal": {
      label: "DCN teal",
      light: {
        "--accent": "#0D7C7C",
        "--accent-hover": "#0A6363",
        "--accent-light": "rgba(13, 124, 124, 0.12)",
        "--input-focus": "#0D7C7C",
      },
      dark: {
        "--accent": "#2DD4BF",
        "--accent-hover": "#5EEAD4",
        "--accent-light": "rgba(45, 212, 191, 0.15)",
        "--input-focus": "#2DD4BF",
      },
    },
    "cell-orange": {
      label: "Cell naranja",
      light: {
        "--accent": "#E85D04",
        "--accent-hover": "#C44D03",
        "--accent-light": "rgba(232, 93, 4, 0.12)",
        "--input-focus": "#E85D04",
      },
      dark: {
        "--accent": "#FB923C",
        "--accent-hover": "#FDBA74",
        "--accent-light": "rgba(251, 146, 60, 0.18)",
        "--input-focus": "#FB923C",
      },
    },
    violet: {
      label: "Violeta",
      light: {
        "--accent": "#7C3AED",
        "--accent-hover": "#6D28D9",
        "--accent-light": "rgba(124, 58, 237, 0.12)",
        "--input-focus": "#7C3AED",
      },
      dark: {
        "--accent": "#A78BFA",
        "--accent-hover": "#C4B5FD",
        "--accent-light": "rgba(167, 139, 250, 0.18)",
        "--input-focus": "#A78BFA",
      },
    },
    "classic-green": {
      label: "Verde clásico",
      light: {
        "--accent": "#1a7f37",
        "--accent-hover": "#15652a",
        "--accent-light": "rgba(26, 127, 55, 0.1)",
        "--input-focus": "#0969da",
      },
      dark: {
        "--accent": "#2ea043",
        "--accent-hover": "#3fb950",
        "--accent-light": "rgba(46, 160, 67, 0.15)",
        "--input-focus": "#58a6ff",
      },
    },
  };

  function readMode() {
    const saved = localStorage.getItem(MODE_KEY) || localStorage.getItem(LEGACY_KEY);
    if (saved === "dark" || saved === "light") return saved;
    return "light";
  }

  function readPalette() {
    const saved = localStorage.getItem(PALETTE_KEY);
    if (saved && PALETTES[saved]) return saved;
    return "electric-blue";
  }

  function applyVars(mode, paletteId) {
    const palette = PALETTES[paletteId] || PALETTES["electric-blue"];
    const vars = palette[mode] || palette.light;
    const root = document.documentElement;
    root.setAttribute("data-theme", mode);
    root.setAttribute("data-palette", paletteId);
    Object.entries(vars).forEach(([k, v]) => root.style.setProperty(k, v));
  }

  function wirePaletteSelect(select) {
    select.innerHTML = "";
    Object.entries(PALETTES).forEach(([id, meta]) => {
      const opt = document.createElement("option");
      opt.value = id;
      opt.textContent = meta.label;
      select.appendChild(opt);
    });
    select.value = readPalette();
    select.addEventListener("change", () => {
      try {
        localStorage.setItem(PALETTE_KEY, select.value);
      } catch (e) {
        /* ignore */
      }
      applyVars(readMode(), select.value);
    });
  }

  function enhanceNav() {
    document.querySelectorAll(".site-nav .nav-links").forEach((ul) => {
      if (ul.querySelector(".palette-picker-wrap")) return;
      const li = document.createElement("li");
      li.className = "palette-picker-wrap";
      li.innerHTML =
        '<label for="dcn-palette-select">Tema</label><select id="dcn-palette-select" class="palette-picker" aria-label="Paleta de color"></select>';
      const toggleLi = ul.querySelector(".theme-toggle-btn")?.closest("li");
      if (toggleLi) ul.insertBefore(li, toggleLi);
      else ul.appendChild(li);
      const select = li.querySelector("select");
      if (select) wirePaletteSelect(select);
    });
  }

  const origToggle = window.toggleTheme;
  window.toggleTheme = function () {
    if (typeof origToggle === "function") origToggle();
    const mode =
      document.documentElement.getAttribute("data-theme") || readMode();
    try {
      localStorage.setItem(MODE_KEY, mode);
    } catch (e) {
      /* ignore */
    }
    applyVars(mode, readPalette());
  };

  applyVars(readMode(), readPalette());

  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", enhanceNav);
  } else {
    enhanceNav();
  }

  document.addEventListener("DOMContentLoaded", () => {
    applyVars(readMode(), readPalette());
  });
})();