#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <iostream>
#include "Camera.hpp"
#include "Config.hpp"
#include "Project.hpp"
#include "Version.hpp"
#include "Game.hpp"
#include "Log.hpp"
#include "SpriteSheet.hpp"
#include <algorithm>
#include <cstdio>
#include <random>

// Editor picking (Lesson 22): the index of the topmost entity whose
// sprite box contains the world point, or -1. Topmost = draw order
// reversed: the player draws last, then later spawns above earlier.
int pickEntity(const Game& game, sf::Vector2f world)
{
    const auto& list = game.entities();
    for (int pass = 0; pass < 2; ++pass)
        for (int i = static_cast<int>(list.size()) - 1; i >= 0; --i)
        {
            const Entity& e = list[static_cast<std::size_t>(i)];
            if ((pass == 0) != e.playerControlled)
                continue;
            if (world.x >= e.position.x && world.x <= e.position.x + e.size.x &&
                world.y >= e.position.y && world.y <= e.position.y + e.size.y)
                return i;
        }
    return -1;
}

int main()
{
    Log::info(std::string("MyEngine ") + engine::VERSION + " - Editor");
    // The project file (Lesson 41): what game is being edited. The
    // editor reads it for identity; the seed/scene it names are the
    // Runtime's business — the editor's own seed config stays live
    // so worlds can be experimented with without touching the file.
    Project project;
    bool haveProject = loadProjectFolder(".", project);

    Config config;
    config.setDefault("windowWidth", "1280");
    config.setDefault("windowHeight", "720");
    config.setDefault("seed", "12345");
    config.setDefault("noiseScale", "0.01");
    config.setDefault("playerSpeed", "200");
    config.setDefault("volume", "80");
    config.setDefault("music", "1");
    if (!config.load("engine.ini"))
        config.save("engine.ini");

    float volume = static_cast<float>(config.getInt("volume", 80));
    sf::Listener::setGlobalVolume(volume);

    sf::RenderWindow window(
        sf::VideoMode({static_cast<unsigned int>(config.getInt("windowWidth", 1280)),
                       static_cast<unsigned int>(config.getInt("windowHeight", 720))}),
        haveProject ? ("MyEngine Editor - " + project.name) : "MyEngine Editor");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // Windows move from their title bars only. The default (drag from
    // anywhere in the body) makes the Game View itself follow the
    // mouse during an entity drag — the gesture has to belong to the
    // canvas, not the window frame. (Found by Lesson 25's first drag.)
    ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;
    Game game;
    if (static_cast<std::uint32_t>(config.getInt("seed", 12345)) != game.seed())
        game.setSeed(static_cast<std::uint32_t>(config.getInt("seed", 12345)));
    if (config.getFloat("noiseScale", 0.01f) != game.noiseFrequency())
        game.setNoiseFrequency(config.getFloat("noiseScale", 0.01f));
    game.speed = config.getFloat("playerSpeed", 200.f);
    if (config.getInt("music", 1))
        game.startMusic("assets/music.wav");

    Camera camera;
    bool followCam = true;
    float zoom = 1.f;
    int selected = -1; // selected entity index (editor state, Lesson 22)
    bool dragging = false; // drag-to-move in progress (Lesson 25)
    sf::Vector2f dragOffset; // entity position minus grab point, world space
    // Undo/redo history (Lesson 27): snapshots of the whole entity
    // list, taken BEFORE each committed editor action.
    std::vector<std::vector<Entity>> undoStack;
    std::vector<std::vector<Entity>> redoStack;
    // Play-test session (Lesson 28): a snapshot taken when the test
    // starts; stopping the test restores it exactly.
    bool testing = false;
    std::vector<Entity> testSnapshot;
    float testSnapshotTime = 0.f;

    // Procgen panel state, hoisted so scene loading can re-sync it.
    int seedInput = static_cast<int>(game.seed());
    float noiseFreq = game.noiseFrequency();

    sf::Clock clock;
    int frames = 0;
    float fpsTimer = 0.f;

    sf::RenderTexture gameView(sf::Vector2u(800, 600));
    sf::Vector2f panelSize{800.f, 600.f};

    bool playing = true;

    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();

        // FPS counter: print once per second
        ++frames;
        fpsTimer += dt;
        if (fpsTimer >= 1.f)
        {
            std::cout << "FPS: " << frames
                      << "  Frame time: " << (fpsTimer / frames * 1000.f) << " ms\n";
            frames = 0;
            fpsTimer = 0.f;
        }

        // Events: discrete things that happened (closing the window)
        while (const std::optional event = window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(window, *event);
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->code == sf::Keyboard::Key::Escape)
                {
                    window.close();
                }
                if (keyPressed->code == sf::Keyboard::Key::Space)
                {
                    playing = !playing;
                }
                if (keyPressed->code == sf::Keyboard::Key::F5)
                {
                    game.saveScene("scene.txt");
                }
                if (keyPressed->code == sf::Keyboard::Key::F9)
                {
                    if (game.loadScene("scene.txt"))
                    {
                        seedInput = static_cast<int>(game.seed());
                        noiseFreq = game.noiseFrequency();
                        selected = -1; // the entity list was replaced
                        dragging = false;
                        undoStack.clear(); // history belongs to the old scene
                        redoStack.clear();
                        testing = false; // and any test snapshot is stale
                    }
                }
            }
        }
        

        ImGui::SFML::Update(window, sf::seconds(dt));

        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

        bool stepClicked = false;
        ImGui::Begin("Engine Debug");
        ImGui::Text("FPS: %.1f", 1.0f / dt);
        ImGui::Text("Frame time: %.3f ms", dt * 1000.f);
        sf::Vector2f p = game.playerPosition();
        ImGui::Text("Player: (%.0f, %.0f) frame %d", p.x, p.y, game.playerFrame());
        ImGui::Text("Tile: %s", Game::tileName(game.playerTile()));
        ImGui::Text("Entities: %d", static_cast<int>(game.entities().size()));
        ImGui::Text("Gold: %d / %d%s", game.goldCollected(), game.goldTotal(),
                    game.won() ? " - CLEARED" : "");
        ImGui::Text("Run time: %.1f s", game.runTime());
        ImGui::Text("Textures cached: %d", static_cast<int>(game.textureCacheSize()));
        ImGui::Text("Sounds played: %d (cache %d)", game.soundsPlayed(),
                    static_cast<int>(game.soundCacheSize()));
        ImGui::Text("Music: %s (%.1f s)", game.musicPlaying() ? "playing" : "stopped",
                    game.musicOffset());
        if (game.musicPlaying())
        {
            if (ImGui::Button("Stop music"))
                game.stopMusic();
        }
        else if (ImGui::Button("Play music"))
            game.startMusic("assets/music.wav");
        if (ImGui::SliderFloat("Volume", &volume, 0.f, 100.f, "%.0f"))
            sf::Listener::setGlobalVolume(volume);
        if (selected >= 0 && selected < static_cast<int>(game.entities().size()))
        {
            const Entity& e = game.entities()[static_cast<std::size_t>(selected)];
            ImGui::Text("Selected: %s (%.0f, %.0f)", e.name.c_str(), e.position.x,
                        e.position.y);
        }
        else
            ImGui::Text("Selected: none");
        ImGui::Text("Camera: (%.0f, %.0f)", camera.center.x, camera.center.y);
        ImGui::Checkbox("Camera follow", &followCam);
        ImGui::Checkbox("Show hitboxes", &game.showHitboxes);
        ImGui::SliderFloat("Zoom", &zoom, 0.4f, 3.f);
        // Play-test mode (Lesson 28): Play snapshots the scene;
        // Stop restores it — experiment freely, keep nothing.
        if (!testing)
        {
            if (ImGui::Button("Play test (snapshot scene)"))
            {
                testSnapshot = game.entities();
                testSnapshotTime = game.time;
                testing = true;
                playing = true; // a test starts running
                Log::info("Play test started - scene snapshot taken");
            }
        }
        else
        {
            if (ImGui::Button("Stop test (restore scene)"))
            {
                game.replaceEntities(testSnapshot);
                game.time = testSnapshotTime;
                testing = false;
                selected = -1;
                dragging = false;
                Log::info("Play test stopped - scene restored");
            }
            ImGui::Text("TESTING: changes are discarded on Stop");
        }
        ImGui::Text("Game time: %.2f s", game.time);
        if (ImGui::Button(playing ? "Pause" : "Resume"))
        {
            playing = !playing;
        }
        ImGui::SameLine();
        if (ImGui::Button("Step"))
        {
            stepClicked = true;
        }
        ImGui::ColorEdit3("Clear color", game.clearColor);
        ImGui::SliderFloat("Speed", &game.speed, 50.f, 600.f);
        ImGui::SeparatorText("Scene");
        if (ImGui::Button("Save scene (F5)"))
        {
            game.saveScene("scene.txt");
        }
        ImGui::SameLine();
        if (ImGui::Button("Load scene (F9)"))
        {
            if (game.loadScene("scene.txt"))
            {
                seedInput = static_cast<int>(game.seed());
                noiseFreq = game.noiseFrequency();
                selected = -1; // the entity list was replaced
                        dragging = false;
                        undoStack.clear(); // history belongs to the old scene
                        redoStack.clear();
                        testing = false; // and any test snapshot is stale
            }
        }
        ImGui::End();

        // Undo/redo (Lesson 27): snapshot the entity list BEFORE a
        // committed action; undo swaps the current list onto the
        // redo stack and restores the snapshot.
        auto pushUndoState = [&](std::vector<Entity> state) {
            undoStack.push_back(std::move(state));
            if (undoStack.size() > 100)
                undoStack.erase(undoStack.begin());
            redoStack.clear();
        };
        auto pushUndo = [&]() { pushUndoState(game.entities()); };
        auto doUndo = [&]() {
            if (undoStack.empty())
                return;
            redoStack.push_back(game.entities());
            game.replaceEntities(undoStack.back());
            undoStack.pop_back();
            selected = -1;
            dragging = false;
            Log::info("Undo (" + std::to_string(game.entities().size()) +
                      " entities)");
        };
        auto doRedo = [&]() {
            if (redoStack.empty())
                return;
            undoStack.push_back(game.entities());
            game.replaceEntities(redoStack.back());
            redoStack.pop_back();
            selected = -1;
            dragging = false;
            Log::info("Redo (" + std::to_string(game.entities().size()) +
                      " entities)");
        };
        if (!ImGui::GetIO().WantTextInput)
        {
            bool ctrl = ImGui::GetIO().KeyCtrl;
            bool shift = ImGui::GetIO().KeyShift;
            if (ctrl && shift && ImGui::IsKeyPressed(ImGuiKey_Z))
                doRedo();
            else if (ctrl && ImGui::IsKeyPressed(ImGuiKey_Y))
                doRedo();
            else if (ctrl && ImGui::IsKeyPressed(ImGuiKey_Z))
                doUndo();
        }

        // Spawn/delete plumbing (Lesson 26), shared by the Hierarchy
        // buttons and the Delete key. If the count doesn't drop, the
        // removal was refused (the player) and the selection stays.
        auto deleteSelected = [&]() {
            if (selected < 0 || selected >= static_cast<int>(game.entities().size()))
                return;
            if (selected != static_cast<int>(game.playerIndex()))
                pushUndo(); // a refused delete leaves no history
            std::size_t before = game.entities().size();
            game.removeEntity(static_cast<std::size_t>(selected));
            if (game.entities().size() < before)
            {
                selected = -1;
                dragging = false;
            }
        };
        if (ImGui::IsKeyPressed(ImGuiKey_Delete) && !ImGui::GetIO().WantTextInput)
            deleteSelected();

        // Hierarchy (Lesson 23): every entity by name; clicking a row
        // drives the SAME selection the Game View picking writes.
        ImGui::Begin("Hierarchy");
        {
            const auto& list = game.entities();
            ImGui::Text("Entities: %d", static_cast<int>(list.size()));
            ImGui::Separator();
            for (int i = 0; i < static_cast<int>(list.size()); ++i)
            {
                const Entity& e = list[static_cast<std::size_t>(i)];
                // ##<index>: the visible label stays the bare name
                // while ImGui gets the unique ID duplicates require.
                std::string label = e.name + "##" + std::to_string(i);
                if (ImGui::Selectable(label.c_str(), selected == i))
                {
                    selected = i;
                    game.playSound("assets/click.wav"); // UI feedback (Lesson 31)
                    Log::info("Selected: " + e.name + " (index " +
                              std::to_string(i) + ")");
                }
            }
            ImGui::Separator();
            if (ImGui::Button("Spawn crate"))
            {
                // At the camera's centre, and it arrives selected —
                // ready to drag wherever it actually belongs.
                pushUndo();
                sf::Vector2f at = camera.center - sf::Vector2f(20.f, 20.f);
                selected = static_cast<int>(game.spawnEntity(
                    "Crate", at, {40.f, 40.f}, sf::Color(220, 140, 60), false,
                    sprites::CRATE));
            }
            ImGui::SameLine();
            if (ImGui::Button("Delete selected"))
                deleteSelected();
            if (ImGui::Button("Undo"))
                doUndo();
            ImGui::SameLine();
            if (ImGui::Button("Redo"))
                doRedo();
            ImGui::SameLine();
            ImGui::Text("%zu back / %zu fwd", undoStack.size(), redoStack.size());
        }
        ImGui::End();

        // Inspector (Lesson 24): the selected entity's properties,
        // editable — edits write straight into the live game.
        ImGui::Begin("Inspector");
        if (selected >= 0 && selected < static_cast<int>(game.entities().size()))
        {
            Entity& e = game.entities()[static_cast<std::size_t>(selected)];
            ImGui::Text("Index %d", selected);

            // Name edits through a buffer that re-syncs whenever the
            // selection changes (typing must not fight the game).
            static char nameBuf[64] = "";
            static int nameFor = -2;
            if (nameFor != selected)
            {
                std::snprintf(nameBuf, sizeof(nameBuf), "%s", e.name.c_str());
                nameFor = selected;
            }
            // Undo hooks: drag/text/colour widgets snapshot on
            // ACTIVATION — the press frame, before the gesture's
            // first change lands — so one gesture = one undo step.
            if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf)))
                e.name = nameBuf;
            if (ImGui::IsItemActivated())
                pushUndo();

            ImGui::DragFloat2("Position", &e.position.x, 1.f);
            if (ImGui::IsItemActivated())
                pushUndo();
            ImGui::DragFloat2("Size", &e.size.x, 1.f, 1.f, 500.f);
            if (ImGui::IsItemActivated())
                pushUndo();

            float rgb[3] = {e.color.r / 255.f, e.color.g / 255.f, e.color.b / 255.f};
            if (ImGui::ColorEdit3("Colour", rgb))
            {
                e.color = {static_cast<std::uint8_t>(rgb[0] * 255.f),
                           static_cast<std::uint8_t>(rgb[1] * 255.f),
                           static_cast<std::uint8_t>(rgb[2] * 255.f)};
            }
            if (ImGui::IsItemActivated())
                pushUndo();

            // A checkbox flips DURING its call, so its pre-state is
            // reconstructed and pushed explicitly.
            bool wasSolid = e.solid;
            if (ImGui::Checkbox("Solid", &e.solid))
            {
                std::vector<Entity> prior = game.entities();
                prior[static_cast<std::size_t>(selected)].solid = wasSolid;
                pushUndoState(std::move(prior));
            }
            ImGui::Text("Sprite: %d", e.sprite);
            ImGui::Text("Player-controlled: %s", e.playerControlled ? "yes" : "no");
        }
        else
        {
            ImGui::TextWrapped(
                "Nothing selected. Click an entity in the Game View or the Hierarchy.");
        }
        ImGui::End();

        ImGui::Begin("Procgen");
        ImGui::Text("Master seed: %u", game.seed());
        ImGui::InputInt("Seed", &seedInput);
        if (ImGui::Button("Apply seed"))
            game.setSeed(static_cast<std::uint32_t>(seedInput));
        ImGui::SameLine();
        if (ImGui::Button("Random seed"))
        {
            std::random_device rd;
            seedInput = static_cast<int>(rd());
            game.setSeed(static_cast<std::uint32_t>(seedInput));
        }
        // noiseFreq itself is declared with the other panel state, above the loop.
        if (ImGui::SliderFloat("Noise scale", &noiseFreq, 0.004f, 0.03f))
            game.setNoiseFrequency(noiseFreq);
        if (ImGui::Button("Reshuffle terrain"))
            game.generateTerrain();
        ImGui::SameLine();
        if (ImGui::Button("Reshuffle decor"))
            game.generateDecor();
        ImGui::End();

        // The Feel panel (Lesson 53): Arc 2's whole layer, live.
        // Every slider writes Game::feel — the same struct the
        // harness pins — and the Fire buttons trigger the game's
        // own event hooks, so the panel is a workbench, not a poster.
        ImGui::Begin("Feel");
        ImGui::Checkbox("Feel enabled (master)", &game.feel.enabled);
        ImGui::Checkbox("Particles & rings", &game.feel.particlesEnabled);
        ImGui::SliderFloat("Spawn pop (s)", &game.feel.popDuration, 0.05f, 0.8f);
        ImGui::SliderFloat("Shake max", &game.feel.shakeMax, 0.f, 30.f);
        ImGui::SliderFloat("Trauma decay (/s)", &game.feel.traumaDecay, 0.4f, 3.f);
        ImGui::SliderFloat("Win hit-stop (s)", &game.feel.hitStopWin, 0.f, 0.3f);
        ImGui::SliderFloat("Streak window (s)", &game.feel.streakWindow, 0.3f, 3.f);
        ImGui::Separator();
        if (ImGui::Button("Fire: pickup"))
            game.onGoldCollected(game.playerCenter());
        ImGui::SameLine();
        if (ImGui::Button("Fire: shake"))
            game.addTrauma(0.6f);
        ImGui::SameLine();
        if (ImGui::Button("Fire: win"))
            game.onRunWon();
        ImGui::Separator();
        ImGui::Text("Trauma: %.2f   Streak: %d", game.trauma(), game.collectStreak());
        ImGui::Text("Particles alive: %zu   Rings: %zu", game.particleCount(),
                    game.ringCount());
        ImGui::End();

        // The Physics panel (Lesson 58): Arc 3's numbers, live —
        // the same public fields the harness pins. Solidity is a
        // row of checkboxes over Game's table, and the model
        // switch flips the player between Gold Rush and platformer
        // so the whole toolkit can be felt without leaving the
        // editor.
        ImGui::Begin("Physics");
        ImGui::SliderFloat("Gravity", &game.gravity, 200.f, 4000.f);
        ImGui::SliderFloat("Terminal velocity", &game.terminalVelocity, 100.f,
                           2000.f);
        ImGui::SliderFloat("Run speed", &game.platformerSpeed, 50.f, 600.f);
        ImGui::SliderFloat("Jump speed", &game.jumpSpeed, 100.f, 1200.f);
        ImGui::SliderFloat("Coyote time (s)", &game.coyoteTime, 0.f, 0.3f);
        ImGui::SliderFloat("Jump buffer (s)", &game.jumpBuffer, 0.f, 0.3f);
        ImGui::SliderFloat("Jump cut", &game.jumpCut, 0.1f, 1.f);
        ImGui::Separator();
        ImGui::Text("Solid tiles (platformer):");
        const char* tileNames[6] = {"Deep water", "Water", "Sand",
                                    "Grass",     "Forest", "Rock"};
        for (int id = 0; id < 6; ++id)
        {
            bool solid = game.tileSolid(id);
            if (ImGui::Checkbox(tileNames[id], &solid))
                game.setTileSolid(id, solid);
        }
        ImGui::Separator();
        for (auto& e : game.entities())
        {
            if (!e.playerControlled)
                continue;
            bool platformer = e.moveModel == MoveModel::Platformer;
            if (ImGui::Checkbox("Player is a platformer", &platformer))
                e.moveModel = platformer ? MoveModel::Platformer
                                         : MoveModel::TopDown;
            ImGui::Text("Grounded: %s   Velocity: (%.0f, %.0f)",
                        e.grounded ? "yes" : "no", e.velocity.x, e.velocity.y);
        }
        ImGui::Separator();
        if (ImGui::Button("Build climber test"))
            game.buildClimberTest();
        ImGui::End();

        // The Project panel (Lesson 61): the place, visible — a
        // tree over listProjectFiles, rebuilt on Refresh, with the
        // selected file's facts below. Files are Selectables whose
        // ImGui ID is the full project-relative path (###), so two
        // files with the same leaf name in different folders can
        // never collide.
        ImGui::Begin("Project");
        if (!haveProject)
        {
            ImGui::TextDisabled(
                "No project loaded — this folder is not a project place.");
        }
        else
        {
            static std::vector<ProjectEntry> listing;
            static bool listingDirty = true;
            static std::string selectedFile;
            ImGui::Text("%s", project.name.c_str());
            ImGui::SameLine();
            if (ImGui::SmallButton("Refresh"))
                listingDirty = true;
            ImGui::TextDisabled("%s", project.root.c_str());
            ImGui::Separator();
            if (listingDirty)
            {
                listing = listProjectFiles(project);
                listingDirty = false;
            }
            for (const auto& entry : listing)
            {
                const std::string& rel = entry.relativePath;
                const std::size_t depth =
                    static_cast<std::size_t>(std::count(rel.begin(), rel.end(), '/'));
                const std::string leaf = rel.substr(rel.find_last_of('/') + 1);
                ImGui::Indent(depth * 14.f);
                if (entry.isDirectory)
                    ImGui::TextDisabled("%s/", leaf.c_str());
                else if (ImGui::Selectable((leaf + "###" + rel).c_str(),
                                            selectedFile == rel))
                    selectedFile = rel;
                ImGui::Unindent(depth * 14.f);
            }
            if (!selectedFile.empty())
            {
                ImGui::Separator();
                ImGui::TextWrapped("%s", selectedFile.c_str());
                for (const auto& entry : listing)
                    if (entry.relativePath == selectedFile &&
                        !entry.isDirectory)
                        ImGui::TextDisabled(
                            "%llu bytes",
                            static_cast<unsigned long long>(entry.size));
            }
        }
        ImGui::End();

        ImGui::Begin("Console");
        if (ImGui::Button("Clear"))
            Log::clear();
        ImGui::SameLine();
        static bool autoScroll = true;
        ImGui::Checkbox("Auto-scroll", &autoScroll);
        ImGui::BeginChild("ConsoleLog", ImVec2(0.f, 0.f), ImGuiChildFlags_Borders);
        for (const auto& entry : Log::entries())
        {
            ImVec4 color = entry.level == LogLevel::Warning
                               ? ImVec4(1.f, 0.8f, 0.3f, 1.f)
                               : entry.level == LogLevel::Error
                               ? ImVec4(1.f, 0.35f, 0.35f, 1.f)
                               : ImVec4(0.85f, 0.85f, 0.85f, 1.f);
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::TextUnformatted(entry.message.c_str());
            ImGui::PopStyleColor();
        }
        static std::size_t lastEntryCount = 0;
        if (autoScroll && Log::entries().size() != lastEntryCount)
            ImGui::SetScrollHereY(1.f);
        lastEntryCount = Log::entries().size();
        ImGui::EndChild();
        ImGui::End();


        if (playing || stepClicked)
            game.update(dt);

        sf::Vector2u wantSize(panelSize);
        if (wantSize.x > 0 && wantSize.y > 0 && wantSize != gameView.getSize())
            gameView.resize(wantSize);

        // The Game View's camera: follows the player unless told not
        // to; zoom sets the visible height; the view is rebuilt every
        // frame from the texture's current size.
        camera.viewHeight = 600.f / zoom;
        sf::Vector2f canvasSize(gameView.getSize());
        if (followCam)
            camera.follow(game.playerCenter(), game.playerVelocity(), dt,
                          game.worldSize(), canvasSize);
        else
            camera.clampToWorld(game.worldSize(), canvasSize);
        sf::View worldView = camera.makeView(canvasSize);
        worldView.move(game.shakeOffset()); // camera feel (Lesson 51)
        gameView.setView(worldView);

        game.render(gameView);

        // Selection highlight: an editor overlay drawn into the same
        // texture, in the same view — the game itself knows nothing
        // about selection.
        if (selected >= 0 && selected < static_cast<int>(game.entities().size()))
        {
            const Entity& e = game.entities()[static_cast<std::size_t>(selected)];
            sf::RectangleShape highlight;
            highlight.setSize(e.size + sf::Vector2f(4.f, 4.f));
            highlight.setPosition(e.position - sf::Vector2f(2.f, 2.f));
            highlight.setFillColor(sf::Color::Transparent);
            highlight.setOutlineColor(sf::Color::Yellow);
            highlight.setOutlineThickness(2.f);
            gameView.draw(highlight);
        }
        gameView.display();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
        ImGui::Begin("Game View");
        ImVec2 avail = ImGui::GetContentRegionAvail();
        if (avail.x > 0.f && avail.y > 0.f)
            panelSize = sf::Vector2(avail.x, avail.y);
        ImVec2 imagePos = ImGui::GetCursorScreenPos();
        ImGui::Image(gameView, panelSize);
        if (ImGui::IsWindowHovered())
        {
            // Left click picks: panel pixels -> world via the view the
            // texture was rendered with, then a topmost-first search.
            // A successful pick also starts a drag (Lesson 25): the
            // grab offset is kept so the entity doesn't jump to centre
            // itself on the cursor.
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                ImVec2 mouse = ImGui::GetMousePos();
                sf::Vector2f world = gameView.mapPixelToCoords(
                    {static_cast<int>(mouse.x - imagePos.x),
                     static_cast<int>(mouse.y - imagePos.y)});
                selected = pickEntity(game, world);
                if (selected >= 0)
                {
                    game.playSound("assets/click.wav"); // UI feedback (Lesson 31)
                    Log::info("Selected: " +
                              game.entities()[static_cast<std::size_t>(selected)].name +
                              " (index " + std::to_string(selected) + ")");
                    pushUndo(); // one snapshot per drag, taken at the grab
                    dragging = true;
                    dragOffset =
                        game.entities()[static_cast<std::size_t>(selected)].position -
                        world;
                }
                else
                    Log::info("Selection cleared");
            }
            // Mouse wheel zooms; right-drag grabs the world and pans
            // (disable Camera follow to stay where you pan to).
            float wheel = ImGui::GetIO().MouseWheel;
            if (wheel != 0.f)
                zoom = std::clamp(zoom * (1.f + wheel * 0.1f), 0.4f, 3.f);
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
            {
                ImVec2 drag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
                sf::Vector2f view = camera.viewSize(panelSize);
                camera.center -= sf::Vector2f(drag.x * view.x / panelSize.x,
                                              drag.y * view.y / panelSize.y);
                ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
            }
        }
        // Drag-to-move continues outside the hover check: a drag that
        // leaves the panel mid-pull still tracks, and the release is
        // caught wherever it happens. The move is logged once, on
        // drop — never per frame.
        if (dragging)
        {
            bool valid = selected >= 0 &&
                         selected < static_cast<int>(game.entities().size());
            if (valid && ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                ImVec2 mouse = ImGui::GetMousePos();
                sf::Vector2f world = gameView.mapPixelToCoords(
                    {static_cast<int>(mouse.x - imagePos.x),
                     static_cast<int>(mouse.y - imagePos.y)});
                game.entities()[static_cast<std::size_t>(selected)].position =
                    world + dragOffset;
            }
            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
            {
                dragging = false;
                if (valid)
                {
                    const Entity& e = game.entities()[static_cast<std::size_t>(selected)];
                    Log::info("Moved " + e.name + " to (" +
                              std::to_string(static_cast<int>(e.position.x)) + ", " +
                              std::to_string(static_cast<int>(e.position.y)) + ")");
                }
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();

        window.clear(sf::Color(25, 25, 25));
        ImGui::SFML::Render(window);
        window.display();
    }
    ImGui::SFML::Shutdown();
    return 0;
}
