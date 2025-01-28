#include <stdlib.h>
#include "../include/ConnectionHandler.h"
#include "StompProtocol.h"
#include "util.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <host> <port>" << std::endl;
        return 1;
    }

    std::string host = argv[1];
    short port = std::stoi(argv[2]);

	ConnectionHandler handler(host, port);
    StompProtocol protocolHandler;

    while (true) {
        std::cout << "Enter command: ";
        std::string command;
        std::getline(std::cin, command);

        if (startsWith(command, "login ")) {
            std::istringstream iss(command.substr(6));
            std::string host, username, password;
            iss >> host >> username >> password;
            std::cout << protocolHandler.login(host, username, password) << std::endl;

        } else if (startsWith(command, "logout")) {
            std::cout << protocolHandler.logout() << std::endl;

        } else if (startsWith(command, "join ")) {
            std::string destination = command.substr(5);
            std::cout << protocolHandler.subscribe(destination) << std::endl;

        } else if (startsWith(command, "exit ")) {
            int id = std::stoi(command.substr(5));
            std::cout << protocolHandler.unsubscribe(id) << std::endl;

        } else if (startsWith(command, "send ")) {
            std::istringstream iss(command.substr(5));
            std::string destination, message;
            iss >> destination;
            std::getline(iss, message);
            message = message.substr(1); // Remove leading space
            std::cout << protocolHandler.send(destination, message) << std::endl;
        } else {
            std::cout << "Unknown command." << std::endl;
        }
    }
    if (!handler.connect()) {
        return 1;
    }
}