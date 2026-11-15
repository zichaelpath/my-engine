#include <SFML/Graphics.hpp>
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
    if (!config.load("engine.ini"))
        config.save("engine.ini");

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
            }
            
            if (const auto* resized = event->getIf<sf::Event::Resized>())
            {
                window.setView(sf::View(sf::FloatRect({0.f, 0.f}, sf::Vector2f(resized->size))));
            }
        }
        game.update(dt, sf::Vector2f(window.getSize()));
        game.render(window);
        window.display();
    }
    return 0;
}