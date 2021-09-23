

#include "echo_client.h"
#include "engine.h"

int main()
{
    echo::EchoClient client;
    Engine engine;
    engine.AddSystem(&client);
    engine.AddDrawImGuiSystem(&client);
    engine.Run();
    return 0;
}
