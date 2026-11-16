#pragma once

#include <SFML/Graphics.hpp>

#include <algorithm>
#include <cmath>

// A viewpoint into the world: a centre (in world units) and a visible
// height (in world units). The visible width follows the canvas aspect
// ratio, so the picture is never distorted. Zoom is nothing more than
// the visible height changing — a smaller height is a closer camera.
// Hosts own cameras (the editor's and the runtime's differ); the game
// owns the world they look at.
struct Camera
{
    sf::Vector2f center{400.f, 300.f};
    float viewHeight = 600.f;

    // The world-unit rectangle this camera shows on a canvas of the
    // given pixel size.
    sf::Vector2f viewSize(sf::Vector2f canvasSize) const
    {
        float aspect = canvasSize.y > 0.f ? canvasSize.x / canvasSize.y : 1.f;
        return {viewHeight * aspect, viewHeight};
    }

    sf::View makeView(sf::Vector2f canvasSize) const
    {
        sf::View view;
        view.setCenter(center);
        view.setSize(viewSize(canvasSize));
        return view;
    }

    // Keeps the whole view inside the world: the centre may not come
    // within half a view of any edge. On an axis where the view is
    // larger than the world, the camera centres on the world instead.
    void clampToWorld(sf::Vector2f worldSize, sf::Vector2f canvasSize)
    {
        sf::Vector2f half = viewSize(canvasSize) / 2.f;
        center.x = worldSize.x <= half.x * 2.f
                       ? worldSize.x / 2.f
                       : std::clamp(center.x, half.x, worldSize.x - half.x);
        center.y = worldSize.y <= half.y * 2.f
                       ? worldSize.y / 2.f
                       : std::clamp(center.y, half.y, worldSize.y - half.y);
    }

    // Smooth follow with lookahead (Lesson 51): the camera glides
    // toward a point slightly ahead of the target — led by its
    // velocity, capped so a sprint never loses it — instead of
    // snapping. The approach is exponential in dt, so the same
    // fraction of the remaining distance closes per second at any
    // frame rate.
    void follow(sf::Vector2f target, sf::Vector2f velocity, float dt,
                sf::Vector2f worldSize, sf::Vector2f canvasSize)
    {
        sf::Vector2f lead = velocity * 0.18f;
        const float leadLen = std::sqrt(lead.x * lead.x + lead.y * lead.y);
        if (leadLen > 36.f)
            lead *= 36.f / leadLen;
        const sf::Vector2f desired = target + lead;
        const float k = 1.f - std::exp(-8.f * dt);
        center += (desired - center) * k;
        clampToWorld(worldSize, canvasSize);
    }
};
