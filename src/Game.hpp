#pragma once

#include <SFML/Graphics.hpp>
#include "Noise.hpp"
#include "Random.hpp"
#include <vector>

class Game
{
public:
    Game();

    void update(float dt, sf::Vector2f worldSize);
    void render(sf::RenderTarget& target) const;
    void setSeed(std::uint32_t seed);
    void generateTerrain();
    void generateDecor();
    void generateField();
    void setNoiseFrequency(float frequency);
    std::uint32_t seed() const { return m_seed; }
    sf::Vector2f playerPosition() const;

    float speed = 200.f;
    float clearColor[3] = {30.f / 255.f, 30.f / 255.f, 40.f / 255.f};
    float time = 0.f;

private:
    // Declaration order is initialisation order: m_seed must be declared
    // before m_random, whose constructor reads it.
    std::uint32_t m_seed = 12345;
    Random m_random;
    ValueNoise m_noise{0};
    float m_noiseFrequency = 0.01f;

    sf::RectangleShape m_player;
    std::vector<sf::RectangleShape> m_blocks;
    std::vector<sf::CircleShape> m_dots;
    sf::VertexArray m_field;
};
