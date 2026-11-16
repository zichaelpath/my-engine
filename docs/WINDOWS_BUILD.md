# Building MyEngine on Windows

The walkthrough for Zac's Windows machine (the engine's home). Everything is one CMake project; CMake downloads and builds SFML, ImGui, and ImGui-SFML itself on first configure, so there is nothing to install by hand beyond the toolchain.

## 1. Prerequisites

- **Git** (you already have it — the repo lives at `C:\Users\Zac\Documents\my-engine`).
- **CMake 3.28 or newer** (`cmake --version` to check).
- **Visual Studio 2022** (Community edition is fine) with the **"Desktop development with C++"** workload installed. That workload provides the MSVC compiler, the Windows SDK, and everything CMake needs. VS Code stays your editor; Visual Studio itself is only the compiler provider.

## 2. Get the code

```powershell
cd C:\Users\Zac\Documents\my-engine
git pull
```

## 3. Configure (first time, and whenever a `.cpp` is added)

```powershell
cmake -B Build -DCMAKE_BUILD_TYPE=Release
```

The first configure downloads the dependencies (a few minutes, needs internet) into `Build\_deps`. The `Build` folder is disposable and already gitignored — delete it and re-run this step whenever anything seems off.

> Standing rule (learned in Lesson 8): a **new `.cpp` file must be added to `CMakeLists.txt` and the configure re-run**, or the link fails. Editing existing files needs no re-configure.

## 4. Build

```powershell
cmake --build Build --config Release
```

The executables land in `Build\Release\`:

- `MyEngine.exe` — the editor
- `MyRuntime.exe` — the game player (plays whichever project you point it at)
- `MyGL.exe` — the raw-OpenGL 3D host
- `MyTests.exe` — the headless test harness (prints PASS/FAIL lines; the last line is the score)

## 5. Run

Run them **from the repo root** — assets, `project.myengine`, and `engine.ini` all resolve relative to the working directory:

```powershell
cd C:\Users\Zac\Documents\my-engine
.\Build\Release\MyRuntime.exe
```

That plays **Gold Rush** (the repo root is its project). **Temple Run** is the project in `games\temple-run` — pass its folder:

```powershell
.\Build\Release\MyRuntime.exe games\temple-run
```

The editor (`MyEngine.exe`) and the 3D host (`MyGL.exe`) also run from the repo root. Note there are no prebuilt games to download from the repo: `dist\` is gitignored, and the zips Dietrich builds are Linux binaries — on Windows, this build *is* the way to play.

If an executable complains about a missing DLL, the build produced shared libraries: copy the `*.dll` files from `Build\_deps\sfml-build\lib\Release\` next to the `.exe` and run again. (The Linux build links SFML statically and needs none of this; the check is here so the first Windows run is boring either way.)

## 6. Package Gold Rush for Windows

The Linux packager is `tools/package_game.sh`; the Windows equivalent, by hand or in PowerShell from the repo root after a Release build:

```powershell
$stage = "dist\GoldRush"
Remove-Item -Recurse -Force $stage -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force "$stage\assets" | Out-Null
Copy-Item Build\Release\MyRuntime.exe "$stage\GoldRush.exe"
foreach ($f in "crate.tga","step.wav","splash.wav","bump.wav","pickup.wav",
               "win.wav","music.wav","DejaVuSans.ttf","DejaVu-LICENSE.txt") {
    Copy-Item "assets\$f" "$stage\assets\$f"
}
Copy-Item tools\GoldRush-README.txt "$stage\README.txt"
Copy-Item project.myengine "$stage\project.myengine"
"1.8.0" | Set-Content "$stage\VERSION.txt"
```

(Use whatever version `CMakeLists.txt` currently declares — see the Notes. Temple Run stages the same way from `games\temple-run\`: its `project.myengine`, its `chunks.txt`, the shared `assets\`, and `tools\TempleRun-README.txt`.)

Then write the default `engine.ini` into the stage folder (the Linux script's version is in `tools/package_game.sh` — same seven lines), and zip the `dist\GoldRush` folder. Players run `GoldRush.exe` from inside its own folder; the folder is the install.

## Notes

- The engine version every host prints at startup comes from the `project()` line in `CMakeLists.txt` (currently 1.8.0) — bump it there, never in code.
- `imgui.ini` in the repo root is the editor's saved panel layout; it is yours, and it is tracked on purpose.
