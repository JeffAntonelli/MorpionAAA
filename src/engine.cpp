#include <engine.h>
#include "imgui.h"
#include "imgui-SFML.h"

#include <SFML/Window/Event.hpp>

Engine::Engine() : window_(sf::VideoMode(640,480), "Turn Based")
{
}


Engine::Engine(sf::Vector2i windowSize) : window_(sf::VideoMode(windowSize.x, windowSize.y), "Turn Based")
{

}


void Engine::AddSystem(System* system)
{
    systems_.push_back(system);
}

void Engine::AddDrawSystem(DrawInterface* drawSystem)
{
    drawSystems_.push_back(drawSystem);
}

void Engine::AddDrawImGuiSystem(DrawImGuiInterface* drawImGuiSystem)
{
    drawImGuiSystems_.push_back(drawImGuiSystem);
}

void Engine::AddOnEventInterface(OnEventInterface* onEventInterface)
{
    onEventInterfaces_.push_back(onEventInterface);
}

void Engine::Run()
{
    Init();
    sf::Clock clock;
    while(window_.isOpen())
    {
        const auto dt = clock.restart();
        Update(dt);
    }
    Destroy();
}

void Engine::Init()
{
    for(auto* system: systems_)
    {
        system->Init();
    }
    window_.setVerticalSyncEnabled(true);
    ImGui::SFML::Init(window_);
}

void Engine::Update(sf::Time dt)
{
    sf::Event event{};
    while (window_.pollEvent(event))
    {
        ImGui::SFML::ProcessEvent(event);
        switch (event.type)
        {
        case sf::Event::Closed:
            window_.close();
            return;
        default:
            break;
        }
        for(auto& onEventInterface : onEventInterfaces_)
        {
            onEventInterface->OnEvent(event);
        }
    }
    window_.clear(sf::Color::Black);
    for (auto* system : systems_)
    {
        system->Update();
    }
    for(auto* drawSystem : drawSystems_)
    {
        drawSystem->Draw(window_);
    }
    ImGui::SFML::Update(window_, dt);

    for(auto* system : drawImGuiSystems_)
    {
        system->DrawImGui();
    }
    ImGui::SFML::Render(window_);
    window_.display();
}

void Engine::Destroy()
{
    for (auto* system : systems_)
    {
        system->Destroy();
    }
    ImGui::SFML::Shutdown();
}
