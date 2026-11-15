#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <iostream>
#include "Camera.hpp"
#include "Config.hpp"
#include "Game.hpp"
#include "Log.hpp"
#include <algorithm>
#include <random>

int main()
{
    Config config;
    config.setDefault("windowWidth", "1280");
    config.setDefault("windowHeight", "720");
    config.setDefault("seed", "12345");
    config.setDefault("noiseScale", "0.01");
    config.setDefault("playerSpeed", "200");
    if (!config.load("engine.ini"))
        config.save("engine.ini");

    sf::RenderWindow window(
        sf::VideoMode({static_cast<unsigned int>(config.getInt("windowWidth", 1280)),
                       static_cast<unsigned int>(config.getInt("windowHeight", 720))}),
        "MyEngine - Lesson 1");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    Game game;
    if (static_cast<std::uint32_t>(config.getInt("seed", 12345)) != game.seed())
        game.setSeed(static_cast<std::uint32_t>(config.getInt("seed", 12345)));
    if (config.getFloat("noiseScale", 0.01f) != game.noiseFrequency())
        game.setNoiseFrequency(config.getFloat("noiseScale", 0.01f));
    game.speed = config.getFloat("playerSpeed", 200.f);

    Camera camera;
    bool followCam = true;
    float zoom = 1.f;

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
        ImGui::Text("Player: (%.0f, %.0f) frame %d", p.x, p.y, game.playerFrame());
        ImGui::Text("Entities: %d", static_cast<int>(game.entities().size()));
        ImGui::Text("Camera: (%.0f, %.0f)", camera.center.x, camera.center.y);
        ImGui::Checkbox("Camera follow", &followCam);
        ImGui::SliderFloat("Zoom", &zoom, 0.4f, 3.f);
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
        static int seedInput = static_cast<int>(game.seed());
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
        static float noiseFreq = game.noiseFrequency();
        if (ImGui::SliderFloat("Noise scale", &noiseFreq, 0.004f, 0.03f))
            game.setNoiseFrequency(noiseFreq);
        if (ImGui::Button("Reshuffle terrain"))
            game.generateTerrain();
        ImGui::SameLine();
        if (ImGui::Button("Reshuffle decor"))
            game.generateDecor();
        ImGui::End();

        ImGui::Begin("Console");
        if (ImGui::Button("Clear"))
            Log::clear();
        ImGui::SameLine();
        static bool autoScroll = true;
        ImGui::Checkbox("Auto-scroll", &autoScroll);
        ImGui::BeginChild("ConsoleLog", ImVec2(0.f, 0.f), ImGuiChildFlags_Borders);
        for (const auto& entry : Log::entries())
        {
            ImVec4 color = entry.level == LogLevel::Warning
                               ? ImVec4(1.f, 0.8f, 0.3f, 1.f)
                               : entry.level == LogLevel::Error
                               ? ImVec4(1.f, 0.35f, 0.35f, 1.f)
                               : ImVec4(0.85f, 0.85f, 0.85f, 1.f);
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::TextUnformatted(entry.message.c_str());
            ImGui::PopStyleColor();
        }
        static std::size_t lastEntryCount = 0;
        if (autoScroll && Log::entries().size() != lastEntryCount)
            ImGui::SetScrollHereY(1.f);
        lastEntryCount = Log::entries().size();
        ImGui::EndChild();
        ImGui::End();


        if (playing || stepClicked)
            game.update(dt);

        sf::Vector2u wantSize(panelSize);
        if (wantSize.x > 0 && wantSize.y > 0 && wantSize != gameView.getSize())
            gameView.resize(wantSize);

        // The Game View's camera: follows the player unless told not
        // to; zoom sets the visible height; the view is rebuilt every
        // frame from the texture's current size.
        camera.viewHeight = 600.f / zoom;
        sf::Vector2f canvasSize(gameView.getSize());
        if (followCam)
            camera.follow(game.playerCenter(), game.worldSize(), canvasSize);
        else
            camera.clampToWorld(game.worldSize(), canvasSize);
        gameView.setView(camera.makeView(canvasSize));

        game.render(gameView);
        gameView.display();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
        ImGui::Begin("Game View");
        ImVec2 avail = ImGui::GetContentRegionAvail();
        if (avail.x > 0.f && avail.y > 0.f)
            panelSize = sf::Vector2(avail.x, avail.y);
        ImGui::Image(gameView, panelSize);
        if (ImGui::IsWindowHovered())
        {
            // Mouse wheel zooms; right-drag grabs the world and pans
            // (disable Camera follow to stay where you pan to).
            float wheel = ImGui::GetIO().MouseWheel;
            if (wheel != 0.f)
                zoom = std::clamp(zoom * (1.f + wheel * 0.1f), 0.4f, 3.f);
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
            {
                ImVec2 drag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
                sf::Vector2f view = camera.viewSize(panelSize);
                camera.center -= sf::Vector2f(drag.x * view.x / panelSize.x,
                                              drag.y * view.y / panelSize.y);
                ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();

        window.clear(sf::Color(25, 25, 25));
        ImGui::SFML::Render(window);
        window.display();
    }
    ImGui::SFML::Shutdown();
    return 0;
}
