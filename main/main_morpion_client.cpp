

#include "engine.h"
#include "morpion_client.h"

int main()
{
    morpion::MorpionClient client;
    morpion::MorpionView view(client);
    Engine engine(sf::Vector2i(480,480));
    engine.AddDrawImGuiSystem(&view);
    engine.AddDrawSystem(&view);
    engine.AddOnEventInterface(&view);
    engine.AddSystem(&client);
    engine.Run();
    return 0;
}
