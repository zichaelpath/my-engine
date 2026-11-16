// MyTests — the engine's headless test harness (Lesson 45).
//
// Phase 1's checks lived in throwaway probes (the Lesson 38 grid
// checksum, the Lesson 44 flip measurement). This is those probes
// grown up: one executable, one command, a line per check, and an
// exit code CI can read. Checks use the engine's own instruments —
// tile data, scene files, project files — never screenshots.
//
// A check that cannot run in this environment (the render check on
// a machine with no GL context) reports SKIP, never a fake PASS
// and never an environmental FAIL.

#include "Config.hpp"
#include "Easing.hpp"
#include "Game.hpp"
#include "Project.hpp"
#include "Tween.hpp"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

namespace
{
int g_passed = 0;
int g_failed = 0;
int g_skipped = 0;

void pass(const std::string& name)
{
    ++g_passed;
    std::printf("[PASS] %s\n", name.c_str());
}

void fail(const std::string& name, const std::string& detail)
{
    ++g_failed;
    std::printf("[FAIL] %s — %s\n", name.c_str(), detail.c_str());
}

void skip(const std::string& name, const std::string& reason)
{
    ++g_skipped;
    std::printf("[SKIP] %s — %s\n", name.c_str(), reason.c_str());
}

void check(bool ok, const std::string& name, const std::string& detail = "")
{
    if (ok)
        pass(name);
    else
        fail(name, detail.empty() ? "condition false" : detail);
}

// FNV-1a over the tile grid, row-major. The baseline below is the
// fingerprint of the shipped 1.0 world (seed 12345): if generation
// ever changes silently, this check is the alarm.
std::uint32_t gridChecksum(const Game& game)
{
    std::uint32_t hash = 2166136261u;
    auto mix = [&hash](std::uint32_t byte) {
        hash ^= byte;
        hash *= 16777619u;
    };
    mix(game.tilesX());
    mix(game.tilesY());
    for (int ty = 0; ty < game.tilesY(); ++ty)
        for (int tx = 0; tx < game.tilesX(); ++tx)
            mix(static_cast<std::uint32_t>(game.tileAt(tx, ty) + 1));
    return hash;
}

constexpr std::uint32_t kBaselineChecksum = 936912399u; // 0x37d8260f

std::filesystem::path tempPath(const std::string& file)
{
    return std::filesystem::temp_directory_path() / file;
}

void writeFile(const std::filesystem::path& path, const std::string& text)
{
    std::ofstream out(path);
    out << text;
}
} // namespace

int main()
{
    // A harness that crashes must not take its results with it:
    // every line hits the terminal the moment it is printed.
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("MyTests — MyEngine headless checks\n");

#if defined(__linux__)
    // The engine's core owns GL resources (the sprite sheet is a
    // texture), so even the non-render checks need a display to
    // construct a Game. Without one SFML aborts mid-construction —
    // detect it up front and skip the suite honestly instead.
    if (std::getenv("DISPLAY") == nullptr)
    {
        skip("the whole suite", "no DISPLAY — run under xvfb-run");
        std::printf("RESULT: %d passed, %d failed, %d skipped\n", g_passed,
                    g_failed, g_skipped);
        return 0;
    }
#endif

    // --- World generation: determinism ------------------------------
    {
        Game a;
        Game b;
        bool identical = a.tilesX() == b.tilesX() && a.tilesY() == b.tilesY();
        for (int ty = 0; identical && ty < a.tilesY(); ++ty)
            for (int tx = 0; tx < a.tilesX(); ++tx)
                if (a.tileAt(tx, ty) != b.tileAt(tx, ty))
                    identical = false;
        check(identical, "determinism: two fresh worlds are cell-identical");

        const std::uint32_t sum = gridChecksum(a);
        check(sum == kBaselineChecksum,
              "determinism: grid checksum matches the 1.0 baseline",
              "got " + std::to_string(sum));

        Game other;
        other.setSeed(777);
        int differing = 0;
        for (int ty = 0; ty < a.tilesY(); ++ty)
            for (int tx = 0; tx < a.tilesX(); ++tx)
                if (a.tileAt(tx, ty) != other.tileAt(tx, ty))
                    ++differing;
        check(differing > 100, "generation: a different seed is a different world",
              "only " + std::to_string(differing) + " cells differ");
    }

    // --- Scene save / load round-trip -------------------------------
    {
        Game a;
        a.spawnEntity("TestCrate", {123.5f, 456.25f}, {40.f, 40.f},
                      sf::Color(10, 20, 30));
        const auto sceneFile = tempPath("mytests_scene.txt");
        const bool saved = a.saveScene(sceneFile.string());
        Game b;
        const bool loaded = saved && b.loadScene(sceneFile.string());
        check(saved && loaded, "scene: save then load succeeds");
        if (loaded)
        {
            bool same = a.entities().size() == b.entities().size();
            for (std::size_t i = 0; same && i < a.entities().size(); ++i)
            {
                const Entity& ea = a.entities()[i];
                const Entity& eb = b.entities()[i];
                same = ea.name == eb.name && ea.sprite == eb.sprite &&
                       ea.solid == eb.solid &&
                       std::abs(ea.position.x - eb.position.x) < 0.001f &&
                       std::abs(ea.position.y - eb.position.y) < 0.001f &&
                       std::abs(ea.size.x - eb.size.x) < 0.001f &&
                       std::abs(ea.size.y - eb.size.y) < 0.001f;
            }
            check(same, "scene: round-trip preserves every entity");
        }
        std::filesystem::remove(sceneFile);
    }

    // --- Project files ----------------------------------------------
    {
        const auto good = tempPath("mytests_project.myengine");
        writeFile(good, "# test card\nformat=1\nname=Test Card\nseed=777\nscene=foo.txt\n");
        Project p;
        const bool ok = loadProject(good.string(), p);
        check(ok && p.name == "Test Card" && p.seed == 777u &&
                  p.startScene == "foo.txt" && p.formatVersion == 1,
              "project: a valid file loads its fields");
        std::filesystem::remove(good);

        const auto bad = tempPath("mytests_project_bad.myengine");
        writeFile(bad, "format=99\nname=Future\n");
        Project sentinel;
        sentinel.name = "Sentinel";
        sentinel.seed = 42;
        const bool rejected = !loadProject(bad.string(), sentinel);
        check(rejected && sentinel.name == "Sentinel" && sentinel.seed == 42u,
              "project: a future format is refused, output untouched");
        std::filesystem::remove(bad);

        Project unused;
        check(!loadProject(tempPath("mytests_no_such_file.myengine").string(), unused),
              "project: a missing file is a clean false");
    }

    // --- Config -------------------------------------------------------
    {
        const auto file = tempPath("mytests_engine.ini");
        writeFile(file, "windowWidth=1024\nthis line is malformed\nvolume=55\n");
        Config config;
        config.setDefault("windowWidth", "800");
        config.setDefault("volume", "80");
        config.setDefault("music", "1");
        const bool loaded = config.load(file.string());
        check(loaded && config.getInt("windowWidth", 0) == 1024 &&
                  config.getInt("volume", 0) == 55 &&
                  config.getInt("music", 0) == 1,
              "config: file values land, defaults fill the gaps, junk is skipped");
        std::filesystem::remove(file);
    }

    // --- Easing & tweens (Lesson 49) ----------------------------------
    {
        const ease::Fn curves[] = {ease::linear,    ease::inQuad,  ease::outQuad,
                                   ease::inOutQuad, ease::outCubic, ease::outBack,
                                   ease::outBounce};
        bool endpoints = true;
        for (ease::Fn f : curves)
            endpoints = endpoints && std::abs(f(0.f)) < 1e-6f &&
                        std::abs(f(1.f) - 1.f) < 1e-6f;
        check(endpoints, "easing: every curve starts at 0 and lands on 1");
        check(ease::outBack(0.5f) > 1.0f,
              "easing: outBack overshoots mid-flight (the pop)");
        bool monotonic = true;
        for (float t = 0.1f; t <= 1.0f; t += 0.1f)
            monotonic = monotonic && ease::outQuad(t) >= ease::outQuad(t - 0.1f);
        check(monotonic, "easing: outQuad never runs backwards");

        FloatTween tween;
        tween.start(10.f, 20.f, 2.0f, ease::linear);
        const bool startsAtFrom = std::abs(tween.value() - 10.f) < 1e-6f;
        tween.update(1.0f);
        const bool midIsMid = std::abs(tween.value() - 15.f) < 1e-4f;
        tween.update(5.0f); // far past the end: must clamp, not drift
        check(startsAtFrom && midIsMid && tween.done() &&
                  std::abs(tween.value() - 20.f) < 1e-6f,
              "tween: from, midpoint, clamped settle at to");

        Game game;
        game.newRun(game.seed());
        const bool popping = game.playerSpawnScale() < 1.f;
        game.update(1.0f); // the pop lasts 0.35 s
        check(popping && std::abs(game.playerSpawnScale() - 1.f) < 1e-6f,
              "tween: a new run pops the player in, then settles at 1");
    }

    // --- The Lesson 44 regression: a flipped sprite stays home -------
#if defined(__linux__)
    if (std::getenv("DISPLAY") == nullptr)
    {
        skip("render: flipped sprite aligns with its hitbox",
             "no DISPLAY in this environment (run under xvfb-run)");
    }
    else
#endif
    {
        Game game;
        game.showHitboxes = true;
        game.entities()[0].flipX = true; // face left, standing at spawn
        sf::RenderTexture rt({800, 600});
        if (rt.getSize() != sf::Vector2u(800, 600))
        {
            skip("render: flipped sprite aligns with its hitbox",
                 "no render context in this environment");
        }
        else
        {
            rt.setView(sf::View({400.f, 300.f}, {800.f, 600.f}));
            game.render(rt);
            rt.display();
            const sf::Image img = rt.getTexture().copyToImage();
            if (img.getSize() != sf::Vector2u(800, 600))
            {
                skip("render: flipped sprite aligns with its hitbox",
                     "render readback unavailable in this environment");
            }
            else
            {
                unsigned minX = 800, maxX = 0;
                for (unsigned y = 0; y < 600; ++y)
                    for (unsigned x = 0; x < 800; ++x)
                    {
                        const sf::Color c = img.getPixel({x, y});
                        if (c.r == 80 && c.g == 200 && c.b == 120)
                        {
                            minX = std::min(minX, x);
                            maxX = std::max(maxX, x);
                        }
                    }
                // Entity box at spawn is x 375..425; the hood must be
                // inside it (pre-fix it sat at 438..462, fully outside).
                check(maxX >= minX && minX >= 375 && maxX <= 425,
                      "render: flipped sprite aligns with its hitbox",
                      "hood span " + std::to_string(minX) + ".." +
                          std::to_string(maxX) + ", entity box 375..425");
            }
        }
    }

    std::printf("RESULT: %d passed, %d failed, %d skipped\n", g_passed, g_failed,
                g_skipped);
    return g_failed == 0 ? 0 : 1;
}
