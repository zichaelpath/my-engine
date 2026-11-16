// Particles (Lesson 50) — a CPU particle pool with emitters as
// data. One pool, one vertex array, no per-particle objects on the
// heap: the pool is a flat vector with swap-with-last removal, the
// same trick the gold dots have used since Lesson 32.
//
// An Emitter is a plain description — where, how many, how fast,
// which way, how long, how big, what colour, how much gravity —
// and spawn() rolls it against a Random stream. Cosmetic randomness
// draws from the "fx" stream only: particles must never perturb
// the world's generation streams (determinism is sacred, and the
// harness's grid checksum stands guard over exactly that).

#pragma once

#include "Random.hpp"

#include <SFML/Graphics.hpp>
#include <cstddef>
#include <vector>

struct Particle
{
    sf::Vector2f position;
    sf::Vector2f velocity;
    float life = 0.f;    // seconds remaining
    float maxLife = 1.f; // seconds at birth (the fade's denominator)
    float size = 2.f;    // quad edge, world units
    float gravity = 0.f; // downward acceleration, units/s^2
    sf::Color color = sf::Color::White;
};

struct Emitter
{
    sf::Vector2f position;
    int count = 8;
    sf::Color color = sf::Color::White;
    float speedMin = 40.f;
    float speedMax = 110.f;
    float angleRad = -1.5707963f; // straight up
    float spreadRad = 6.2831853f; // full circle by default
    float lifeMin = 0.35f;
    float lifeMax = 0.6f;
    float sizeMin = 2.f;
    float sizeMax = 3.5f;
    float gravity = 260.f;
};

class ParticleSystem
{
public:
    void spawn(const Emitter& emitter, Random& rng);
    void update(float dt);
    void draw(sf::RenderTarget& target) const;
    void clear() { m_particles.clear(); }

    std::size_t activeCount() const { return m_particles.size(); }
    const std::vector<Particle>& particles() const { return m_particles; }

private:
    std::vector<Particle> m_particles;
    static constexpr std::size_t kCapacity = 1024; // beyond this, spawns clip
};
