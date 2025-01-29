#include <stdlib.h>
#include "../include/ConnectionHandler.h"
#include "StompProtocol.h"
#include "util.h"
#include "event.h"
#include <queue>

bool shouldTerminate = false;
std::queue<std::string> clientFramesQueue;
std::queue<std::string> serverFramesQueue;

std::mutex terminateMutex;
std::mutex clientQueueMutex;
std::mutex serverQueueMutex;

bool isTerminated() {
    std::lock_guard<std::mutex> lock(terminateMutex);
    return shouldTerminate;
}

void setTerminated(bool value) {
    std::lock_guard<std::mutex> lock(terminateMutex);
    shouldTerminate = value;
}

void socketThread(ConnectionHandler &connectionHandler) {
    while (!isTerminated()) {
        std::string frameToSend;
        {
            std::lock_guard<std::mutex> lock(clientQueueMutex);
            if (!clientFramesQueue.empty()) {
                frameToSend = clientFramesQueue.front();
                clientFramesQueue.pop();
            }
        }

        if (!frameToSend.empty()) {
            bool sent = connectionHandler.sendFrameAscii(frameToSend, '\0');
            if (!sent) {
                std::cerr << "Sending frame failed. Marking terminate.\n";
                setTerminated(true);
                break;
            }

            std::string serverFrame;
            bool gotData = connectionHandler.getFrameAscii(serverFrame, '\0');
            if (gotData) {
                std::cout << "[Server -> Client] " << serverFrame << std::endl;
                std::lock_guard<std::mutex> lock(serverQueueMutex);
                serverFramesQueue.push(serverFrame);
            }
            else {
                // If false => no data or real disconnect
                // In a real client, you might differentiate between "no data yet"
                // and "socket disconnected."
            }
        }
    }
    std::cerr << "[Network Thread] Exiting.\n";
}

void cliThreadFunc(StompProtocol &protocolHandler, std::string &currentUser) {
    while (!isTerminated()) {
        std::string command;
        std::getline(std::cin, command);

        if (startsWith(command, "login ")) {
            std::cout << command << "\n";
            std::string delimiter = " ";
            command.erase(0, command.find(delimiter) + delimiter.length());
            std::string host = command.substr(0, command.find(delimiter));
            command.erase(0, command.find(delimiter) + delimiter.length());
            std::string username = command.substr(0, command.find(delimiter));
            command.erase(0, command.find(delimiter) + delimiter.length());
            std::string password = command.substr(0, command.find(delimiter));
            command.erase(0, command.find(delimiter) + delimiter.length());
            std::cout << host << "\n";

            std::string frame = protocolHandler.login(host, username, password);
            std::cout << frame << "\n";
            currentUser = username;
            {
                std::lock_guard<std::mutex> lock(clientQueueMutex);
                clientFramesQueue.push(frame);
            }
        }
        else if (startsWith(command, "logout")) {
            std::string frame = protocolHandler.logout();
            {
                std::lock_guard<std::mutex> lock(clientQueueMutex);
                clientFramesQueue.push(frame);
            }
            setTerminated(true);
        } else if (startsWith(command, "join ")) {
            std::string dest = command.substr(5);
            std::string frame = protocolHandler.subscribe(dest);
            std::cout << frame << "\n";
            {
                std::lock_guard<std::mutex> lock(clientQueueMutex);
                clientFramesQueue.push(frame);
            }

        } else if (startsWith(command, "exit ")) {
            std::string dest = command.substr(5);
            std::string frame = protocolHandler.unsubscribe(dest);
            {
                std::lock_guard<std::mutex> lock(clientQueueMutex);
                clientFramesQueue.push(frame);
            }
        } else if (startsWith(command, "report ")) {
            std::string jsonPath = command.substr(7);
            names_and_events parsedFile = parseEventsFile(jsonPath);
            for (Event &event : parsedFile.events) {
                std::string general_information = "";
                for (auto &info : event.get_general_information()) {
                    general_information += "\t" + info.first + ": " + info.second + "\n";
                }
                std::string message = "user: meni\ncity: " + event.get_city() + "\nevent name: " + event.get_name() + "\ndate time: " + std::to_string(event.get_date_time()) + "\ngeneral information:\n" + general_information + "description:\n" + event.get_description();
                std::string frame = protocolHandler.send(parsedFile.channel_name, message);
                {
                    std::lock_guard<std::mutex> lock(clientQueueMutex);
                    clientFramesQueue.push(frame);
                }
            }
        } else if (startsWith(command, "summary ")) {
            // ...
        } else {
            std::cout << "Unknown command.\n";
        }

        {
            std::lock_guard<std::mutex> lock(serverQueueMutex);
            while (!serverFramesQueue.empty()) {
                std::string frame = serverFramesQueue.front();
                serverFramesQueue.pop();
                protocolHandler.processServerFrame(frame);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <host> <port>" << std::endl;
        return 1;
    }

    std::string host = argv[1];
    short port = std::stoi(argv[2]);
    std::string user = "";
	ConnectionHandler connectionHandler(host, port);
    StompProtocol protocolHandler;

    if (!connectionHandler.connect()) {
        return 1;
    }

    std::thread socket(socketThread, std::ref(connectionHandler));
    cliThreadFunc(protocolHandler, user);

    setTerminated(true);
    socket.join();

    return 0;
}