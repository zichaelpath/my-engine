#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include "Camera.hpp"
#include "Config.hpp"
#include "Game.hpp"
#include "Log.hpp"
#include <cstdio>
#include <random>

int main()
{
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
        "MyEngine Runtime");
    window.setFramerateLimit(60);

    Game game;
    if (static_cast<std::uint32_t>(config.getInt("seed", 12345)) != game.seed())
        game.setSeed(static_cast<std::uint32_t>(config.getInt("seed", 12345)));
    if (config.getFloat("noiseScale", 0.01f) != game.noiseFrequency())
        game.setNoiseFrequency(config.getFloat("noiseScale", 0.01f));
    game.speed = config.getFloat("playerSpeed", 200.f);
    if (config.getInt("music", 1))
        game.startMusic("assets/music.wav");

    // HUD text (Lesson 32): one bundled font, loaded once. If it is
    // missing the game still plays — only the overlay is disabled.
    sf::Font font;
    bool fontOk = font.openFromFile("assets/DejaVuSans.ttf");
    if (!fontOk)
        Log::error("Font failed to load: assets/DejaVuSans.ttf (HUD disabled)");
    sf::Text hud(font, "", 20);
    hud.setPosition({12.f, 8.f});
    hud.setFillColor(sf::Color::White);
    hud.setOutlineColor(sf::Color::Black);
    hud.setOutlineThickness(2.f);
    sf::Text banner(font, "You collected all the gold!", 36);
    banner.setFillColor(sf::Color(255, 215, 80));
    banner.setOutlineColor(sf::Color::Black);
    banner.setOutlineThickness(3.f);
    sf::Text sub(font, "Press R for a new world", 20);
    sub.setFillColor(sf::Color::White);
    sub.setOutlineColor(sf::Color::Black);
    sub.setOutlineThickness(2.f);

    Camera camera;
    sf::Clock clock;

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
                if (keyPressed->code == sf::Keyboard::Key::Escape)
                {
                    window.close();
                }
                if (keyPressed->code == sf::Keyboard::Key::F5)
                {
                    game.saveScene("scene.txt");
                }
                if (keyPressed->code == sf::Keyboard::Key::F9)
                {
                    game.loadScene("scene.txt");
                }
                if (keyPressed->code == sf::Keyboard::Key::R && game.won())
                {
                    game.newRun(std::random_device{}());
                }
            }
        }
        game.update(dt);
        // The camera follows the player, clamped so the view never
        // leaves the world; it is re-derived every frame from the
        // window's current size, so resizes need no event handling.
        camera.follow(game.playerCenter(), game.worldSize(),
                      sf::Vector2f(window.getSize()));
        window.setView(camera.makeView(sf::Vector2f(window.getSize())));
        game.render(window);
        if (fontOk)
        {
            // HUD in screen space: the default view, drawn after the
            // world, so it never moves with the camera.
            window.setView(window.getDefaultView());
            char buf[96];
            std::snprintf(buf, sizeof(buf), "Gold: %d / %d    Time: %d:%02d",
                          game.goldCollected(), game.goldTotal(),
                          static_cast<int>(game.runTime()) / 60,
                          static_cast<int>(game.runTime()) % 60);
            hud.setString(buf);
            window.draw(hud);
            if (game.won())
            {
                auto centreOn = [&](sf::Text& t, float y) {
                    sf::FloatRect b = t.getLocalBounds();
                    t.setOrigin({b.position.x + b.size.x / 2.f,
                                 b.position.y + b.size.y / 2.f});
                    t.setPosition(
                        {static_cast<float>(window.getSize().x) / 2.f, y});
                };
                centreOn(banner, static_cast<float>(window.getSize().y) / 2.f - 30.f);
                centreOn(sub, static_cast<float>(window.getSize().y) / 2.f + 20.f);
                window.draw(banner);
                window.draw(sub);
            }
        }
        window.display();
    }
    return 0;
}