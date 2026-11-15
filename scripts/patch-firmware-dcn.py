#!/usr/bin/env python3
"""Parches mínimos en el submódulo firmware para CI DCN (sin fork upstream)."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
WIFI = ROOT / "firmware" / "src" / "mesh" / "wifi" / "WiFiAPClient.cpp"


def patch_wifi_mdns() -> None:
    if not WIFI.is_file():
        return
    text = WIFI.read_text(encoding="utf-8")
    needle = """        // start mdns
        if (!MDNS.begin("Meshtastic")) {
            LOG_ERROR("mDNS setup failed");
        } else {
            LOG_INFO("mDNS Host: Meshtastic.local");
            MDNS.addService("meshtastic", "tcp", SERVER_API_DEFAULT_PORT);
"""
    if needle not in text:
        return
    if "#if !MESHTASTIC_EXCLUDE_WEBSERVER\n        // start mdns" in text:
        return
    wrapped = """#if !MESHTASTIC_EXCLUDE_WEBSERVER
        // start mdns
        if (!MDNS.begin("Meshtastic")) {
            LOG_ERROR("mDNS setup failed");
        } else {
            LOG_INFO("mDNS Host: Meshtastic.local");
            MDNS.addService("meshtastic", "tcp", SERVER_API_DEFAULT_PORT);
"""
    text = text.replace(needle, wrapped, 1)
    text = text.replace(
        """            LOG_INFO("Obtained IP address: %s", WiFi.localIP().toString().c_str());
#endif
        }

#ifndef DISABLE_NTP""",
        """            LOG_INFO("Obtained IP address: %s", WiFi.localIP().toString().c_str());
#endif
        }
#endif

#ifndef DISABLE_NTP""",
        1,
    )

    eth_block = """            MDNS.end();
            if (MDNS.begin("Meshtastic")) {
                MDNS.addService("meshtastic", "tcp", SERVER_API_DEFAULT_PORT);
                MDNS.addServiceTxt("meshtastic", "tcp", "shortname", String(owner.short_name));
                MDNS.addServiceTxt("meshtastic", "tcp", "id", String(nodeDB->getNodeId().c_str()));
                MDNS.addServiceTxt("meshtastic", "tcp", "pio_env", optstr(APP_ENV));
            }
"""
    if eth_block in text and "#if !MESHTASTIC_EXCLUDE_WEBSERVER\n            MDNS.end();" not in text:
        text = text.replace(
            eth_block,
            "#if !MESHTASTIC_EXCLUDE_WEBSERVER\n" + eth_block + "#endif\n",
            1,
        )

    include_block = "#ifdef ARCH_ESP32\n#if !MESHTASTIC_EXCLUDE_WEBSERVER\n#include \"mesh/http/WebServer.h\"\n#endif\n#include <ESPmDNS.h>"
    if "#include <ESPmDNS.h>" in text and include_block not in text:
        text = text.replace(
            "#ifdef ARCH_ESP32\n#if !MESHTASTIC_EXCLUDE_WEBSERVER\n#include \"mesh/http/WebServer.h\"\n#endif\n#include <ESPmDNS.h>",
            "#ifdef ARCH_ESP32\n#if !MESHTASTIC_EXCLUDE_WEBSERVER\n#include \"mesh/http/WebServer.h\"\n#include <ESPmDNS.h>\n#endif",
            1,
        )

    WIFI.write_text(text, encoding="utf-8")
    print("Patched WiFiAPClient mDNS guards for MESHTASTIC_EXCLUDE_WEBSERVER")


def patch_radio_friend() -> None:
    rl = ROOT / "firmware" / "src" / "mesh" / "RadioLibInterface.h"
    if not rl.is_file():
        return
    text = rl.read_text(encoding="utf-8")
    if "friend class DcnSpectrumModule" in text:
        return
    text = text.replace(
        "namespace meshtastic { class NexusModule; }",
        "namespace meshtastic { class NexusModule; }\nclass DcnSpectrumModule;",
        1,
    )
    text = text.replace(
        "    friend class meshtastic::NexusModule;",
        "    friend class meshtastic::NexusModule;\n    friend class DcnSpectrumModule;",
        1,
    )
    rl.write_text(text, encoding="utf-8")
    print("RadioLibInterface: friend DcnSpectrumModule")


def main() -> None:
    patch_wifi_mdns()
    patch_radio_friend()


if __name__ == "__main__":
    main()