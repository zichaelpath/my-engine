#include "Game.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

Game::Game()
{
    m_player.setSize({50.f, 50.f});
    m_player.setFillColor(sf::Color(80, 200, 120));
    m_player.setPosition({375.f, 275.f});
}

void Game::update(float dt, sf::Vector2f worldSize)
{
    time += dt;

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

    const float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len > 0.f)
    {
        dir.x /= len;
        dir.y /= len;
    }

    sf::Vector2f pos = m_player.getPosition() + dir * speed * dt;
    pos.x = std::clamp(pos.x, 0.f, std::max(0.f, worldSize.x - 50.f));
    pos.y = std::clamp(pos.y, 0.f, std::max(0.f, worldSize.y - 50.f));
    m_player.setPosition(pos);
}

void Game::render(sf::RenderTarget& target) const
{
    sf::Color clearCol(
        static_cast<std::uint8_t>(clearColor[0] * 255.f),
        static_cast<std::uint8_t>(clearColor[1] * 255.f),
        static_cast<std::uint8_t>(clearColor[2] * 255.f));

    target.clear(clearCol);
    target.draw(m_player);
}

sf::Vector2f Game::playerPosition() const
{
    return m_player.getPosition();
}
