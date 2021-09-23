#include <echo_server.h>
#include <echo_settings.h>
#include <iostream>
#include <array>

namespace echo
{
int EchoServer::Run()
{
    listener_.setBlocking(false);
    if(listener_.listen(serverPortNumber) != sf::Socket::Done)
    {
        std::cerr << "[Error] Server cannot bind port: " << serverPortNumber << '\n';
        return SERVER_BINDING_ERROR;
    }
    std::cout << "Server bound to port " << serverPortNumber << '\n';
    while (true)
    {
        // accept a new connection
        const auto nextIndex = GetNextSocket();

        if (nextIndex != -1)
        {
            auto& newSocket = sockets_[nextIndex];
            if (listener_.accept(newSocket) == sf::Socket::Done)
            {
                newSocket.setBlocking(false);
                selector_.add(newSocket);
            }
        }
        // receive new messages
        if(selector_.wait(sf::milliseconds(20)))
        {
            for(auto& socket: sockets_)
            {
                if(selector_.isReady(socket))
                {
                    std::array<char, maxDataSize> receivedMsg{};
                    std::size_t received = 0;
                    sf::Socket::Status receivingStatus;
                    do
                    {
                        receivingStatus = socket.receive(receivedMsg.data(), maxDataSize, received);
                    } while (receivingStatus == sf::Socket::Partial);

                    if(receivingStatus == sf::Socket::Done)
                    {
                        std::cout << "Received a msg from " << socket.getRemoteAddress().toString() << ":"
                        << socket.getRemotePort() << "\n" << receivedMsg.data() << '\n';

                        //Resending the msg
                        std::size_t sent = 0;
                        sf::Socket::Status sentStatus;

                        do
                        {
                            sentStatus = socket.send(receivedMsg.data(), maxDataSize, sent);
                        } while (sentStatus == sf::Socket::Partial);

                        if(sentStatus == sf::Socket::Done)
                        {
                            std::cout << "Successfully resent the msg to the client\n";
                        }
                    }
                }
            }
        }
    }
    return SERVER_FINISH_OK;
}

int EchoServer::GetNextSocket()
{
    for(int i = 0; i < maxClientNmb; i++)
    {
        if(sockets_[i].getLocalPort() == 0)
        {
            return i;
        }
    }
    return -1;
}
}
