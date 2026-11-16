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

#include "Camera.hpp"
#include "Config.hpp"
#include "Entity.hpp"
#include "Easing.hpp"
#include "Game.hpp"
#include "Particles.hpp"
#include "Project.hpp"
#include "ScriptHost.hpp"
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

Entity& playerOf(Game& g)
{
    for (auto& e : g.entities())
        if (e.playerControlled)
            return e;
    return g.entities().front(); // unreachable: a player exists
}

// Stages a clean perch for the physics tests (Lessons 54-56):
// finds the skip-th 2x2 patch of non-solid tiles with clear
// neighbours, moves the crate onto it, drops the player, and
// verifies he comes to rest at exactly the crate's top. Since
// Lesson 56 the tile grid itself is collision, so a perch is
// only honest when the geology around it is verified clear —
// and a terrain block squatting on the patch (invisible to the
// tile scan) shows up as a wrong rest height, which is exactly
// what the verification catches. Returns the rest height.
bool stageCratePerch(Game& g, int skip, float& restY,
                     sf::Vector2f* perchPos = nullptr)
{
    int seen = -1;
    for (int ty = 8; ty + 1 < g.tilesY(); ++ty) // deep enough to jump in
        for (int tx = 1; tx + 2 < g.tilesX(); ++tx)
        {
            bool clear = true;
            for (int dy = -6; dy <= 1 && clear; ++dy) // headroom for a full jump
                for (int dx = -1; dx <= 2 && clear; ++dx)
                    if (g.tileSolid(g.tileAt(tx + dx, ty + dy)))
                        clear = false;
            if (!clear || ++seen != skip)
                continue;
            for (auto& e : g.entities())
                if (e.name == "Crate")
                    e.position = {tx * 20.f, ty * 20.f};
            if (perchPos)
                *perchPos = {tx * 20.f, ty * 20.f};
            Entity& p = playerOf(g);
            p.moveModel = MoveModel::Platformer;
            p.position = {tx * 20.f + 5.f, ty * 20.f - 70.f};
            p.velocity = {0.f, 0.f};
            for (int i = 0; i < 240 && !p.grounded; ++i)
                g.update(1.f / 60.f);
            restY = p.position.y;
            return p.grounded &&
                   std::abs(restY - (ty * 20.f - 50.f)) < 0.01f;
        }
    return false;
}

// Tries successive patches on the same game until one stages.
bool standOnPerch(Game& g, float& restY)
{
    for (int skip = 0; skip < 8; ++skip)
        if (stageCratePerch(g, skip, restY))
            return true;
    return false;
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

    // --- Particles (Lesson 50) -----------------------------------------
    {
        Emitter burst;
        burst.count = 12;

        Game game;
        const bool startsEmpty = game.particleCount() == 0;
        game.spawnParticles(burst);
        const bool spawned = game.particleCount() == 12;
        game.update(2.0f); // every lifespan in the default emitter is < 2 s
        check(startsEmpty && spawned && game.particleCount() == 0,
              "particles: spawn fills the pool, time empties it");

        game.feel.particlesEnabled = false;
        game.spawnParticles(burst);
        check(game.particleCount() == 0,
              "particles: the feel toggle silences spawns");

        ParticleSystem a, b;
        Random rngA(999), rngB(999);
        a.spawn(burst, rngA);
        b.spawn(burst, rngB);
        bool same = a.particles().size() == b.particles().size();
        for (std::size_t i = 0; same && i < a.particles().size(); ++i)
            same = a.particles()[i].velocity == b.particles()[i].velocity &&
                   a.particles()[i].life == b.particles()[i].life;
        check(same, "particles: same seed, same burst (the fx stream is honest)");
    }

    // --- Camera feel (Lesson 51) ---------------------------------------
    {
        Game game;
        const bool calmAtRest = game.shakeOffset() == sf::Vector2f(0.f, 0.f);
        game.addTrauma(1.f);
        float peak = 0.f;
        bool bounded = true;
        for (int i = 0; i < 60; ++i)
        {
            game.update(1.f / 60.f);
            const sf::Vector2f o = game.shakeOffset();
            peak = std::max(peak, std::abs(o.x));
            bounded = bounded && std::abs(o.x) <= 14.01f && std::abs(o.y) <= 11.21f;
        }
        game.update(2.0f); // trauma decays at 1.4/s: long gone
        check(calmAtRest && peak > 4.f && bounded && game.trauma() == 0.f &&
                  game.shakeOffset() == sf::Vector2f(0.f, 0.f),
              "shake: trauma rattles within bounds, then decays to calm");

        Game run;
        run.newRun(run.seed());
        run.update(0.5f);
        const float frozenAt = run.runTime();
        run.hitStop(0.2f);
        run.update(0.1f);
        const bool froze = run.isHitStopped() && run.runTime() == frozenAt;
        run.update(0.15f); // 0.1 s finishes the stop, 0.05 s of world follows
        check(froze && !run.isHitStopped() && run.runTime() > frozenAt,
              "hit-stop: the run clock freezes, then resumes");

        Camera cam;
        cam.center = {400.f, 300.f};
        const sf::Vector2f world(1600.f, 1200.f), canvas(800.f, 600.f);
        cam.follow({400.f, 300.f}, {200.f, 0.f}, 1.f / 60.f, world, canvas);
        const bool glides = cam.center.x > 400.5f && cam.center.x < 436.f;
        for (int i = 0; i < 240; ++i)
            cam.follow({400.f, 300.f}, {200.f, 0.f}, 1.f / 60.f, world, canvas);
        check(glides && std::abs(cam.center.x - 436.f) < 1.5f,
              "camera: follow glides to the lookahead point, capped at 36");
    }

    // --- Gold Rush, juiced (Lesson 52) ----------------------------------
    {
        Game game;
        game.onGoldCollected({400.f, 300.f});
        const bool firstBurst = game.collectStreak() == 1 &&
                                game.particleCount() == 12 && game.ringCount() == 1;
        game.onGoldCollected({400.f, 300.f});
        const bool crescendo = game.collectStreak() == 2 &&
                               game.particleCount() == 26; // 12 + 14
        game.update(1.3f); // the streak window is 1.2 s
        check(firstBurst && crescendo && game.collectStreak() == 0,
              "juice: pickups escalate a streak, then the window forgets");

        Game win;
        win.onRunWon();
        const bool staged = win.isHitStopped() && win.trauma() >= 0.35f &&
                            win.ringCount() >= 1 && win.particleCount() == 0;
        win.update(0.2f); // the stop (0.12 s) releases; the fountain starts
        check(staged && win.particleCount() > 0,
              "juice: the win stages freeze first, fountain after");

        // The water-exit drip, driven honestly: settle on land (any
        // dot there is collected and its burst expires), wade in,
        // let the splash die, then step out and count the drip.
        Game wade;
        sf::Vector2f landPos, waterPos;
        bool foundLand = false, foundWater = false;
        for (int ty = 0; ty < wade.tilesY() && !(foundLand && foundWater); ++ty)
            for (int tx = 0; tx < wade.tilesX(); ++tx)
            {
                const sf::Vector2f centre(tx * 20.f + 10.f, ty * 20.f + 10.f);
                if (!foundLand && wade.tileAt(tx, ty) == tiles::Grass)
                {
                    landPos = centre;
                    foundLand = true;
                }
                if (!foundWater && wade.tileAt(tx, ty) == tiles::Water)
                {
                    waterPos = centre;
                    foundWater = true;
                }
            }
        // playerTile() reads the hitbox CENTRE: place that point.
        auto placeFeet = [&wade](sf::Vector2f feet) {
            for (auto& e : wade.entities())
                if (e.playerControlled)
                {
                    const sf::Vector2f hb =
                        e.hitboxSize.x > 0.f ? e.hitboxSize : e.size;
                    e.position = feet - e.hitboxOffset - hb / 2.f;
                }
        };
        bool dripOk = false;
        if (foundLand && foundWater)
        {
            placeFeet(landPos);
            for (int i = 0; i < 130; ++i)
                wade.update(1.f / 60.f);
            placeFeet(waterPos);
            wade.update(1.f / 60.f);
            for (int i = 0; i < 130; ++i)
                wade.update(1.f / 60.f);
            const std::size_t before = wade.particleCount();
            placeFeet(landPos);
            wade.update(1.f / 60.f);
            dripOk = wade.particleCount() == before + 6;
        }
        check(dripOk, "juice: stepping out of water drips (6, exactly)");
    }

    // --- Feel is data (Lesson 53) ---------------------------------------
    {
        // The master switch silences the whole layer at once.
        Game off;
        off.feel.enabled = false;
        off.newRun(off.seed());
        off.addTrauma(0.8f);
        off.hitStop(0.2f);
        off.onGoldCollected({400.f, 300.f});
        check(off.playerSpawnScale() == 1.f && off.trauma() == 0.f &&
                  !off.isHitStopped() && off.particleCount() == 0 &&
                  off.shakeOffset() == sf::Vector2f(0.f, 0.f),
              "feel: the master switch turns the whole layer off");

        // Tuning reaches the code it names.
        Game tuned;
        tuned.feel.streakWindow = 5.f;
        tuned.onGoldCollected({400.f, 300.f});
        tuned.update(2.0f); // past the shipped window, inside the tuned one
        const bool windowHeld = tuned.collectStreak() == 1;
        tuned.feel.burstBase = 0;
        tuned.feel.burstPerStreak = 0;
        tuned.onGoldCollected({400.f, 300.f});
        check(windowHeld && tuned.particleCount() == 0 && tuned.ringCount() == 1,
              "feel: panel numbers change behaviour (window, burst size)");
    }

    // --- Platformer physics: gravity & grounded (Lesson 54) -------------
    {
        auto platformerPlayer = [](Game& g, sf::Vector2f pos) {
            for (auto& e : g.entities())
                if (e.playerControlled)
                {
                    e.moveModel = MoveModel::Platformer;
                    e.position = pos;
                    e.velocity = {0.f, 0.f};
                }
        };

        // Find a clear drop column empirically: a column where the
        // player starts embedded in a terrain block resolves on
        // top of it instead of falling (the resolver assumes you
        // arrived from your movement side), so reject any column
        // where the first frames do not move DOWN.
        bool fallOk = false;
        for (float x : {150.f, 400.f, 650.f, 900.f, 1150.f, 1400.f})
        {
            Game fall;
            platformerPlayer(fall, {x, 100.f});
            for (int i = 0; i < 10; ++i)
                fall.update(1.f / 60.f);
            float fellTo = 0.f;
            for (auto& e : fall.entities())
                if (e.playerControlled)
                    fellTo = e.position.y;
            if (fellTo <= 100.f)
                continue; // embedded start: not a fall
            bool landed = false;
            float restY = 0.f;
            for (int i = 0; i < 300 && !landed; ++i)
            {
                fall.update(1.f / 60.f);
                for (auto& e : fall.entities())
                    if (e.playerControlled && e.grounded)
                    {
                        landed = true;
                        restY = e.position.y;
                    }
            }
            for (int i = 0; i < 10; ++i)
                fall.update(1.f / 60.f);
            bool stable = false;
            for (auto& e : fall.entities())
                if (e.playerControlled)
                    stable = e.position.y == restY && e.velocity.y == 0.f &&
                             e.grounded;
            fallOk = landed && stable;
            if (fallOk)
                break;
        }
        check(fallOk,
              "platformer: gravity pulls, the ground answers, rest is rest");

        // Landing on a crate: staged on a verified-clear patch of
        // the tile grid (Lesson 56 made the grid itself collision,
        // so an honest entity landing needs clear geology).
        Game crate;
        float crateRestY = 0.f;
        const bool onCrate = standOnPerch(crate, crateRestY);
        check(onCrate, "platformer: a fall can land on an entity, not just "
                       "the floor");
    }

    // --- Jump feel (Lesson 55), driven through the input seam -----------
    {
        const float dt = 1.f / 60.f;

        // Held jump: launches, rises well clear, returns to the rest.
        Game held;
        float heldRest = 0.f;
        const bool heldStaged = standOnPerch(held, heldRest);
        held.setInput({{0.f, 0.f}, true});
        held.update(dt);
        const bool launched = playerOf(held).velocity.y < -100.f;
        float heldApex = playerOf(held).position.y;
        for (int i = 0; i < 240 && !playerOf(held).grounded; ++i)
        {
            held.update(dt);
            heldApex = std::min(heldApex, playerOf(held).position.y);
        }
        // Tapped jump: release after two frames — the cut bites.
        Game tapped;
        float tappedRest = 0.f;
        const bool tappedStaged = standOnPerch(tapped, tappedRest);
        tapped.setInput({{0.f, 0.f}, true});
        tapped.update(dt);
        tapped.update(dt);
        tapped.setInput({{0.f, 0.f}, false});
        float tappedApex = playerOf(tapped).position.y;
        for (int i = 0; i < 240 && !playerOf(tapped).grounded; ++i)
        {
            tapped.update(dt);
            tappedApex = std::min(tappedApex, playerOf(tapped).position.y);
        }
        check(heldStaged && tappedStaged && launched &&
                  heldApex < heldRest - 60.f &&
                  tappedApex > heldApex + 25.f &&
                  playerOf(held).position.y == heldRest,
              "jump: held flies high, tapped cuts short, both land home");

        // Coyote: walk off the perch, press jump 3 frames later.
        Game coyote;
        float coyoteRest = 0.f;
        standOnPerch(coyote, coyoteRest);
        coyote.setInput({{1.f, 0.f}, false});
        for (int i = 0; i < 90 && playerOf(coyote).grounded; ++i)
            coyote.update(dt);
        for (int i = 0; i < 3; ++i)
            coyote.update(dt);
        coyote.setInput({{1.f, 0.f}, true});
        coyote.update(dt);
        const bool coyoteSaved = playerOf(coyote).velocity.y < 0.f;
        // Same walk-off, but the press comes 12 frames late: no save.
        Game late;
        float lateRest = 0.f;
        standOnPerch(late, lateRest);
        late.setInput({{1.f, 0.f}, false});
        for (int i = 0; i < 90 && playerOf(late).grounded; ++i)
            late.update(dt);
        for (int i = 0; i < 12; ++i)
            late.update(dt);
        late.setInput({{1.f, 0.f}, true});
        late.update(dt);
        check(coyoteSaved && playerOf(late).velocity.y > 0.f,
              "jump: coyote time saves the early press, not the late one");

        // Buffer: stage the perch, lift the player back above it,
        // press 4 frames BEFORE the landing; the jump must fire on
        // the first grounded decision after touchdown.
        Game buffered;
        float bufRest = 0.f;
        sf::Vector2f perch{0.f, 0.f};
        bool bufStaged = false;
        for (int skip = 0; skip < 8 && !bufStaged; ++skip)
            bufStaged = stageCratePerch(buffered, skip, bufRest, &perch);
        Entity& bp = playerOf(buffered);
        bp.position = perch + sf::Vector2f(5.f, -110.f);
        bp.velocity = {0.f, 0.f};
        bp.grounded = false;
        for (int i = 0; i < 11; ++i)
            buffered.update(dt);
        buffered.setInput({{0.f, 0.f}, true});
        buffered.update(dt);
        buffered.setInput({{0.f, 0.f}, false});
        bool fired = false;
        for (int i = 0; i < 30 && !fired; ++i)
        {
            const bool wasGrounded = bp.grounded;
            buffered.update(dt);
            if (wasGrounded && bp.velocity.y < -100.f)
                fired = true;
        }
        check(bufStaged && fired,
              "jump: a press just before landing is remembered");
    }


    // --- Tile collision (Lesson 56) --------------------------------------
    {
        auto playerOf = [](Game& g) -> Entity& {
            for (auto& e : g.entities())
                if (e.playerControlled)
                    return e;
            return g.entities().front();
        };
        const float dt = 1.f / 60.f;

        // A drop onto rock: find a rock tile with clear air above.
        Game drop;
        int rockTx = -1, rockTy = -1;
        for (int ty = 2; ty < drop.tilesY() && rockTx < 0; ++ty)
            for (int tx = 0; tx < drop.tilesX(); ++tx)
                if (drop.tileAt(tx, ty) == tiles::Rock &&
                    !drop.tileSolid(drop.tileAt(tx, ty - 1)) &&
                    !drop.tileSolid(drop.tileAt(tx, ty - 2)))
                {
                    rockTx = tx;
                    rockTy = ty;
                    break;
                }
        bool rockLanded = false;
        if (rockTx >= 0)
        {
            Entity& p = playerOf(drop);
            p.moveModel = MoveModel::Platformer;
            p.position = {rockTx * 20.f - 4.f, rockTy * 20.f - 110.f};
            p.velocity = {0.f, 0.f};
            for (int i = 0; i < 240 && !p.grounded; ++i)
                drop.update(dt);
            rockLanded = p.grounded &&
                         std::abs(p.position.y - (rockTy * 20.f - 50.f)) < 0.01f;
        }
        check(rockLanded, "tiles: a platformer lands on rock, exactly on its top");

        // A step up is a wall: plateau of rock with a rock tile
        // rising at its right end. Walk right; stop at the wall.
        // The default world's geology has no such step, so scan a
        // handful of seeds — the physics claim is world-independent.
        Game wall;
        int stepTx = -1, stepTy = -1;
        for (int seed : {12345, 777, 42, 2024, 555, 9001, 13, 2718, 31415})
        {
            wall.setSeed(seed);
            for (int ty = 2; ty < wall.tilesY() && stepTx < 0; ++ty)
                for (int tx = 1; tx + 1 < wall.tilesX(); ++tx)
                    if (wall.tileAt(tx, ty) == tiles::Rock &&
                        wall.tileAt(tx + 1, ty) == tiles::Rock &&
                        wall.tileAt(tx + 1, ty - 1) == tiles::Rock &&
                        !wall.tileSolid(wall.tileAt(tx, ty - 1)) &&
                        !wall.tileSolid(wall.tileAt(tx - 1, ty - 1)))
                    {
                        stepTx = tx;
                        stepTy = ty;
                        break;
                    }
            if (stepTx >= 0)
                break;
        }
        bool wallHeld = false;
        if (stepTx >= 0)
        {
            Entity& p = playerOf(wall);
            p.moveModel = MoveModel::Platformer;
            // Drop fully left of the wall column: the 28-wide feet
            // box spans two tile columns, and straddling the wall's
            // column would land him on top of it instead.
            p.position = {stepTx * 20.f - 24.f, stepTy * 20.f - 110.f};
            p.velocity = {0.f, 0.f};
            for (int i = 0; i < 240 && !p.grounded; ++i)
                wall.update(dt);
            wall.setInput({{1.f, 0.f}, false});
            for (int i = 0; i < 90; ++i)
                wall.update(dt);
            const float stoppedX = p.position.x;
            for (int i = 0; i < 10; ++i)
                wall.update(dt);
            wallHeld = p.grounded && p.position.x == stoppedX &&
                       std::abs(p.position.x -
                                ((stepTx + 1) * 20.f - 39.f)) < 0.01f;
        }
        check(wallHeld, "tiles: a rock step is a wall, met exactly at its face");

        // The top-down model walks OVER the same rock (Gold Rush's
        // world is a ground plane). Try candidate tiles until one
        // gives a clear run — blocks are invisible to this scan.
        bool crossed = false;
        for (int ty = 0; ty < 60 && !crossed; ++ty)
        {
            for (int tx = 2; tx < 78 && !crossed; ++tx)
            {
                Game over;
                if (over.tileAt(tx, ty) != tiles::Rock)
                    continue;
                Entity& p = playerOf(over);
                p.position = {tx * 20.f - 34.f, ty * 20.f - 15.f};
                const float startX = p.position.x;
                over.setInput({{1.f, 0.f}, false});
                for (int i = 0; i < 60; ++i)
                    over.update(dt);
                crossed = p.position.x > startX + 60.f;
            }
        }
        check(crossed, "tiles: the top-down player crosses rock unimpeded");
    }

    // --- Platforms (Lesson 57) ---------------------------------------------
    {
        const float dt = 1.f / 60.f;
        auto makePlatform = [](sf::Vector2f pos, sf::Vector2f size) {
            Entity plat;
            plat.name = "TestPlatform";
            plat.position = pos;
            plat.size = size;
            plat.platform = true;
            plat.solid = true;
            return plat;
        };

        // One-way: from the staged perch, jump up THROUGH the
        // platform (feet cross above its top), then land ON it.
        Game ow;
        float perchRest = 0.f;
        sf::Vector2f perch{0.f, 0.f};
        bool staged = false;
        for (int skip = 0; skip < 8 && !staged; ++skip)
            staged = stageCratePerch(ow, skip, perchRest, &perch);
        const float platTop = perchRest + 50.f - 70.f;
        Entity oneWay = makePlatform({perch.x - 10.f, platTop}, {80.f, 12.f});
        oneWay.oneWay = true;
        ow.entities().push_back(oneWay);
        Entity& op = playerOf(ow);
        ow.setInput({{0.f, 0.f}, true}); // a held jump, ~102 of rise
        float minFeet = 1e9f;
        bool leftGround = false, landedBack = false;
        for (int i = 0; i < 300 && !landedBack; ++i)
        {
            ow.update(dt);
            minFeet = std::min(minFeet, op.position.y + 50.f);
            if (!op.grounded)
                leftGround = true;
            else if (leftGround)
                landedBack = true;
        }
        check(staged && landedBack && minFeet < platTop - 15.f &&
                  std::abs(op.position.y + 50.f - platTop) < 0.01f,
              "platforms: jump up through a one-way, land on top of it");

        // A moving platform carries its rider: 60 frames at
        // 60 u/s of patrol = 60 units of travel, feet glued.
        bool carried = false;
        for (int attempt = 0; attempt < 6 && !carried; ++attempt)
        {
            Game mv;
            int bandTx = -1, bandTy = -1, seenBand = -1;
            for (int ty = 8; ty + 1 < mv.tilesY() && bandTx < 0; ++ty)
                for (int tx = 1; tx + 10 < mv.tilesX(); ++tx)
                {
                    bool clear = true;
                    for (int dy = -3; dy <= 1 && clear; ++dy)
                        for (int dx = 0; dx < 10 && clear; ++dx)
                            if (mv.tileSolid(mv.tileAt(tx + dx, ty + dy)))
                                clear = false;
                    if (clear && ++seenBand == attempt)
                    {
                        bandTx = tx;
                        bandTy = ty;
                        break;
                    }
                }
            if (bandTx < 0)
                break;
            Entity mover =
                makePlatform({bandTx * 20.f, bandTy * 20.f}, {60.f, 12.f});
            mover.platformTo = mover.position + sf::Vector2f(120.f, 0.f);
            mover.platformSpeed = 60.f;
            mv.entities().push_back(mover);
            Entity& p = playerOf(mv);
            p.moveModel = MoveModel::Platformer;
            p.position = mover.position + sf::Vector2f(16.f, -70.f);
            p.velocity = {0.f, 0.f};
            for (int i = 0; i < 240 && !p.grounded; ++i)
                mv.update(dt);
            if (!p.grounded)
                continue;
            const float startX = p.position.x;
            const float feetY = p.position.y + 50.f;
            for (int i = 0; i < 60; ++i)
                mv.update(dt);
            carried = p.grounded &&
                      std::abs((p.position.x - startX) - 60.f) < 2.f &&
                      std::abs((p.position.y + 50.f) - feetY) < 0.01f;
        }
        check(carried, "platforms: a moving platform carries its rider");
    }

    // --- The Physics panel's levers (Lesson 58) ----------------------------
    {
        const float dt = 1.f / 60.f;
        // Doubled gravity halves the jump's rise (v^2 / 2g): the
        // panel's sliders write these same fields.
        Game heavy;
        float rest = 0.f;
        const bool staged = standOnPerch(heavy, rest);
        heavy.gravity = 3600.f;
        heavy.setInput({{0.f, 0.f}, true});
        heavy.update(dt);
        float apex = playerOf(heavy).position.y;
        for (int i = 0; i < 240 && !playerOf(heavy).grounded; ++i)
        {
            heavy.update(dt);
            apex = std::min(apex, playerOf(heavy).position.y);
        }
        // Solidity is a switch: with rock switched off, the
        // Lesson 56 drop falls straight through the tile's band.
        Game ghost;
        int rockTx = -1, rockTy = -1;
        for (int ty = 8; ty < ghost.tilesY() && rockTx < 0; ++ty)
            for (int tx = 0; tx < ghost.tilesX(); ++tx)
                if (ghost.tileAt(tx, ty) == tiles::Rock &&
                    !ghost.tileSolid(ghost.tileAt(tx, ty - 1)) &&
                    !ghost.tileSolid(ghost.tileAt(tx, ty - 2)))
                {
                    rockTx = tx;
                    rockTy = ty;
                    break;
                }
        bool fellThrough = false;
        if (rockTx >= 0)
        {
            ghost.setTileSolid(tiles::Rock, false);
            Entity& p = playerOf(ghost);
            p.moveModel = MoveModel::Platformer;
            p.position = {rockTx * 20.f - 4.f, rockTy * 20.f - 110.f};
            p.velocity = {0.f, 0.f};
            for (int i = 0; i < 90; ++i)
                ghost.update(dt);
            fellThrough = p.position.y + 50.f > rockTy * 20.f + 20.f;
        }
        check(staged && apex > rest - 75.f && fellThrough,
              "physics levers: gravity retunes the jump, solidity switches");
    }

    // --- The climber shaft, climbed end to end (Lesson 59) -----------------
    {
        const float dt = 1.f / 60.f;
        Game g;
        g.buildClimberTest();
        g.buildClimberTest(); // staging twice must not duplicate
        int climberCount = 0;
        for (auto& e : g.entities())
            if (e.name.rfind("Climber", 0) == 0)
                ++climberCount;
        Entity& p = playerOf(g);
        Entity* ride = nullptr;
        Entity* goal = nullptr;
        for (auto& e : g.entities())
        {
            if (e.name == "ClimberRide")
                ride = &e;
            if (e.name == "ClimberGoal")
                goal = &e;
        }
        auto feet = [&p] { return p.position.y + 50.f; };

        for (int i = 0; i < 120 && !p.grounded; ++i)
            g.update(dt);
        const bool onFloor = p.grounded && std::abs(feet() - 1100.f) < 0.01f;

        // Hop 1: straight up through the one-way Ledge1, land on it.
        g.setInput({{0.f, 0.f}, true}); // held: no jump cut
        bool onLedge1 = false;
        for (int i = 0; i < 300 && !onLedge1; ++i)
        {
            g.update(dt);
            onLedge1 = p.grounded && std::abs(feet() - 1010.f) < 0.01f;
        }
        // Hop 2: run right, jump to Ledge2. Release first so the
        // press registers as a fresh edge.
        g.setInput({{1.f, 0.f}, false});
        for (int i = 0; i < 3; ++i)
            g.update(dt);
        g.setInput({{1.f, 0.f}, true});
        bool onLedge2 = false;
        for (int i = 0; i < 300 && !onLedge2; ++i)
        {
            g.update(dt);
            onLedge2 = p.grounded && std::abs(feet() - 920.f) < 0.01f;
        }
        // Board the ride: wait on Ledge2 for a pass that is moving
        // TOWARD his column and close — the flight then closes the
        // distance instead of chasing it. Retry on later passes.
        // The press is a state machine, not a level: hold the
        // button until the rise ends (a release mid-rise would cut
        // the jump), then release so the next press is a fresh
        // edge. (A level-held press never re-edges — the first
        // driver learned this by jumping zero times in 2400 frames.)
        g.setInput({{0.f, 0.f}, false});
        bool onRide = false, holding = false;
        float lastRideX = ride->position.x;
        for (int i = 0; i < 2400 && !onRide; ++i)
        {
            const bool standing = p.grounded && std::abs(feet() - 920.f) < 0.01f;
            const float deckCentre = ride->position.x + 35.f;
            const float myCentre = p.position.x + 25.f;
            const float rideVel = ride->position.x - lastRideX;
            lastRideX = ride->position.x;
            const bool approaching =
                (deckCentre < myCentre && rideVel > 0.f) ||
                (deckCentre > myCentre && rideVel < 0.f);
            if (!holding && standing && approaching &&
                std::abs(deckCentre - myCentre) > 55.f && std::abs(deckCentre - myCentre) <= 80.f)
            {
                g.setInput({{0.f, 0.f}, true});
                holding = true;
            }
            g.update(dt);
            if (holding && (p.velocity.y >= 0.f || p.grounded))
            {
                g.setInput({{0.f, 0.f}, false});
                holding = false;
            }
            if (p.grounded && std::abs(feet() - 830.f) < 0.01f)
                onRide = true;
        }
        // Ride to the goal: it hangs above the deck's path, so a
        // straight jump from under it bonks its underside. The
        // play is to jump from BESIDE its span and drift over it —
        // trigger 35-65 units from the near edge, drift held
        // through the flight.
        bool onGoal = false;
        if (onRide)
        {
            g.setInput({{0.f, 0.f}, false});
            holding = false;
            float drift = 0.f;
            for (int i = 0; i < 1200 && !onGoal; ++i)
            {
                const bool riding =
                    p.grounded && std::abs(feet() - 830.f) < 0.01f;
                const float myCentre = p.position.x + 25.f;
                const float goalCentre = goal->position.x + 60.f;
                const bool leftSide = myCentre < goalCentre;
                // Distance from the LEADING hitbox edge to the
                // span's edge: the rise needs ~15 units of drift
                // between clearing the underside and crossing the
                // top, so the window is tight and measured at the
                // edge that arrives first.
                const float leadDist =
                    leftSide ? goal->position.x - (p.position.x + 39.f)
                             : (p.position.x + 11.f) -
                                   (goal->position.x + 120.f);
                if (!holding && riding && leadDist > 26.f && leadDist < 60.f)
                {
                    drift = leftSide ? 1.f : -1.f;
                    g.setInput({{drift, 0.f}, true});
                    holding = true;
                }
                g.update(dt);
                if (holding && p.velocity.y >= 0.f)
                    g.setInput({{drift, 0.f}, false}); // keep drifting
                if (holding && p.grounded)
                {
                    g.setInput({{0.f, 0.f}, false});
                    holding = false;
                }
                if (p.grounded && std::abs(feet() - 745.f) < 0.01f)
                    onGoal = true;
            }
        }
        check(climberCount == 5 && onFloor && onLedge1 && onLedge2 && onRide &&
                  onGoal,
              "climber: the shaft climbs end to end — floor, ledges, ride, "
              "goal");
    }

    // --- Lua lives in the engine (Lesson 66) ------------------------------
    {
        namespace fs = std::filesystem;
        ScriptHost host;
        double value = 0;
        const bool math = host.valid() &&
                          host.evalNumber("return 6 * 7", value) &&
                          value == 42.0;
        const bool state = host.runString("answer = 40 + 2") &&
                           host.evalNumber("return answer", value) &&
                           value == 42.0;
        // A broken script is a false with a log line, never a crash.
        const bool survives = !host.runString("this is not lua ((") &&
                              !host.runString("error('boom')") &&
                              host.evalNumber("return 1", value);
        const fs::path scriptFile = tempPath("host-test.lua");
        writeFile(scriptFile, "fromFile = 7 * 6");
        const bool file = host.runFile(scriptFile.string()) &&
                          host.evalNumber("return fromFile", value) &&
                          value == 42.0 &&
                          !host.runFile("/nonexistent-66.lua");
        check(math && state && survives && file,
              "lua: the host runs, remembers, and survives bad scripts");
        fs::remove(scriptFile);
    }

    // --- The bridge: Lua drives an entity (Lesson 67) ----------------------
    {
        Game bg;
        Entity* target = nullptr;
        for (auto& e : bg.entities())
            if (e.name == "Crate")
            {
                e.name = "ScriptCrate"; // unique for the binding test
                target = &e;
                break;
            }
        ScriptHost host;
        host.installEntityApi(bg);
        int moduleId = -1, instanceId = -1;
        const bool loaded = host.loadModuleFromString(
            "local M = {}\n"
            "function M.onUpdate(self, dt)\n"
            "  local x, y = entity.getPosition(self)\n"
            "  entity.setPosition(self, x + 30 * dt, y)\n"
            "  self.t = (self.t or 0) + dt\n"
            "  _G.lastT = self.t\n"
            "end\n"
            "return M\n",
            "mover", moduleId);
        const bool instanced =
            loaded && host.createInstance(moduleId, "ScriptCrate",
                                          instanceId);
        const float startX = target ? target->position.x : 0.f;
        bool updatesOk = instanced;
        for (int i = 0; i < 60 && updatesOk; ++i)
            updatesOk = host.callUpdate(instanceId, 1.f / 60.f);
        double lastT = 0;
        const bool stateKept = host.evalNumber("return lastT", lastT) &&
                               std::abs(lastT - 1.0) < 0.001;
        // A module whose onUpdate errors: reported, survived.
        int badModule = -1, badInstance = -1;
        const bool badLoaded = host.loadModuleFromString(
            "local M = {}\n"
            "function M.onUpdate(self, dt) error('nope') end\n"
            "return M\n",
            "bad", badModule);
        const bool badSurvived =
            badLoaded &&
            host.createInstance(badModule, "ScriptCrate", badInstance) &&
            !host.callUpdate(badInstance, 1.f / 60.f) &&
            host.evalNumber("return 1", lastT);
        check(target && updatesOk && stateKept && badSurvived &&
                  std::abs(target->position.x - (startX + 30.f)) < 0.01f,
              "bridge: a Lua module moves an entity, keeps state, and "
              "its errors stay its own");
    }

    // --- Entity types: the data half (Lesson 68) ---------------------------
    {
        namespace fs = std::filesystem;
        Game tg;
        EntityType guard;
        guard.name = "Guard";
        guard.size = {44.f, 44.f};
        guard.color = sf::Color(80, 120, 220);
        guard.moveModel = MoveModel::Platformer;
        guard.script = "scripts/guard.lua";
        tg.defineType(guard);
        const bool spawned = tg.spawnTyped("Guard", "Guard 1", {500, 500});
        const Entity* g1 = nullptr;
        for (const auto& e : tg.entities())
            if (e.name == "Guard 1")
                g1 = &e;
        const bool stamped = g1 && g1->type == "Guard" &&
                             g1->size == sf::Vector2f(44.f, 44.f) &&
                             g1->moveModel == MoveModel::Platformer &&
                             g1->script == "scripts/guard.lua" &&
                             g1->color == sf::Color(80, 120, 220);
        // Types load from a file in the engine's key=value family.
        const fs::path typesFile = tempPath("types.txt");
        writeFile(typesFile,
                  "# cast\n"
                  "type=Sentry\n"
                  "size=30,60\n"
                  "model=platformer\n"
                  "script=scripts/sentry.lua\n"
                  "\n"
                  "type=Patroller\n"
                  "size=36,36\n"
                  "solid=0\n");
        const bool loadedFile = tg.loadTypes(typesFile.string());
        const EntityType* sentry = tg.findType("Sentry");
        const EntityType* patroller = tg.findType("Patroller");
        const bool fromFile =
            loadedFile && sentry && sentry->size == sf::Vector2f(30.f, 60.f) &&
            sentry->script == "scripts/sentry.lua" && patroller &&
            !patroller->solid &&
            patroller->moveModel == MoveModel::TopDown;
        // Unknown types fail cleanly; redefining replaces.
        const bool unknownFails = !tg.spawnTyped("Nope", "Ghost", {0, 0});
        guard.size = {48.f, 48.f};
        tg.defineType(guard);
        const bool replaced = tg.findType("Guard") &&
                              tg.findType("Guard")->size ==
                                  sf::Vector2f(48.f, 48.f);
        check(spawned && stamped && fromFile && unknownFails && replaced,
              "types: definitions stamp entities, load from files, and "
              "replace cleanly");
        fs::remove(typesFile);
    }

    // --- The project is a folder (Lesson 60) -------------------------------
    {
        namespace fs = std::filesystem;
        const fs::path dir = tempPath("project-place");
        fs::remove_all(dir);
        Project created;
        const bool made = createProjectFolder(dir.string(), "Test Game",
                                              created);
        const bool layout =
            fs::exists(dir / "project.myengine") &&
            fs::is_directory(dir / "assets") &&
            fs::is_directory(dir / "scenes") &&
            fs::is_directory(dir / "scripts");
        Project loaded;
        const bool readBack = loadProjectFolder(dir.string(), loaded) &&
                              loaded.name == "Test Game" &&
                              loaded.root == dir.string() &&
                              loaded.seed == created.seed;
        const bool noClobber =
            !createProjectFolder(dir.string(), "Other", created);
        loaded.name = "Renamed Game";
        const bool saved = saveProject(loaded);
        Project again;
        const bool renamed = saved && loadProjectFolder(dir.string(), again) &&
                             again.name == "Renamed Game";
        const bool paths =
            projectPath(again, "assets/a.png") ==
                dir.string() + "/assets/a.png" &&
            projectPath(Project{}, "assets/a.png") == "assets/a.png";
        check(made && layout && readBack && noClobber && renamed && paths,
              "project: a folder is created, read back, saved, resolved");

        // The tree listing (Lesson 61): files appear with their
        // project-relative paths and sizes; hidden plumbing does
        // not; a rootless project lists nothing.
        writeFile(dir / "assets" / "hero.png", "PNGDATA");
        writeFile(dir / "scenes" / "level1.scene", "scene");
        writeFile(dir / ".hidden", "shh");
        const auto listing = listProjectFiles(again);
        auto has = [&listing](const std::string& rel, bool dirWanted) {
            for (const auto& e : listing)
                if (e.relativePath == rel && e.isDirectory == dirWanted)
                    return true;
            return false;
        };
        std::uintmax_t heroSize = 0;
        for (const auto& e : listing)
            if (e.relativePath == "assets/hero.png")
                heroSize = e.size;
        check(has("assets", true) && has("assets/hero.png", false) &&
                  has("scenes/level1.scene", false) &&
                  has("project.myengine", false) && !has(".hidden", false) &&
                  heroSize == 7 && listProjectFiles(Project{}).empty(),
              "project: the tree lists files, skips hidden, sizes honestly");

        // One directory, the file browser's view (Lesson 62):
        // directories first, then files, each by name; hidden
        // entries skipped; a missing directory lists nothing.
        const auto entries = listDirectory(dir.string());
        std::vector<std::string> names;
        for (const auto& e : entries)
            names.push_back(std::string(e.isDirectory ? "d:" : "f:") + e.name);
        check(names == std::vector<std::string>{"d:assets", "d:scenes",
                                                "d:scripts", "f:project.myengine"} &&
                  listDirectory("/nonexistent-place-62").empty(),
              "dialog: one directory lists dirs first, then files, by name");

        // Import (Lesson 63): a file lands in assets/ under its
        // own name — or, when that name is taken (the planted
        // hero.png), under the next suffix, contents intact,
        // source untouched; a missing source imports nothing.
        const fs::path outside = tempPath("hero.png");
        writeFile(outside, "OUTSIDE");
        const std::string landed1 = importAsset(again, outside.string());
        const std::string landed2 = importAsset(again, outside.string());
        std::string landedContent;
        {
            std::ifstream in(dir / "assets" / "hero-2.png");
            landedContent.assign(std::istreambuf_iterator<char>(in),
                                 std::istreambuf_iterator<char>());
        }
        check(landed1 == "assets/hero-2.png" &&
                  landed2 == "assets/hero-3.png" &&
                  landedContent == "OUTSIDE" &&
                  fs::exists(outside) &&
                  importAsset(again, "/nonexistent-63.png").empty(),
              "import: copies into assets, renames on collision, never "
              "overwrites");

        // Project-relative resources (Lesson 65): a real image,
        // imported, then loaded as a texture purely by resolving
        // its project-relative path against the root — from this
        // process's unrelated working directory. This is the
        // resolution the Project panel's preview uses.
        const fs::path artSource = tempPath("art.png");
        sf::Image art({6, 4}, sf::Color::Magenta);
        const bool artSaved = art.saveToFile(artSource.string());
        const std::string artLanded = importAsset(again, artSource.string());
        sf::Texture artTexture;
        const bool artLoaded = !artLanded.empty() &&
                               artTexture.loadFromFile(
                                   projectPath(again, artLanded));
        check(artSaved && artLoaded &&
                  artTexture.getSize() == sf::Vector2u(6, 4),
              "resources: an imported image loads through its "
              "project-relative path");
        fs::remove_all(dir);
        fs::remove(outside);
        fs::remove(artSource);
    }

    // --- Scene format v2 (Lesson 69) ---------------------------------------
    {
        namespace fs = std::filesystem;
        Game sg;
        for (auto& e : sg.entities())
            if (e.name == "Crate")
            {
                e.type = "Guard";
                e.script = "scripts/guard.lua";
                e.moveModel = MoveModel::Platformer;
                e.platform = true;
                e.oneWay = true;
                e.platformTo = {900.f, 830.f};
                e.platformSpeed = 70.f;
                break;
            }
        const fs::path v2File = tempPath("scene-v2.txt");
        const bool saved = sg.saveScene(v2File.string());
        std::string text;
        {
            std::ifstream in(v2File);
            text.assign(std::istreambuf_iterator<char>(in),
                        std::istreambuf_iterator<char>());
        }
        Game lg;
        const bool loaded = lg.loadScene(v2File.string());
        const Entity* back = nullptr;
        for (const auto& e : lg.entities())
            if (e.name == "Crate")
            {
                back = &e;
                break;
            }
        const bool roundTrip =
            saved && text.find("version=2") != std::string::npos && loaded &&
            back && back->type == "Guard" &&
            back->script == "scripts/guard.lua" &&
            back->moveModel == MoveModel::Platformer && back->platform &&
            back->oneWay && back->platformTo == sf::Vector2f(900.f, 830.f) &&
            back->platformSpeed == 70.f;
        // A v1 file (11 fields) still loads, new fields defaulted.
        const fs::path v1File = tempPath("scene-v1.txt");
        writeFile(v1File,
                  "# MyEngine scene\n"
                  "version=1\n"
                  "seed=12345\n"
                  "noiseScale=0.01\n"
                  "entities=1\n"
                  "entity=Old|100|100|40|40|200|100|50|0|1|-1\n");
        Game og;
        const bool v1Loaded = og.loadScene(v1File.string());
        const bool v1Defaults =
            v1Loaded && og.entities().size() == 1 &&
            og.entities()[0].type.empty() && og.entities()[0].script.empty() &&
            og.entities()[0].moveModel == MoveModel::TopDown &&
            !og.entities()[0].platform;
        // A future version is refused whole, never half-loaded.
        const fs::path v3File = tempPath("scene-v3.txt");
        writeFile(v3File, "version=3\nseed=1\nentities=0\n");
        Game fg;
        const bool v3Refused = !fg.loadScene(v3File.string());
        check(roundTrip && v1Defaults && v3Refused,
              "scene v2: new fields round-trip, v1 loads defaulted, v3 "
              "refused");
        fs::remove(v2File);
        fs::remove(v1File);
        fs::remove(v3File);
    }

    // --- Scripts in the loop (Lesson 70) -------------------------------
    {
        namespace fs = std::filesystem;
        const fs::path dir = tempPath("script-loop");
        fs::remove_all(dir);
        fs::create_directories(dir / "scripts");
        writeFile(dir / "scripts" / "mover.lua",
                  "local M = {}\n"
                  "function M.onUpdate(self, dt)\n"
                  "  local x, y = entity.getPosition(self)\n"
                  "  entity.setPosition(self, x + 30 * dt, y)\n"
                  "end\n"
                  "return M\n");
        writeFile(dir / "scripts" / "boom.lua",
                  "local M = {}\n"
                  "function M.onUpdate(self, dt)\n"
                  "  _G.boomCount = (_G.boomCount or 0) + 1\n"
                  "  error('boom')\n"
                  "end\n"
                  "return M\n");
        // Two scripted crates: one mover, one that errors every call.
        Game g;
        g.setScriptRoot(dir.string());
        bool tagged = false;
        for (auto& e : g.entities())
        {
            if (e.name != "Crate")
                continue;
            if (!tagged)
            {
                e.name = "ScriptRunner";
                e.script = "scripts/mover.lua";
                tagged = true;
            }
            else
            {
                e.name = "ScriptBoomer";
                e.script = "scripts/boom.lua";
            }
        }
        g.bindScripts();
        const bool bound = g.scriptBindingCount() == 2;
        float startX = 0.f;
        for (const auto& e : g.entities())
            if (e.name == "ScriptRunner")
                startX = e.position.x;
        for (int i = 0; i < 60; ++i)
            g.update(1.f / 60.f);
        float endX = 0.f;
        for (const auto& e : g.entities())
            if (e.name == "ScriptRunner")
                endX = e.position.x;
        const bool moved = std::abs(endX - (startX + 30.f)) < 0.5f;
        // The error latch: sixty frames of updates, exactly ONE call
        // into the broken script — it failed once and was latched off.
        double booms = 0.0;
        const bool counted =
            g.scriptHost().evalNumber("return _G.boomCount or -1", booms);
        const bool latched = counted && booms == 1.0;
        // And the promise of the lesson: a scene FILE goes live on
        // load — no explicit bind in between.
        const fs::path sceneFile = tempPath("script-loop.scene");
        const bool saved = g.saveScene(sceneFile.string());
        Game lg;
        lg.setScriptRoot(dir.string());
        const bool loadedLive =
            saved && lg.loadScene(sceneFile.string()) &&
            lg.scriptBindingCount() == 2;
        check(bound && moved && latched && loadedLive,
              "scripts in the loop: update() drives onUpdate, a broken "
              "script latches after one failure, scene loads go live");
        fs::remove(sceneFile);
        fs::remove_all(dir);
    }

    // --- Hot reload (Lesson 71) ----------------------------------------
    {
        namespace fs = std::filesystem;
        const fs::path dir = tempPath("script-reload");
        fs::remove_all(dir);
        fs::create_directories(dir / "scripts");
        const fs::path moverFile = dir / "scripts" / "mover.lua";
        const auto moverText = [](int speed) {
            return std::string(
                       "local M = {}\n"
                       "function M.onUpdate(self, dt)\n"
                       "  self.t = (self.t or 0) + dt\n"
                       "  _G.lastT = self.t\n"
                       "  local x, y = entity.getPosition(self)\n"
                       "  entity.setPosition(self, x + ") +
                   std::to_string(speed) +
                   " * dt, y)\n"
                   "end\n"
                   "return M\n";
        };
        writeFile(moverFile, moverText(30));
        Game g;
        g.setScriptRoot(dir.string());
        for (auto& e : g.entities())
            if (e.name == "Crate")
            {
                e.name = "ReloadRunner";
                e.script = "scripts/mover.lua";
                break;
            }
        g.bindScripts();
        const auto runnerX = [&]() {
            for (const auto& e : g.entities())
                if (e.name == "ReloadRunner")
                    return e.position.x;
            return -1.f;
        };
        const float x0 = runnerX();
        for (int i = 0; i < 60; ++i)
            g.update(1.f / 60.f);
        const bool firstLeg = std::abs(runnerX() - (x0 + 30.f)) < 0.5f;
        const std::size_t noneChanged = g.reloadScriptsIfChanged();
        // Edit the file — triple the speed — and stamp its mtime
        // forward so the watch cannot miss it on any filesystem.
        const auto t0 = fs::last_write_time(moverFile);
        writeFile(moverFile, moverText(90));
        fs::last_write_time(moverFile, t0 + std::chrono::seconds(2));
        const std::size_t oneChanged = g.reloadScriptsIfChanged();
        const float x1 = runnerX();
        for (int i = 0; i < 60; ++i)
            g.update(1.f / 60.f);
        const bool secondLeg = std::abs(runnerX() - (x1 + 90.f)) < 0.5f;
        // State reset: after the reload and sixty more frames the
        // script's own clock reads one second — not two.
        double lastT = 0.0;
        const bool stateRead =
            g.scriptHost().evalNumber("return _G.lastT or -1", lastT);
        const bool stateReset = stateRead && std::abs(lastT - 1.0) < 0.01;
        check(firstLeg && noneChanged == 0 && oneChanged == 1 &&
                  secondLeg && stateReset,
              "hot reload: an edited script takes over mid-run, its "
              "state resets, untouched files do not reload");
        fs::remove_all(dir);
    }

    // --- The Lua-only NPC (Lesson 72) ----------------------------------
    {
        namespace fs = std::filesystem;
        const fs::path dir = tempPath("npc-patrol");
        fs::remove_all(dir);
        fs::create_directories(dir / "scripts");
        // The shipped scripts/patrol.lua, with RANGE parameterised
        // so the second half of the check can change the constant.
        const auto patrolText = [](int range) {
            return std::string(
                       "local SPEED = 60\n"
                       "local RANGE = ") +
                   std::to_string(range) +
                   "\n"
                   "local M = {}\n"
                   "function M.onUpdate(self, dt)\n"
                   "  if self.dir == nil then\n"
                   "    local x, _ = entity.getPosition(self)\n"
                   "    self.homeX = x\n"
                   "    self.dir = 1\n"
                   "  end\n"
                   "  local x, y = entity.getPosition(self)\n"
                   "  local nextX = x + self.dir * SPEED * dt\n"
                   "  if nextX > self.homeX + RANGE then\n"
                   "    nextX = self.homeX + RANGE\n"
                   "    self.dir = -1\n"
                   "  elseif nextX < self.homeX - RANGE then\n"
                   "    nextX = self.homeX - RANGE\n"
                   "    self.dir = 1\n"
                   "  end\n"
                   "  entity.setPosition(self, nextX, y)\n"
                   "end\n"
                   "return M\n";
        };
        const auto runPatrol = [&](int range, float& maxX, float& minX,
                                   float& endX) {
            writeFile(dir / "scripts" / "patrol.lua", patrolText(range));
            Game g;
            g.setScriptRoot(dir.string());
            EntityType guard;
            guard.name = "Guard";
            guard.script = "scripts/patrol.lua";
            g.defineType(guard);
            const bool spawned =
                g.spawnTyped("Guard", "NpcPatrol", {400.f, 300.f});
            g.bindScripts();
            maxX = -1e9f;
            minX = 1e9f;
            endX = 0.f;
            for (int i = 0; i < 240; ++i)
            {
                g.update(1.f / 60.f);
                for (const auto& e : g.entities())
                    if (e.name == "NpcPatrol")
                    {
                        maxX = std::max(maxX, e.position.x);
                        minX = std::min(minX, e.position.x);
                        endX = e.position.x;
                    }
            }
            return spawned;
        };
        // RANGE 120: two seconds out to the bound at 520, two back
        // home to 400 — the full beat, from Lua alone.
        float maxX, minX, endX;
        const bool spawned = runPatrol(120, maxX, minX, endX);
        const bool walkedBeat = spawned && std::abs(maxX - 520.f) < 1.f &&
                                std::abs(endX - 400.f) < 1.f;
        // RANGE 60 in the file: a different beat — never past 460,
        // never below 340, and it really walks (reaches its bound).
        float maxX2, minX2, endX2;
        runPatrol(60, maxX2, minX2, endX2);
        const bool constantRules = maxX2 < 460.5f && maxX2 > 450.f &&
                                   minX2 > 339.5f;
        check(walkedBeat && constantRules,
              "the Lua-only NPC: a patrol written entirely in a "
              "project script walks its beat, and editing the "
              "script's constant changes the beat");
        fs::remove_all(dir);
    }

    // --- The chunk model (Lesson 73) -----------------------------------
    {
        namespace fs = std::filesystem;
        const fs::path dir = tempPath("chunk-model");
        fs::remove_all(dir);
        fs::create_directories(dir);
        const fs::path chunksFile = dir / "chunks.txt";
        writeFile(chunksFile,
                  "chunk=Alpha\n"
                  "height=200\n"
                  "platform=340|165|120|1\n"
                  "platform=100|100|120|1|300|40|70\n"
                  "chunk=Beta\n"
                  "height=200\n"
                  "platform=340|165|120|1\n"
                  "platform=500|90|110|1\n"
                  "chunk=Gamma\n"
                  "height=260\n"
                  "platform=340|200|120|1\n"
                  "platform=200|120|120|1\n"
                  "platform=420|40|120|1\n");
        Game g;
        const bool loaded = g.loadChunks(chunksFile.string());
        const bool parsed =
            loaded && g.chunkDefs().size() == 3 &&
            g.chunkDefs()[0].platforms.size() == 2 &&
            g.chunkDefs()[0].platforms[1].speed == 70.f &&
            g.chunkDefs()[0].platforms[1].patrolTo ==
                sf::Vector2f(300.f, 40.f) &&
            g.chunkDefs()[2].height == 260.f;
        g.buildAscent(42, 4);
        const auto& deal = g.ascentDeal();
        bool noAdjacentRepeat = deal.size() == 4;
        for (std::size_t i = 1; i < deal.size(); ++i)
            if (deal[i] == deal[i - 1])
                noAdjacentRepeat = false;
        // Expected layout, computed from the defs themselves.
        const auto defOf = [&](const std::string& name) -> const ChunkDef& {
            for (const auto& d : g.chunkDefs())
                if (d.name == name)
                    return d;
            return g.chunkDefs()[0];
        };
        std::size_t expectedEntities = 2; // player + floor
        float top = 1150.f;
        for (const auto& name : deal)
        {
            expectedEntities += defOf(name).platforms.size();
            top -= defOf(name).height;
        }
        const Entity* c0p0 = nullptr;
        const Entity* c1p0 = nullptr;
        for (const auto& e : g.entities())
        {
            if (e.name == "Chunk0P0")
                c0p0 = &e;
            if (e.name == "Chunk1P0")
                c1p0 = &e;
        }
        const float h0 = defOf(deal[0]).height;
        const float h1 = defOf(deal[1]).height;
        const sf::Vector2f want0(400.f + defOf(deal[0]).platforms[0].at.x,
                                 1150.f - h0 +
                                     defOf(deal[0]).platforms[0].at.y);
        const sf::Vector2f want1(400.f + defOf(deal[1]).platforms[0].at.x,
                                 1150.f - h0 - h1 +
                                     defOf(deal[1]).platforms[0].at.y);
        const bool placed = c0p0 && c0p0->position == want0 && c1p0 &&
                            c1p0->position == want1 &&
                            g.entities().size() == expectedEntities &&
                            g.ascentTopY() == top;
        // Determinism: same seed, same deal, same world; a
        // different seed deals a different climb. Captured BEFORE
        // any update runs — patrol platforms move once the world
        // ticks, and a moved patrol is not a different deal.
        const auto layout = [](Game& game) {
            std::vector<std::pair<std::string, sf::Vector2f>> out;
            for (const auto& e : game.entities())
                out.push_back({e.name, e.position});
            return out;
        };
        const auto layoutG = layout(g);
        Game g2;
        g2.loadChunks(chunksFile.string());
        g2.buildAscent(42, 4);
        const bool sameDeal = layout(g2) == layoutG;
        Game g3;
        g3.loadChunks(chunksFile.string());
        g3.buildAscent(7, 4);
        const bool otherDeal = layout(g3) != layoutG;
        // The player stands on the floor as a platformer.
        for (int i = 0; i < 30; ++i)
            g.update(1.f / 60.f);
        const Entity& player = g.entities()[g.playerIndex()];
        const bool standing = player.grounded &&
                              std::abs(player.position.y - 1100.f) < 1.f &&
                              player.moveModel == MoveModel::Platformer;
        // Patrol mapping, forced: a one-chunk vocabulary can only
        // deal Alpha, so its patrol's world target is exact.
        const fs::path soloFile = dir / "solo.txt";
        writeFile(soloFile,
                  "chunk=Alpha\n"
                  "height=200\n"
                  "platform=340|165|120|1\n"
                  "platform=100|100|120|1|300|40|70\n");
        Game g4;
        g4.loadChunks(soloFile.string());
        g4.buildAscent(99, 1);
        const Entity* patrol = nullptr;
        for (const auto& e : g4.entities())
            if (e.name == "Chunk0P1")
                patrol = &e;
        const bool patrolMapped =
            patrol && patrol->platformSpeed == 70.f &&
            patrol->platformTo - patrol->position ==
                sf::Vector2f(200.f, -60.f);
        check(parsed && noAdjacentRepeat && placed && standing &&
                  sameDeal && otherDeal && patrolMapped,
              "chunk model: chunks parse, the deal is seeded and "
              "stacks exactly, patrols map to world space, the "
              "player stands at the climb's foot");
        fs::remove_all(dir);
    }

    // --- The rising pressure (Lesson 74) -------------------------------
    {
        namespace fs = std::filesystem;
        const fs::path dir = tempPath("pressure");
        fs::remove_all(dir);
        fs::create_directories(dir);
        writeFile(dir / "chunks.txt",
                  "chunk=Solo\n"
                  "height=200\n"
                  "platform=340|165|120|1\n"
                  "platform=300|105|120|1\n"
                  "platform=340|25|120|1\n");
        // A fresh Gold Rush world has no pressure at all.
        Game plain;
        for (int i = 0; i < 120; ++i)
            plain.update(1.f / 60.f);
        const bool noPressure = !plain.pressureActive() &&
                                !plain.caught() &&
                                plain.pressureY() == 0.f;
        // The idle climber: grace, rise, caught — on schedule.
        Game g;
        g.loadChunks((dir / "chunks.txt").string());
        g.buildAscent(5, 3);
        float yAt2s = -1.f, yAt200 = -1.f;
        int caughtFrame = -1;
        for (int i = 1; i <= 600; ++i)
        {
            g.update(1.f / 60.f);
            if (i == 120)
                yAt2s = g.pressureY();
            if (i == 200)
                yAt200 = g.pressureY();
            if (g.caught() && caughtFrame < 0)
                caughtFrame = i;
        }
        const float frozenY = g.pressureY();
        for (int i = 0; i < 60; ++i)
            g.update(1.f / 60.f);
        const bool onSchedule =
            yAt2s == 1200.f && yAt200 < 1190.f && yAt200 > 1160.f &&
            caughtFrame > 240 && caughtFrame < 300 &&
            g.pressureY() == frozenY && g.heightReached() == 0.f;
        // The climbing player banks a jump's worth of height and
        // keeps it after the pressure takes him.
        Game c;
        c.loadChunks((dir / "chunks.txt").string());
        c.buildAscent(5, 3);
        const Entity& climber = c.entities()[c.playerIndex()];
        for (int i = 0; i < 30; ++i)
            c.update(1.f / 60.f);
        Game::InputState in;
        in.jump = true;
        c.setInput(in);
        bool released = false;
        for (int i = 0; i < 1200 && !c.caught(); ++i)
        {
            c.update(1.f / 60.f);
            if (!released && climber.velocity.y >= 0.f)
            {
                in.jump = false;
                c.setInput(in);
                released = true;
            }
        }
        c.clearInput();
        const float bankedHeight = c.heightReached();
        for (int i = 0; i < 60; ++i)
            c.update(1.f / 60.f);
        const bool banked = c.caught() && bankedHeight > 85.f &&
                            bankedHeight < 108.f &&
                            c.heightReached() == bankedHeight;
        check(noPressure && onSchedule && banked,
              "rising pressure: grace, then the line climbs on "
              "schedule; height is banked at the highest feet and "
              "kept after the catch; Gold Rush has none");
        fs::remove_all(dir);
    }

    // --- The endless deal (Lesson 75) ----------------------------------
    {
        namespace fs = std::filesystem;
        const fs::path dir = tempPath("endless-deal");
        fs::remove_all(dir);
        fs::create_directories(dir);
        writeFile(dir / "chunks.txt",
                  "chunk=Solo\n"
                  "height=200\n"
                  "platform=340|165|120|1\n"
                  "platform=300|105|120|1\n"
                  "platform=340|25|120|1\n");
        Game g;
        g.loadChunks((dir / "chunks.txt").string());
        g.buildAscent(5, 3);
        g.pressureDelay = 0.f;  // the test sets the tempo: the
        g.pressureSpeed = 60.f; // line climbs, the bot climbs
        // (at 120 u/s the line reaches the floor at 0.42 s —
        // before the bot's first lift at 0.75 s; the tempo must
        // let the staged climb live, which is the test's choice
        // to make, not the machinery's fault)
        const bool windowAtStart = g.liveChunkCount() == 3 &&
                                   g.chunksDealtTotal() == 3 &&
                                   g.ascentTopY() == 550.f;
        // The staging bot: every 45 frames it is lifted 260 units
        // (the jumping itself is Lessons 59/73's proof; what is
        // under test here is the window machinery). The score is
        // sampled every frame once recycling begins: it may never
        // fall across a scroll.
        bool sawRecycle = false;
        float scoreFloor = 0.f;
        bool scoreHeld = true;
        for (int i = 1; i <= 3600 && !g.caught(); ++i)
        {
            if (i % 45 == 0) // re-fetched every time: recycling
                g.entities()[g.playerIndex()].position.y -=
                    260.f; // erases entities, references would rot
            g.update(1.f / 60.f);
            if (g.chunksDealtTotal() > 3)
            {
                if (!sawRecycle)
                {
                    sawRecycle = true;
                    scoreFloor = g.heightReached();
                }
                else if (g.heightReached() < scoreFloor - 0.01f)
                    scoreHeld = false;
                scoreFloor = std::max(scoreFloor, g.heightReached());
            }
        }
        const bool streamed = sawRecycle && g.chunksDealtTotal() >= 6 &&
                              g.liveChunkCount() == 3 &&
                              g.ascentTopY() == 550.f &&
                              g.scrollTotal() >= 600.f;
        const bool survived = !g.caught() && g.heightReached() > 500.f;
        check(windowAtStart && streamed && scoreHeld && survived,
              "endless deal: consumed chunks pop, the world scrolls "
              "by their height, fresh chunks deal in — the window "
              "never changes size and the score never falls");
        fs::remove_all(dir);
    }

    // --- The project says what kind of game it is (Lesson 76) ----------
    {
        namespace fs = std::filesystem;
        const fs::path dir = tempPath("project-mode");
        fs::remove_all(dir);
        fs::create_directories(dir);
        writeFile(dir / "project.myengine",
                  "format=1\nname=Climb Test\nseed=777\nscene=\n"
                  "mode=ascent\nchunks=chunks.txt\n");
        Project p;
        const bool parsed = loadProjectFolder(dir.string(), p) &&
                            p.mode == "ascent" &&
                            p.chunksFile == "chunks.txt" &&
                            p.seed == 777u;
        const bool savedKind = saveProject(p);
        Project q;
        const bool roundTripped =
            savedKind && loadProjectFolder(dir.string(), q) &&
            q.mode == "ascent" && q.chunksFile == "chunks.txt";
        // A scene game's saved card gains no new keys.
        Project sceneGame;
        sceneGame.root = dir.string();
        sceneGame.name = "Plain";
        saveProject(sceneGame);
        std::ifstream in(dir / "project.myengine");
        const std::string card((std::istreambuf_iterator<char>(in)),
                               std::istreambuf_iterator<char>());
        const bool sceneCardClean =
            card.find("mode=") == std::string::npos &&
            card.find("chunks=") == std::string::npos;
        // The shipped Temple Run card, when the suite runs from
        // the repo root (elsewhere this clause is vacuous — the
        // runtime boot is Lesson 76's shipped-card proof).
        Project temple;
        const bool templeOk =
            !fs::exists("games/temple-run") ||
            (loadProjectFolder("games/temple-run", temple) &&
             temple.mode == "ascent" &&
             temple.chunksFile == "chunks.txt");
        check(parsed && roundTripped && sceneCardClean && templeOk,
              "project mode: ascent cards parse and round-trip; "
              "scene cards stay byte-clean; Temple Run ships as an "
              "ascent");
        fs::remove_all(dir);
    }

    // --- Scenes live in the project as files (Lesson 64) ---------------
    {
        namespace fs = std::filesystem;
        const fs::path dir = tempPath("scene-place");
        fs::remove_all(dir);
        Project sceneProject;
        const bool made = createProjectFolder(dir.string(), "Scene Game",
                                              sceneProject);
        Game sg;
        const std::size_t before = sg.entities().size();
        const std::string sceneFile =
            projectPath(sceneProject, "scenes/level1.scene");
        const bool saved = sg.saveScene(sceneFile);
        sg.newRun(777); // a different world entirely
        const bool loadedBack = sg.loadScene(sceneFile);
        check(made && saved && loadedBack &&
                  sg.entities().size() == before &&
                  fs::exists(dir / "scenes" / "level1.scene"),
              "scenes: a scene saves into scenes/ and loads back whole");
        fs::remove_all(dir);
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
