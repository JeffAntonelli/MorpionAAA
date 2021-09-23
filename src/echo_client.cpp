#include <array>
#include <echo_client.h>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <iostream>
#include <cstring>
#include <SFML/Network/IpAddress.hpp>

namespace echo
{
void EchoClient::Init()
{
}

void EchoClient::Update()
{
    if(socket_.getLocalPort() != 0)
    {
        std::array<char, maxDataSize> receivedMsg{};
        std::size_t received = 0;
        sf::Socket::Status status;
        do
        {
            status = socket_.receive(receivedMsg.data(), receivedMsg.size(), received);
        } while (status == sf::Socket::Partial);

        if(status == sf::Socket::Done)
        {
            receivedMsgs_.emplace_back(receivedMsg.data());
        }

        if(status == sf::Socket::Disconnected)
        {
            socket_.disconnect();
            std::cerr << "Server disconnected\n";
        }
    }
}

void EchoClient::Destroy()
{
}

void EchoClient::DrawImGui()
{
    ImGui::Begin("Client");
    if(socket_.getLocalPort() != 0)
    {
        ImGui::InputTextMultiline("Message", msg_.data(), msg_.size());
        if(ImGui::Button("Send"))
        {
            std::size_t sent = 0;
            sf::Socket::Status status;
            do
            {
                status = socket_.send(msg_.data(), msg_.size(), sent);
            } while (status == sf::Socket::Partial);

            if(status == sf::Socket::Done)
            {
                std::cout << "Message sent successfully\n";
                //Clear msg
                msg_[0] = 0;
            }
        }
        ImGui::BeginChild("log");
        for(auto& msg : receivedMsgs_)
        {
            ImGui::Text("%s", msg.data());
        }
        ImGui::EndChild();
    }
    else
    {
        ImGui::InputText("Ip Address", &ipAddressBuffer);
        ImGui::SameLine();
        ImGui::InputInt("Port Number", &portNumber);
        if(ImGui::Button("Connect"))
        {
            const auto status = socket_.connect(sf::IpAddress(ipAddressBuffer.data()), portNumber);
            socket_.setBlocking(false);
            if(status != sf::Socket::Done)
            {
                switch (status)
                {
                case sf::Socket::NotReady: 
                    std::cerr << "Not ready to connect to " << ipAddressBuffer.data() << ':' << portNumber << '\n';
                    break;
                case sf::Socket::Partial: 
                    std::cerr << "Connecting to " << ipAddressBuffer.data() << ':' << portNumber << '\n';
                    break;
                case sf::Socket::Disconnected: 
                    std::cerr << "Disconnecting to " << ipAddressBuffer.data() << ':' << portNumber << '\n';
                    break;
                case sf::Socket::Error: 
                    std::cerr << "Error connecting to " << ipAddressBuffer.data() << ':' << portNumber << '\n';
                    break;
                default: ;
                }
            }
            else
            {
                std::cout << "Successfully connected to server\n";
            }
        }
    }
    ImGui::End();
}
}
