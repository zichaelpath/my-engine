#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include "Camera.hpp"
#include "Config.hpp"
#include "Game.hpp"

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
        window.display();
    }
    return 0;
}