#pragma once

#include <vector>
#include <system.h>

#include <SFML/Graphics/RenderWindow.hpp>


class Engine
{
public:
    Engine();
    Engine(sf::Vector2i windowSize);
    void AddSystem(System* system);
    void AddDrawSystem(DrawInterface* drawSystem);
    void AddDrawImGuiSystem(DrawImGuiInterface* drawImGuiSystem);
    void AddOnEventInterface(OnEventInterface* onEventInterface);
    void Run();
private:
    void Init();
    void Update(sf::Time dt);
    void Destroy();

    std::vector<System*> systems_;
    std::vector<DrawInterface*> drawSystems_;
    std::vector<DrawImGuiInterface*> drawImGuiSystems_;
    std::vector<OnEventInterface*> onEventInterfaces_;
    sf::RenderWindow window_; 
};