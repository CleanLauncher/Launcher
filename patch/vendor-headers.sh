#!/bin/bash
# Vendor file header fixes
# Replaces dash with colon in vendor file copyright headers
# Pattern: "project - see" -> "project: see"

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LIBRARIES_DIR="$REPO_ROOT/libraries"

echo "Applying vendor header fixes..."

# rainbow library
if [ -f "$LIBRARIES_DIR/rainbow/src/rainbow.cpp" ]; then
    sed -i 's|KDE project - see|KDE project: see|g' "$LIBRARIES_DIR/rainbow/src/rainbow.cpp"
    echo "Fixed: rainbow/src/rainbow.cpp"
fi

if [ -f "$LIBRARIES_DIR/rainbow/include/rainbow.h" ]; then
    sed -i 's|KDE project - see|KDE project: see|g' "$LIBRARIES_DIR/rainbow/include/rainbow.h"
    echo "Fixed: rainbow/include/rainbow.h"
fi

# libnbt++ library (already fixed in main repo)
# Pattern: "libnbt++ - A" -> "libnbt++: A"

echo "Vendor header fixes applied."
