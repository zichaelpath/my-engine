#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <iostream>
#include "Game.hpp"
#include <random>

int main()
{
    sf::RenderWindow window(sf::VideoMode({1280, 720}), "MyEngine - Lesson 1");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    Game game;

    sf::Clock clock;
    int frames = 0;
    float fpsTimer = 0.f;

    sf::RenderTexture gameView(sf::Vector2u(800, 600));
    sf::Vector2f panelSize{800.f, 600.f};

    bool playing = true;

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
            ImGui::SFML::ProcessEvent(window, *event);
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
                if (keyPressed->code == sf::Keyboard::Key::Space)
                {
                    playing = !playing;
                }
            }
        }
        

        ImGui::SFML::Update(window, sf::seconds(dt));

        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

        bool stepClicked = false;
        ImGui::Begin("Engine Debug");
        ImGui::Text("FPS: %.1f", 1.0f / dt);
        ImGui::Text("Frame time: %.3f ms", dt * 1000.f);
        sf::Vector2f p = game.playerPosition();
        ImGui::Text("Player: (%.0f, %.0f)", p.x, p.y);
        ImGui::Text("Game time: %.2f s", game.time);
        if (ImGui::Button(playing ? "Pause" : "Play"))
        {
            playing = !playing;
        }
        ImGui::SameLine();
        if (ImGui::Button("Step"))
        {
            stepClicked = true;
        }
        ImGui::ColorEdit3("Clear color", game.clearColor);
        ImGui::SliderFloat("Speed", &game.speed, 50.f, 600.f);
        ImGui::End();

        ImGui::Begin("Procgen");
        ImGui::Text("Master seed: %u", game.seed());
        static int seedInput = 12345;
        ImGui::InputInt("Seed", &seedInput);
        if (ImGui::Button("Apply seed"))
            game.setSeed(static_cast<std::uint32_t>(seedInput));
        ImGui::SameLine();
        if (ImGui::Button("Random seed"))
        {
            std::random_device rd;
            seedInput = static_cast<int>(rd());
            game.setSeed(static_cast<std::uint32_t>(seedInput));
        }
        if (ImGui::Button("Reshuffle terrain"))
            game.generateTerrain();
        ImGui::SameLine();
        if (ImGui::Button("Reshuffle decor"))
            game.generateDecor();
        ImGui::End();


        if (playing || stepClicked)
            game.update(dt, sf::Vector2f(gameView.getSize()));

        sf::Vector2u wantSize(panelSize);
        if (wantSize.x > 0 && wantSize.y > 0 && wantSize != gameView.getSize())
        {
            gameView.resize(wantSize);
            gameView.setView(sf::View(sf::FloatRect({0.f, 0.f}, sf::Vector2f(wantSize))));
        }

        game.render(gameView);
        gameView.display();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
        ImGui::Begin("Game View");
        ImVec2 avail = ImGui::GetContentRegionAvail();
        if (avail.x > 0.f && avail.y > 0.f)
            panelSize = sf::Vector2(avail.x, avail.y);
        ImGui::Image(gameView, panelSize);
        ImGui::End();
        ImGui::PopStyleVar();

        window.clear(sf::Color(25, 25, 25));
        ImGui::SFML::Render(window);
        window.display();
    }
    ImGui::SFML::Shutdown();
    return 0;
}
