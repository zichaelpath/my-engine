#include <SFML/Graphics.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>

int main()
{
    sf::RenderWindow window(sf::VideoMode({800, 600}), "MyEngine - Lesson 1");
    window.setFramerateLimit(60);

    sf::Clock clock;
    int frames = 0;
    float fpsTimer = 0.f;

    sf::RectangleShape player(sf::Vector2f(50.f, 50.f));
    player.setFillColor(sf::Color(80, 200, 120));
    player.setPosition({375.f, 275.f});

    constexpr float speed = 200.f; // pixels per second

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
        }

        // Movement: polled — "is this key held down right now?"
        sf::Vector2f dir(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            dir.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            dir.y += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            dir.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            dir.x += 1.f;

        // Normalise: a diagonal (1,1) has length sqrt(2) ≈ 1.41, so divide
        // by the length to bring the direction back to distance 1
        const float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len > 0.f)
        {
            dir.x /= len;
            dir.y /= len;
        }

        sf::Vector2f pos = player.getPosition() + dir * speed * dt;

        // Clamp inside the window (window size − shape size)
        pos.x = std::clamp(pos.x, 0.f, 750.f);
        pos.y = std::clamp(pos.y, 0.f, 550.f);
        player.setPosition(pos);

        window.clear(sf::Color(30, 30, 40));
        window.draw(player);
        window.display();
    }

    return 0;
}
