#include <SFML/Graphics.hpp>
#include "Game.hpp"

int main()
{
    sf::RenderWindow window(sf::VideoMode({800, 600}), "MyEngine Runtime");
    window.setFramerateLimit(60);

    Game game;
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