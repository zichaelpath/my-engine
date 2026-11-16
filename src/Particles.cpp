#include "Particles.hpp"

#include <algorithm>
#include <cmath>

void ParticleSystem::spawn(const Emitter& emitter, Random& rng)
{
    for (int i = 0; i < emitter.count; ++i)
    {
        if (m_particles.size() >= kCapacity)
            return; // a full pool clips the burst, never grows unbounded
        const float angle =
            emitter.angleRad +
            rng.rangeFloat("fx", -emitter.spreadRad / 2.f, emitter.spreadRad / 2.f);
        const float speed = rng.rangeFloat("fx", emitter.speedMin, emitter.speedMax);
        Particle p;
        p.position = emitter.position;
        p.velocity = {std::cos(angle) * speed, std::sin(angle) * speed};
        p.maxLife = rng.rangeFloat("fx", emitter.lifeMin, emitter.lifeMax);
        p.life = p.maxLife;
        p.size = rng.rangeFloat("fx", emitter.sizeMin, emitter.sizeMax);
        p.gravity = emitter.gravity;
        p.color = emitter.color;
        m_particles.push_back(p);
    }
}

void ParticleSystem::update(float dt)
{
    for (std::size_t i = m_particles.size(); i-- > 0;)
    {
        Particle& p = m_particles[i];
        p.life -= dt;
        if (p.life <= 0.f)
        {
            m_particles[i] = m_particles.back(); // swap-with-last, like the dots
            m_particles.pop_back();
            continue;
        }
        p.velocity.y += p.gravity * dt;
        p.position += p.velocity * dt;
    }
}

void ParticleSystem::draw(sf::RenderTarget& target) const
{
    if (m_particles.empty())
        return;
    sf::VertexArray quads(sf::PrimitiveType::Triangles, m_particles.size() * 6);
    std::size_t v = 0;
    for (const Particle& p : m_particles)
    {
        // Fade out over the particle's life: born solid, dies clear.
        const float fade = std::clamp(p.life / p.maxLife, 0.f, 1.f);
        sf::Color c = p.color;
        c.a = static_cast<std::uint8_t>(255.f * fade);
        const float h = p.size / 2.f;
        const sf::Vector2f corners[4] = {
            p.position + sf::Vector2f(-h, -h), p.position + sf::Vector2f(h, -h),
            p.position + sf::Vector2f(h, h),   p.position + sf::Vector2f(-h, h)};
        const int idx[6] = {0, 1, 2, 0, 2, 3};
        for (int k = 0; k < 6; ++k)
        {
            quads[v].position = corners[idx[k]];
            quads[v].color = c;
            ++v;
        }
    }
    target.draw(quads);
}
