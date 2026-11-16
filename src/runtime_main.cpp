#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include "Camera.hpp"
#include "Config.hpp"
#include "Game.hpp"
#include "Log.hpp"
#include "Project.hpp"
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
    Won
};

int main()
{
    Log::info(std::string("MyEngine ") + engine::VERSION + " - Runtime");
    // The project file (Lesson 41) is read first: it says what
    // game this is. engine.ini below stays per-machine settings.
    Project project;
    bool haveProject = loadProject("project.myengine", project);

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
        std::ifstream sceneFile(project.startScene);
        if (sceneFile && !game.loadScene(project.startScene))
            Log::warning("Project start scene failed to load: " + project.startScene);
    }
    game.speed = config.getFloat("playerSpeed", 200.f);
    if (config.getInt("music", 1))
        game.startMusic("assets/music.wav");

    // Text (Lesson 32/33): one bundled font, loaded once. If it is
    // missing the game still plays — only the overlays are disabled.
    sf::Font font;
    bool fontOk = font.openFromFile("assets/DejaVuSans.ttf");
    if (!fontOk)
        Log::error("Font failed to load: assets/DejaVuSans.ttf (overlays disabled)");
    auto makeText = [&](const std::string& s, unsigned int size, sf::Color fill) {
        sf::Text t(font, s, size);
        t.setFillColor(fill);
        t.setOutlineColor(sf::Color::Black);
        t.setOutlineThickness(size > 30 ? 3.f : 2.f);
        return t;
    };
    sf::Text hud = makeText("", 20, sf::Color::White);
    hud.setPosition({12.f, 8.f});
    // The title is the project's name, shouted (Lesson 41): with no
    // project file it falls back to the game's built-in name.
    std::string titleText = haveProject ? project.name : "Gold Rush";
    std::transform(titleText.begin(), titleText.end(), titleText.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    sf::Text gameTitle = makeText(titleText, 52, sf::Color(255, 215, 80));
    sf::Text titleSub = makeText("", 18, sf::Color::White);
    sf::Text titlePrompt = makeText("Press Enter to play", 22, sf::Color::White);
    sf::Text seedLine = makeText("", 16, sf::Color(180, 180, 180));
    sf::Text pausedTitle = makeText("Paused", 36, sf::Color::White);
    sf::Text pausedSub = makeText("P to resume - Esc to quit", 18, sf::Color::White);
    sf::Text banner = makeText("You collected all the gold!", 36, sf::Color(255, 215, 80));
    sf::Text winTime = makeText("", 22, sf::Color::White);
    sf::Text sub = makeText("Press R for a new world", 20, sf::Color::White);
    sf::RectangleShape dim;
    dim.setFillColor(sf::Color(8, 8, 18, 170));

    Screen screen = Screen::Title;
    Camera camera;
    sf::Clock clock;

    auto centreOn = [&](sf::Text& t, float y) {
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});
        t.setPosition({static_cast<float>(window.getSize().x) / 2.f, y});
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
                        if (keyPressed->code == Key::Enter)
                            screen = Screen::Playing;
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
                        {
                            game.newRun(std::random_device{}());
                            screen = Screen::Playing;
                        }
                        if (keyPressed->code == Key::Escape)
                            window.close();
                        break;
                }
            }
        }

        if (screen == Screen::Playing)
        {
            game.update(dt);
            if (game.won())
                screen = Screen::Won;
        }

        // The camera follows the player, clamped so the view never
        // leaves the world; it is re-derived every frame from the
        // window's current size, so resizes need no event handling.
        camera.follow(game.playerCenter(), game.playerVelocity(), dt,
                      game.worldSize(), sf::Vector2f(window.getSize()));
        sf::View worldView = camera.makeView(sf::Vector2f(window.getSize()));
        worldView.move(game.shakeOffset()); // camera feel (Lesson 51)
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
                std::snprintf(buf, sizeof(buf), "Gold: %d / %d    Time: %d:%02d",
                              game.goldCollected(), game.goldTotal(),
                              static_cast<int>(game.runTime()) / 60,
                              static_cast<int>(game.runTime()) % 60);
                hud.setString(buf);
                window.draw(hud);
            }

            if (screen == Screen::Title)
            {
                dim.setSize({w, h});
                window.draw(dim);
                centreOn(gameTitle, h / 2.f - 80.f);
                window.draw(gameTitle);
                std::snprintf(buf, sizeof(buf),
                              "Collect all %d gold dots - mind the water, it slows you down",
                              game.goldTotal());
                titleSub.setString(buf);
                centreOn(titleSub, h / 2.f - 25.f);
                window.draw(titleSub);
                centreOn(titlePrompt, h / 2.f + 25.f);
                window.draw(titlePrompt);
                std::snprintf(buf, sizeof(buf), "World seed: %u",
                              static_cast<unsigned int>(game.seed()));
                seedLine.setString(buf);
                centreOn(seedLine, h / 2.f + 70.f);
                window.draw(seedLine);
            }
            else if (screen == Screen::Paused)
            {
                dim.setSize({w, h});
                window.draw(dim);
                centreOn(pausedTitle, h / 2.f - 20.f);
                window.draw(pausedTitle);
                centreOn(pausedSub, h / 2.f + 25.f);
                window.draw(pausedSub);
            }
            else if (screen == Screen::Won)
            {
                dim.setSize({w, h});
                window.draw(dim);
                centreOn(banner, h / 2.f - 45.f);
                window.draw(banner);
                std::snprintf(buf, sizeof(buf), "Time: %d:%02d",
                              static_cast<int>(game.runTime()) / 60,
                              static_cast<int>(game.runTime()) % 60);
                winTime.setString(buf);
                centreOn(winTime, h / 2.f + 5.f);
                window.draw(winTime);
                centreOn(sub, h / 2.f + 50.f);
                window.draw(sub);
            }
        }
        window.display();
    }
    return 0;
}
