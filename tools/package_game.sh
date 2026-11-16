#!/usr/bin/env bash
# Package a MyEngine Runtime game as a versioned zip.
#
# Usage: tools/package_game.sh [build-dir] [game]
#   build-dir defaults to BuildRelease; pass BuildLinux to reuse the
#   dev build's cached dependencies (a fresh dir re-downloads SFML
#   and ImGui via FetchContent on first configure).
#   game is "goldrush" (default) or "templerun" — one runtime binary
#   ships both games; what differs is the project staged beside it.
#
# Output: dist/<Game>/ (the staged game) and
#         dist/<Game>-<version>-linux.zip
#
# SFML is linked statically in this project, so the binary needs no
# bundled libraries — a release is the binary, its assets, a default
# engine.ini, the project files, and the player readme. Assets resolve
# relative to the working directory, so the game must be launched
# from its own folder (double-click works; the folder IS the install).
set -euo pipefail

VERSION="1.5.0"
BUILD_DIR="${1:-BuildRelease}"
GAME="${2:-goldrush}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"

case "$GAME" in
    goldrush)
        STAGE_NAME="GoldRush"
        PROJECT_SRC="$ROOT/project.myengine"
        EXTRA_FILES=""
        README_SRC="tools/GoldRush-README.txt"
        ;;
    templerun)
        STAGE_NAME="TempleRun"
        PROJECT_SRC="$ROOT/games/temple-run/project.myengine"
        EXTRA_FILES="$ROOT/games/temple-run/chunks.txt"
        README_SRC="tools/TempleRun-README.txt"
        ;;
    *)
        echo "unknown game '$GAME' (goldrush | templerun)" >&2
        exit 1
        ;;
esac
STAGE="$ROOT/dist/$STAGE_NAME"

cd "$ROOT"

echo "== Configuring ($BUILD_DIR) =="
cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INCLUDE_PATH="$HOME/workspace/devdeps/include" \
    -DCMAKE_LIBRARY_PATH="$HOME/workspace/devdeps/lib" \
    -DSFML_BUILD_AUDIO=ON -DSFML_BUILD_NETWORK=OFF

echo "== Building MyRuntime =="
cmake --build "$BUILD_DIR" --target MyRuntime -j4

echo "== Staging dist/$STAGE_NAME =="
rm -rf "$STAGE"
mkdir -p "$STAGE/assets"
cp "$BUILD_DIR/MyRuntime" "$STAGE/$STAGE_NAME"
chmod +x "$STAGE/$STAGE_NAME"

# Only the assets the game itself loads (the editor's click.wav and
# the asset-generator scripts stay in the repo). Both games load the
# same shared set — sprites, synthesized sounds, music, the font.
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

cp "$README_SRC" "$STAGE/README.txt"
cp "$PROJECT_SRC" "$STAGE/project.myengine"
for f in $EXTRA_FILES; do
    cp "$f" "$STAGE/"
done
echo "$VERSION" > "$STAGE/VERSION.txt"

echo "== Zipping =="
cd "$ROOT/dist"
rm -f "$STAGE_NAME-$VERSION-linux.zip"
zip -r "$STAGE_NAME-$VERSION-linux.zip" "$STAGE_NAME" > /dev/null
echo "Done: dist/$STAGE_NAME-$VERSION-linux.zip"
