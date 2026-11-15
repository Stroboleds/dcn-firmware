(function () {
  const ATTRIBUTION = `
    <p><strong>Cell-Mesh · Dual-Cell-Net (DCN)</strong> — proyecto público de firmware mesh europeo (868 MHz).</p>
    <p>
      <strong>Origen del código:</strong> esta web y el árbol de compilación se extraen y adaptan desde
      <a href="https://github.com/xejarque/meshtacore-firmware" rel="noopener" target="_blank">meshtacore-firmware</a>
      (Meshtastic + MeshCore + consola Nexus), con modificaciones DCN en
      <a href="https://github.com/Stroboleds/dcn-firmware" rel="noopener" target="_blank">Stroboleds/dcn-firmware</a>.
      <strong>Meshtastic®</strong> es marca registrada de Meshtastic LLC. No somos afiliados oficiales.
    </p>
    <p>Licencias: <strong>GPL-3.0</strong> / componentes <strong>AGPL-3.0</strong> según upstream.</p>
    <p class="dcn-footer-links">
      <a href="builder.html">Generar firmware</a> ·
      <a href="flasher.html">Flasher</a> ·
      <a href="update.html">Instalar</a> ·
      <a href="docs/index.html">Documentación técnica</a> ·
      <a href="https://github.com/Stroboleds/dcn-firmware/issues">Incidencias</a>
    </p>
  `;

  function injectFooter() {
    const existing = document.querySelector(".dcn-attribution-footer");
    if (existing && existing.dataset.dcn === "1") return;

    document.querySelectorAll(".artisan-site-footer").forEach((el) => el.remove());

    const footer = document.createElement("footer");
    footer.className = "dcn-attribution-footer";
    footer.dataset.dcn = "1";
    footer.innerHTML = ATTRIBUTION;

    const wrap =
      document.querySelector(".landing-wrapper") ||
      document.querySelector(".builder-wrapper") ||
      document.querySelector("main") ||
      document.body;
    wrap.appendChild(footer);
  }

  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", injectFooter);
  } else {
    injectFooter();
  }
})();