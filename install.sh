#!/bin/bash
set -e

VST3_BUILD="build/ChiptuneVST_artefacts/Release/VST3/ChiptuneVST.vst3"
VST3_DEST="$HOME/Library/Audio/Plug-Ins/VST3/ChiptuneVST.vst3"
AU_BUILD="build/ChiptuneVST_artefacts/Release/AU/ChiptuneVST.component"
AU_DEST="$HOME/Library/Audio/Plug-Ins/Components/ChiptuneVST.component"
DB="$HOME/Library/Application Support/Ableton/Live Database/Live-files-1218.db"

cd "$(dirname "$0")"

echo "→ Building..."
cmake --build build --config Release -j$(sysctl -n hw.logicalcpu)

# JUCE adds moduleinfo.json after signing, so we re-sign the build artifact
echo "→ Re-signing build artifact (JUCE invalidates signature with moduleinfo.json)..."
codesign --force --deep --sign - "$VST3_BUILD"

echo "→ Installing VST3..."
cp -r "$VST3_BUILD" "$VST3_DEST"

echo "→ Installing AU..."
cp -r "$AU_BUILD" "$AU_DEST"

# Clear stale Ableton scan state so it rescans cleanly
if [ -f "$DB" ]; then
    echo "→ Clearing Ableton plugin cache entry..."
    sqlite3 "$DB" "DELETE FROM plugin_modules WHERE path LIKE '%ChiptuneVST%';" 2>/dev/null || true
fi

echo "✓ Done — rescan plugins in Ableton (Preferences → Plug-Ins → Rescan)"
