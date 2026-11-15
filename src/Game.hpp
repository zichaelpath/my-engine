#pragma once

#include <SFML/Graphics.hpp>

class Game
{
public:
    Game();

    void update(float dt, sf::Vector2f worldSize);
    void render(sf::RenderTarget& target) const;
    sf::Vector2f playerPosition() const;

    float speed = 200.f;
    float clearColor[3] = {30.f / 255.f, 30.f / 255.f, 40.f / 255.f};
    float time = 0.f;

private:
    sf::RectangleShape m_player;
};
