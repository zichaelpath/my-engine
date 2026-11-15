#!/usr/bin/env bash
# Package Gold Rush (the MyEngine Runtime game) as a versioned zip.
#
# Usage: tools/package_game.sh [build-dir]
#   build-dir defaults to BuildRelease; pass BuildLinux to reuse the
#   dev build's cached dependencies (a fresh dir re-downloads SFML
#   and ImGui via FetchContent on first configure).
#
# Output: dist/GoldRush/ (the staged game) and
#         dist/GoldRush-<version>-linux.zip
#
# SFML is linked statically in this project, so the binary needs no
# bundled libraries — a release is the binary, its assets, a default
# engine.ini, and the player readme. Assets resolve relative to the
# working directory, so the game must be launched from its own folder
# (double-click works; the folder IS the install).
set -euo pipefail

VERSION="0.1.0"
BUILD_DIR="${1:-BuildRelease}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
STAGE="$ROOT/dist/GoldRush"

cd "$ROOT"

echo "== Configuring ($BUILD_DIR) =="
cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INCLUDE_PATH="$HOME/workspace/devdeps/include" \
    -DCMAKE_LIBRARY_PATH="$HOME/workspace/devdeps/lib" \
    -DSFML_BUILD_AUDIO=ON -DSFML_BUILD_NETWORK=OFF

echo "== Building MyRuntime =="
cmake --build "$BUILD_DIR" --target MyRuntime -j4

echo "== Staging dist/GoldRush =="
rm -rf "$STAGE"
mkdir -p "$STAGE/assets"
cp "$BUILD_DIR/MyRuntime" "$STAGE/GoldRush"
chmod +x "$STAGE/GoldRush"

# Only the assets the game itself loads (the editor's click.wav and
# the asset-generator scripts stay in the repo).
for f in crate.tga step.wav splash.wav bump.wav pickup.wav win.wav \
         music.wav DejaVuSans.ttf DejaVu-LICENSE.txt; do
    cp "assets/$f" "$STAGE/assets/$f"
done

cat > "$STAGE/engine.ini" <<'EOF'
windowWidth=800
windowHeight=600
seed=12345
noiseScale=0.01
playerSpeed=200
volume=80
music=1
EOF

cp tools/GoldRush-README.txt "$STAGE/README.txt"
echo "$VERSION" > "$STAGE/VERSION.txt"

echo "== Zipping =="
cd "$ROOT/dist"
rm -f "GoldRush-$VERSION-linux.zip"
zip -r "GoldRush-$VERSION-linux.zip" GoldRush > /dev/null
echo "Done: dist/GoldRush-$VERSION-linux.zip"
