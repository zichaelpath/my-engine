#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include "Camera.hpp"
#include "Campaign.hpp"
#include "Config.hpp"
#include "Game.hpp"
#include "Log.hpp"
#include "Project.hpp"
#include "SaveData.hpp"
#include "Ui.hpp"
#include "Version.hpp"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <fstream>
#include <random>

// The Runtime's game frame (Lesson 33): a small state machine —
// Title, Playing, Paused, Won. The simulation only advances in
// Playing, so the title's backdrop is a frozen world and the run
// timer cannot start before the player does. Game rules stay in
// Game; this file only decides which screen presents them.
enum class Screen
{
    Title,
    Playing,
    Paused,
    Won,
    Caught,          // the climb's ending (Lesson 76)
    CampaignComplete // the campaign's ending (Lesson 79)
};

int main(int argc, char** argv)
{
    Log::info(std::string("MyEngine ") + engine::VERSION + " - Runtime");
    // The project file (Lesson 41) is read first: it says what
    // game this is. engine.ini below stays per-machine settings.
    // The project is a folder (Lesson 76): an optional argument
    // names it, so one runtime binary boots any game on the
    // engine — the working directory's project by default.
    const std::string projectFolder = argc > 1 ? argv[1] : ".";
    Project project;
    bool haveProject = loadProjectFolder(projectFolder, project);
    const bool ascent = haveProject && project.mode == "ascent";
    // A campaign (Lesson 79): a scene-mode project listing more
    // than one level. The campaign is the player's progress
    // through that list; the title screen presents it.
    Campaign campaign;
    const bool campaignMode =
        haveProject && !ascent && project.scenes.size() > 1;
    if (campaignMode)
        campaign.reset(project.scenes);
    int selectedLevel = 0;

    // The player's save (Lesson 81): campaign progress, best
    // times and heights, per-player settings — restored when the
    // saved campaign still matches this project's level list.
    SaveData save;
    const bool haveSave = haveProject && loadSave(project.root, save);
    if (campaignMode && haveSave && save.hasCampaign &&
        save.campaign.levels == project.scenes)
    {
        campaign = save.campaign;
        selectedLevel =
            campaign.finished() ? 0 : campaign.nextLevel();
        Log::info("Campaign progress restored from the save");
    }
    bool shakeOn = save.setting("shake", "on") != "off";
    bool newBest = false; // the end screens' celebration line
    auto persist = [&]() {
        if (!haveProject)
            return;
        if (campaignMode)
        {
            save.campaign = campaign;
            save.hasCampaign = true;
        }
        writeSave(project.root, save);
    };

    Config config;
    config.setDefault("windowWidth", "800");
    config.setDefault("windowHeight", "600");
    config.setDefault("seed", "12345");
    config.setDefault("noiseScale", "0.01");
    config.setDefault("playerSpeed", "200");
    config.setDefault("volume", "80");
    config.setDefault("music", "1");
    if (!config.load("engine.ini"))
        config.save("engine.ini");
    sf::Listener::setGlobalVolume(static_cast<float>(config.getInt("volume", 80)));

    sf::RenderWindow window(
        sf::VideoMode({static_cast<unsigned int>(config.getInt("windowWidth", 800)),
                       static_cast<unsigned int>(config.getInt("windowHeight", 600))}),
        haveProject ? project.name : "MyEngine Runtime");
    window.setFramerateLimit(60);

    Game game;
    game.hideSpawnMarkers = true; // shipped games hide them (L78)
    if (haveProject)
        game.setScriptRoot(project.root); // scripts resolve in the project (L70)
    if (static_cast<std::uint32_t>(config.getInt("seed", 12345)) != game.seed())
        game.setSeed(static_cast<std::uint32_t>(config.getInt("seed", 12345)));
    if (haveProject)
        game.setSeed(project.seed); // the project defines the game; it wins
    if (config.getFloat("noiseScale", 0.01f) != game.noiseFrequency())
        game.setNoiseFrequency(config.getFloat("noiseScale", 0.01f));
    if (haveProject && !project.startScene.empty())
    {
        // A named start scene loads when it exists; a fresh checkout
        // has no saved scene yet, which is normal, not an error.
        // The scene is named relative to the project root (L60).
        const std::string scenePath = projectPath(project, project.startScene);
        std::ifstream sceneFile(scenePath);
        if (sceneFile && !game.loadScene(scenePath))
            Log::warning("Project start scene failed to load: " + scenePath);
    }
    game.speed = config.getFloat("playerSpeed", 200.f);
    if (ascent)
    {
        // An ascent game (Lesson 76): the stage is dealt, not
        // generated — load the project's chunks and build the
        // climb now, so the title screen already stands at its
        // foot. The project's seed IS the temple: every run of
        // this copy of the game climbs the same mountain.
        const std::string chunksPath =
            projectPath(project, project.chunksFile);
        if (game.loadChunks(chunksPath))
            game.buildAscent(project.seed, 5);
        else
            Log::error("Ascent project without loadable chunks: " +
                       chunksPath);
    }
    if (config.getInt("music", 1))
        game.startMusic("assets/music.wav");

    // Text (Lesson 32/33): one bundled font, loaded once. If it is
    // missing the game still plays — only the overlays are disabled.
    sf::Font font;
    bool fontOk = font.openFromFile("assets/DejaVuSans.ttf");
    if (!fontOk)
        Log::error("Font failed to load: assets/DejaVuSans.ttf (overlays disabled)");
    // The screens are composed from shared UI widgets
    // (Lesson 80): Labels for text, Buttons for anything a
    // pointer can press, one Panel for the dim behind overlays.
    // Keyboard control is unchanged — every button mirrors a key.
    ui::Label hud(font, "", 20, sf::Color::White);
    hud.setPosition({12.f, 8.f});
    // The title is the project's name, shouted (Lesson 41): with no
    // project file it falls back to the game's built-in name.
    std::string titleText = haveProject ? project.name : "Gold Rush";
    std::transform(titleText.begin(), titleText.end(), titleText.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    ui::Label gameTitle(font, titleText, 52, sf::Color(255, 215, 80));
    ui::Label titleSub(font, "", 18, sf::Color::White);
    ui::Label titlePrompt(font, "", 20, sf::Color::White);
    ui::Label seedLine(font, "", 16, sf::Color(180, 180, 180));
    ui::Button playButton(font, "Play", 22);
    std::vector<ui::Button> levelButtons;
    if (campaignMode)
        for (int i = 0; i < campaign.levelCount(); ++i)
            levelButtons.emplace_back(font, "", 18);
    ui::Label pausedTitle(font, "Paused", 36, sf::Color::White);
    ui::Label pausedSub(font, "P to resume - Esc to quit", 18, sf::Color::White);
    ui::Button resumeButton(font, "Resume");
    ui::Label banner(font, "You collected all the gold!", 36, sf::Color(255, 215, 80));
    ui::Label winTime(font, "", 22, sf::Color::White);
    ui::Button wonAgainButton(font, "New world");
    ui::Label caughtTitle(font, "The pressure took you", 36, sf::Color(255, 120, 60));
    ui::Label caughtScore(font, "", 22, sf::Color::White);
    ui::Button caughtAgainButton(font, "Climb again");
    ui::Label campaignTitle(font, "Campaign complete!", 36, sf::Color(255, 215, 80));
    ui::Label campaignScore(font, "", 22, sf::Color::White);
    ui::Button campaignAgainButton(font, "Play it again");
    ui::Button campaignTitleButton(font, "Title");
    ui::Label bestLine(font, "", 20, sf::Color(255, 215, 80));
    ui::Panel dimPanel;

    Screen screen = Screen::Title;
    Camera camera;
    sf::Clock clock;

    // Scene travel (Lesson 78): which scene of the project the
    // player is standing in. An Exit names its target scene; the
    // scene being left names the Spawn the traveller lands on.
    std::string currentScene = haveProject ? project.startScene : "";
    auto sceneBase = [](const std::string& path) {
        std::string base = path;
        if (auto slash = base.find_last_of('/');
            slash != std::string::npos)
            base = base.substr(slash + 1);
        if (auto dot = base.find_last_of('.'); dot != std::string::npos)
            base = base.substr(0, dot);
        return base;
    };

    // Screen actions (Lesson 80): what Enter, R and the buttons
    // *do* lives here once — the keyboard switch and the mouse
    // routing below both call these, so the two input paths can
    // never drift apart.
    auto startPlay = [&]() {
        if (campaignMode)
        {
            // Enter the chosen level fresh: load it and land on
            // its default Spawn (Lesson 79).
            campaign.current = selectedLevel;
            const std::string target =
                campaign.levels[static_cast<std::size_t>(selectedLevel)];
            if (game.loadScene(projectPath(project, target)))
            {
                game.placePlayerAtSpawn("");
                currentScene = target;
            }
        }
        screen = Screen::Playing;
    };
    auto newRunAction = [&]() {
        game.newRun(std::random_device{}());
        newBest = false;
        screen = Screen::Playing;
    };
    auto climbAgainAction = [&]() {
        // The same temple, climbed again (Lesson 76): the
        // project seed re-deals the identical mountain.
        game.buildAscent(project.seed, 5);
        newBest = false;
        screen = Screen::Playing;
    };
    auto campaignAgainAction = [&]() {
        // The whole campaign again (Lesson 79).
        campaign.reset(project.scenes);
        selectedLevel = 0;
        persist(); // the reset is progress too (Lesson 81)
        const std::string target = campaign.levels[0];
        if (game.loadScene(projectPath(project, target)))
        {
            game.placePlayerAtSpawn("");
            currentScene = target;
        }
        screen = Screen::Title;
    };

    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                using Key = sf::Keyboard::Key;
                if (keyPressed->code == Key::F5)
                    game.saveScene("scene.txt");
                if (keyPressed->code == Key::F9)
                    game.loadScene("scene.txt");
                // What a key means depends on the screen — that is
                // the state machine's whole job.
                switch (screen)
                {
                    case Screen::Title:
                        if (campaignMode &&
                            keyPressed->code == Key::Down &&
                            selectedLevel + 1 < campaign.levelCount() &&
                            campaign.isUnlocked(selectedLevel + 1))
                            ++selectedLevel;
                        if (campaignMode &&
                            keyPressed->code == Key::Up &&
                            selectedLevel > 0)
                            --selectedLevel;
                        if (keyPressed->code == Key::Enter)
                            startPlay();
                        if (keyPressed->code == Key::K)
                        {
                            // A per-player preference (Lesson 81):
                            // camera shake, saved in save.dat —
                            // not engine.ini's machine settings.
                            shakeOn = !shakeOn;
                            save.setSetting("shake",
                                            shakeOn ? "on" : "off");
                            persist();
                        }
                        if (keyPressed->code == Key::Escape)
                            window.close();
                        break;
                    case Screen::Playing:
                        if (keyPressed->code == Key::P ||
                            keyPressed->code == Key::Escape)
                            screen = Screen::Paused;
                        break;
                    case Screen::Paused:
                        if (keyPressed->code == Key::P)
                            screen = Screen::Playing;
                        if (keyPressed->code == Key::Escape)
                            window.close();
                        break;
                    case Screen::Won:
                        if (keyPressed->code == Key::R)
                            newRunAction();
                        if (keyPressed->code == Key::Escape)
                            window.close();
                        break;
                    case Screen::Caught:
                        if (keyPressed->code == Key::R)
                            climbAgainAction();
                        if (keyPressed->code == Key::Escape)
                            window.close();
                        break;
                    case Screen::CampaignComplete:
                        if (keyPressed->code == Key::Enter)
                            screen = Screen::Title;
                        if (keyPressed->code == Key::R)
                            campaignAgainAction();
                        if (keyPressed->code == Key::Escape)
                            window.close();
                        break;
                }
            }

            // Mouse routing (Lesson 80): a click asks the visible
            // screen's buttons, in the same terms the keyboard
            // uses. Buttons were placed by the last frame's draw;
            // their rects are this frame's hit targets.
            if (const auto* mousePressed =
                    event->getIf<sf::Event::MouseButtonPressed>();
                mousePressed &&
                mousePressed->button == sf::Mouse::Button::Left)
            {
                const sf::Vector2f point{
                    static_cast<float>(mousePressed->position.x),
                    static_cast<float>(mousePressed->position.y)};
                switch (screen)
                {
                    case Screen::Title:
                        if (campaignMode)
                            for (std::size_t i = 0;
                                 i < levelButtons.size(); ++i)
                                if (levelButtons[i].clickable(point))
                                    selectedLevel =
                                        static_cast<int>(i);
                        if (playButton.clickable(point))
                            startPlay();
                        break;
                    case Screen::Paused:
                        if (resumeButton.clickable(point))
                            screen = Screen::Playing;
                        break;
                    case Screen::Won:
                        if (wonAgainButton.clickable(point))
                            newRunAction();
                        break;
                    case Screen::Caught:
                        if (caughtAgainButton.clickable(point))
                            climbAgainAction();
                        break;
                    case Screen::CampaignComplete:
                        if (campaignAgainButton.clickable(point))
                            campaignAgainAction();
                        if (campaignTitleButton.clickable(point))
                            screen = Screen::Title;
                        break;
                    case Screen::Playing:
                        break;
                }
            }
        }

        // Hot reload (Lesson 71): a shipped game can be developed
        // against, too — changed scripts swap in twice a second.
        static float scriptWatch = 0.f;
        scriptWatch += dt;
        if (scriptWatch > 0.5f)
        {
            scriptWatch = 0.f;
            game.reloadScriptsIfChanged();
        }

        if (screen == Screen::Playing)
        {
            game.update(dt);
            if (game.won())
            {
                // A finished run is a best-time candidate
                // (Lesson 81): recorded per world seed.
                newBest = save.recordTime(game.seed(), game.runTime());
                if (newBest)
                    persist();
                screen = Screen::Won;
            }
            if (game.caught())
            {
                newBest = save.recordHeight(
                    static_cast<int>(game.heightReached()));
                if (newBest)
                    persist();
                screen = Screen::Caught;
            }
            // A touched Goal (Lesson 79): the campaign records
            // the completion and the player travels onward to
            // the next uncompleted level — or the campaign ends.
            if (campaignMode && game.goalTouched())
            {
                const std::string from = sceneBase(currentScene);
                campaign.completeCurrent();
                game.clearGoalTouched();
                persist(); // progress hits the disk as it happens
                if (campaign.finished())
                    screen = Screen::CampaignComplete;
                else
                {
                    const std::string target =
                        campaign.levels[static_cast<std::size_t>(campaign.current)];
                    if (game.loadScene(projectPath(project, target)))
                    {
                        game.placePlayerAtSpawn(from);
                        currentScene = target;
                    }
                }
            }
            // A touched door (Lesson 78): the host does the
            // travelling — load the target scene inside the
            // project, land the player on its named Spawn.
            if (haveProject && !game.pendingExit().empty())
            {
                const std::string target = game.pendingExit();
                const std::string from = sceneBase(currentScene);
                if (game.loadScene(projectPath(project, target)))
                {
                    game.placePlayerAtSpawn(from);
                    currentScene = target;
                    // Free travel keeps the campaign's notion of
                    // "the level in play" honest (Lesson 79).
                    if (campaignMode)
                        for (int i = 0; i < campaign.levelCount(); ++i)
                            if (campaign.levels[static_cast<std::size_t>(i)] == target)
                                campaign.current = i;
                }
                else
                    game.clearPendingExit(); // a broken door
                                             // should not retry
                                             // every frame
            }
        }
        else if (screen == Screen::Won || screen == Screen::Caught ||
                 screen == Screen::CampaignComplete)
        {
            // The world stays alive behind the end screens — the
            // staged fountain (Lesson 52) plays out there, and the
            // climb's aftermath settles. Game ignores input once
            // won or caught; the player takes his bow.
            game.update(dt);
        }

        // The camera follows the player, clamped so the view never
        // leaves the world; it is re-derived every frame from the
        // window's current size, so resizes need no event handling.
        camera.follow(game.playerCenter(), game.playerVelocity(), dt,
                      game.worldSize(), sf::Vector2f(window.getSize()));
        sf::View worldView = camera.makeView(sf::Vector2f(window.getSize()));
        if (shakeOn)
            worldView.move(game.shakeOffset()); // camera feel (L51)
        window.setView(worldView);
        game.render(window);

        if (fontOk)
        {
            // Overlays in screen space: the default view, drawn after
            // the world, so they never move with the camera.
            window.setView(window.getDefaultView());
            float w = static_cast<float>(window.getSize().x);
            float h = static_cast<float>(window.getSize().y);
            char buf[128];

            if (screen != Screen::Title)
            {
                if (ascent)
                    std::snprintf(buf, sizeof(buf), "Height: %d",
                                  static_cast<int>(game.heightReached()));
                else if (campaignMode)
                    std::snprintf(buf, sizeof(buf),
                                  "Level %d of %d    Gold: %d / %d    Time: %d:%02d",
                                  campaign.current + 1,
                                  campaign.levelCount(),
                                  game.goldCollected(), game.goldTotal(),
                                  static_cast<int>(game.runTime()) / 60,
                                  static_cast<int>(game.runTime()) % 60);
                else
                    std::snprintf(buf, sizeof(buf), "Gold: %d / %d    Time: %d:%02d",
                                  game.goldCollected(), game.goldTotal(),
                                  static_cast<int>(game.runTime()) / 60,
                                  static_cast<int>(game.runTime()) % 60);
                hud.setText(buf);
                hud.draw(window);
            }

            // The pointer, for hover states (Lesson 80): overlay
            // space is the default view, where pixels are points.
            const sf::Vector2i mousePx = sf::Mouse::getPosition(window);
            const sf::Vector2f mousePoint{
                static_cast<float>(mousePx.x),
                static_cast<float>(mousePx.y)};
            dimPanel.rect = {{0.f, 0.f}, {w, h}};

            if (screen == Screen::Title)
            {
                dimPanel.draw(window);
                gameTitle.centreOn(w / 2.f, h / 2.f - 80.f);
                gameTitle.draw(window);
                if (ascent)
                    titleSub.setText(
                        "Climb as high as you can - the pressure is rising behind you");
                else if (campaignMode)
                    titleSub.setText(
                        "Reach each level's Goal - finish them all");
                else
                {
                    std::snprintf(buf, sizeof(buf),
                                  "Collect all %d gold dots - mind the water, it slows you down",
                                  game.goldTotal());
                    titleSub.setText(buf);
                }
                titleSub.centreOn(w / 2.f, h / 2.f - 25.f);
                titleSub.draw(window);
                titlePrompt.setText(
                    campaignMode ? "Up/Down or click picks a level"
                                 : "Enter or click Play");
                titlePrompt.centreOn(w / 2.f, h / 2.f + 25.f);
                titlePrompt.draw(window);
                std::snprintf(buf, sizeof(buf),
                              ascent ? "Temple seed: %u" : "World seed: %u",
                              static_cast<unsigned int>(
                                  ascent ? project.seed : game.seed()));
                seedLine.setText(std::string(buf) +
                                 (shakeOn ? "  -  Shake: on (K)"
                                          : "  -  Shake: off (K)"));
                seedLine.centreOn(w / 2.f, h / 2.f + 70.f);
                seedLine.draw(window);
                // The level list (Lesson 79), rebuilt from
                // widgets (Lesson 80): each level is a Button —
                // selected wears the gold ring, a locked level is
                // disabled, a finished one says so.
                float below = h / 2.f + 100.f;
                if (campaignMode)
                {
                    for (int i = 0; i < campaign.levelCount(); ++i)
                    {
                        const bool unlocked = campaign.isUnlocked(i);
                        const bool done = campaign.isCompleted(i);
                        std::snprintf(
                            buf, sizeof(buf), "%d. %s%s", i + 1,
                            sceneBase(campaign.levels[static_cast<std::size_t>(i)]).c_str(),
                            done ? "  [done]" : (unlocked ? "" : "  (locked)"));
                        ui::Button& row =
                            levelButtons[static_cast<std::size_t>(i)];
                        row.setText(buf);
                        row.enabled = unlocked;
                        row.selected = i == selectedLevel;
                        row.placeCentred(w / 2.f, below, 360.f, 36.f);
                        row.setHovered(row.contains(mousePoint));
                        row.draw(window);
                        below += 42.f;
                    }
                    below += 6.f;
                }
                playButton.placeCentred(w / 2.f, below, 200.f, 44.f);
                playButton.setHovered(playButton.contains(mousePoint));
                playButton.draw(window);
            }
            else if (screen == Screen::Paused)
            {
                dimPanel.draw(window);
                pausedTitle.centreOn(w / 2.f, h / 2.f - 20.f);
                pausedTitle.draw(window);
                pausedSub.centreOn(w / 2.f, h / 2.f + 25.f);
                pausedSub.draw(window);
                resumeButton.placeCentred(w / 2.f, h / 2.f + 55.f,
                                          180.f, 40.f);
                resumeButton.setHovered(
                    resumeButton.contains(mousePoint));
                resumeButton.draw(window);
            }
            else if (screen == Screen::Won)
            {
                dimPanel.draw(window);
                banner.centreOn(w / 2.f, h / 2.f - 45.f);
                banner.draw(window);
                const float best = save.bestTime(game.seed());
                if (best > 0.f)
                    std::snprintf(buf, sizeof(buf),
                                  "Time: %d:%02d    Best: %d:%02d",
                                  static_cast<int>(game.runTime()) / 60,
                                  static_cast<int>(game.runTime()) % 60,
                                  static_cast<int>(best) / 60,
                                  static_cast<int>(best) % 60);
                else
                    std::snprintf(buf, sizeof(buf), "Time: %d:%02d",
                                  static_cast<int>(game.runTime()) / 60,
                                  static_cast<int>(game.runTime()) % 60);
                winTime.setText(buf);
                winTime.centreOn(w / 2.f, h / 2.f + 5.f);
                winTime.draw(window);
                if (newBest)
                {
                    bestLine.setText("New best time!");
                    bestLine.centreOn(w / 2.f, h / 2.f + 100.f);
                    bestLine.draw(window);
                }
                wonAgainButton.placeCentred(w / 2.f, h / 2.f + 40.f,
                                            200.f, 42.f);
                wonAgainButton.setHovered(
                    wonAgainButton.contains(mousePoint));
                wonAgainButton.draw(window);
            }
            else if (screen == Screen::Caught)
            {
                dimPanel.draw(window);
                caughtTitle.centreOn(w / 2.f, h / 2.f - 45.f);
                caughtTitle.draw(window);
                if (save.templeBestHeight > 0)
                    std::snprintf(buf, sizeof(buf),
                                  "Height: %d    Best: %d",
                                  static_cast<int>(game.heightReached()),
                                  save.templeBestHeight);
                else
                    std::snprintf(buf, sizeof(buf), "Height: %d",
                                  static_cast<int>(game.heightReached()));
                caughtScore.setText(buf);
                caughtScore.centreOn(w / 2.f, h / 2.f + 5.f);
                caughtScore.draw(window);
                if (newBest)
                {
                    bestLine.setText("New best height!");
                    bestLine.centreOn(w / 2.f, h / 2.f + 100.f);
                    bestLine.draw(window);
                }
                caughtAgainButton.placeCentred(w / 2.f, h / 2.f + 40.f,
                                               220.f, 42.f);
                caughtAgainButton.setHovered(
                    caughtAgainButton.contains(mousePoint));
                caughtAgainButton.draw(window);
            }
            else if (screen == Screen::CampaignComplete)
            {
                dimPanel.draw(window);
                campaignTitle.centreOn(w / 2.f, h / 2.f - 45.f);
                campaignTitle.draw(window);
                std::snprintf(buf, sizeof(buf), "%d levels finished",
                              campaign.levelCount());
                campaignScore.setText(buf);
                campaignScore.centreOn(w / 2.f, h / 2.f + 5.f);
                campaignScore.draw(window);
                campaignAgainButton.placeCentred(w / 2.f - 110.f,
                                                 h / 2.f + 40.f,
                                                 200.f, 42.f);
                campaignAgainButton.setHovered(
                    campaignAgainButton.contains(mousePoint));
                campaignAgainButton.draw(window);
                campaignTitleButton.placeCentred(w / 2.f + 110.f,
                                                 h / 2.f + 40.f,
                                                 200.f, 42.f);
                campaignTitleButton.setHovered(
                    campaignTitleButton.contains(mousePoint));
                campaignTitleButton.draw(window);
            }
        }
        window.display();
    }
    return 0;
}
