#!/usr/bin/env bash
# Static regression checks for the MT7902 driver patch set.
# Exit 0 = all checks pass.  Exit 1 = at least one check failed.
# Run from the repo root or pass the repo root as $1.
set -euo pipefail

ROOT="${1:-$(cd "$(dirname "$0")/.." && pwd)}"
LATEST="$ROOT/latest"
PASS=0
FAIL=0

check_absent() {
    local pattern="$1" dir="${2:-$LATEST}"
    if grep -rq "$pattern" "$dir" 2>/dev/null; then
        echo "FAIL: '$pattern' found in $dir (should be absent)"
        FAIL=$((FAIL + 1))
    else
        echo "PASS: '$pattern' absent from $dir"
        PASS=$((PASS + 1))
    fi
}

check_present() {
    local pattern="$1" file="$2"
    if grep -q "$pattern" "$file" 2>/dev/null; then
        echo "PASS: '$pattern' found in $file"
        PASS=$((PASS + 1))
    else
        echo "FAIL: '$pattern' not found in $file"
        FAIL=$((FAIL + 1))
    fi
}

echo "=== MT7902 API compatibility checks ==="
echo "Source tree: $LATEST"
echo

# The old page-pool API removed in kernel >=6.17 must not appear anywhere.
check_absent "pp_page_to_nmdesc" "$LATEST"

# The replacement API must be present in mt76.h.
check_present "__netmem_get_pp"        "$LATEST/mt76.h"
check_present "page_to_netmem"         "$LATEST/mt76.h"

# MT7902 PCI device ID must be registered.
check_present "0x7902"                 "$LATEST/mt7921/pci.c"

# Firmware filenames must be defined.
check_present "WIFI_RAM_CODE_MT7902_1.bin"        "$LATEST/mt792x.h"
check_present "WIFI_MT7902_patch_mcu_1_1_hdr.bin" "$LATEST/mt792x.h"

# Makefile must have a build target.
check_present "module_compile"         "$LATEST/Makefile"

echo
echo "Results: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ]
