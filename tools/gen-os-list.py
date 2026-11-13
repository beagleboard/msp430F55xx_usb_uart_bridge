#!/bin/python3

import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path

FIRMWARE = Path("usb_uart_bridge.hex.xz")
OUTPUT = Path("os_list.json")

ICON = "https://upload.wikimedia.org/wikipedia/commons/6/67/USB_icon.svg"


def get_file_sha256(file_path: Path) -> str:
    with open(file_path, "rb") as f:
        digest = hashlib.file_digest(f, "sha256")
    return digest.hexdigest()


def get_xz_extract_size(file_path: Path) -> int:
    out = subprocess.run(
        ["xz", "--robot", "--list", file_path],
        capture_output=True,
        text=True,
        check=True,
    ).stdout
    return int(out.splitlines()[-1].split("\t")[4])


if __name__ == "__main__":
    assert FIRMWARE.is_file(), f"{FIRMWARE} not found. Run make and xz first."

    parser = argparse.ArgumentParser()
    parser.add_argument("tag", help="Release tag for constructing urls", type=str)
    args = parser.parse_args()

    release_date = datetime.datetime.now().strftime("%Y-%m-%d")
    base_url = f"https://github.com/beagleboard/msp430F55xx_usb_uart_bridge/releases/download/{args.tag}"

    with open(OUTPUT, "w") as fp:
        json.dump(
            {
                "imager": {},
                "os_list": [
                    {
                        "name": "MSP430 Firmware",
                        "description": "Firmware for the built-in USB to UART Bridge",
                        "icon": ICON,
                        "flasher": "Msp430Usb",
                        "subitems": [
                            {
                                "name": "USB To UART Bridge",
                                "description": "MSP430 USB to UART firmware with Windows Support",
                                "icon": ICON,
                                "url": f"{base_url}/{FIRMWARE.name}",
                                "image_download_size": FIRMWARE.stat().st_size,
                                "image_download_sha256": get_file_sha256(FIRMWARE),
                                "extract_size": get_xz_extract_size(FIRMWARE),
                                "release_date": release_date,
                                "devices": ["beagleconnect-freedom"],
                            }
                        ],
                    }
                ],
            },
            fp,
        )
