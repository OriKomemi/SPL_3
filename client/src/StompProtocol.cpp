#include "StompProtocol.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "util.h"

StompProtocol::StompProtocol() : subscriptionCounter(0), receiptCounter(0), isLoggedIn(false), username("") {}

// Generate a unique subscription ID
int StompProtocol::generateSubscriptionId() {
    return ++subscriptionCounter;
}

// Generate a unique receipt ID
int StompProtocol::generateReceiptId() {
    return ++receiptCounter;
}

// Login to the server
std::string StompProtocol::login(const std::string& host, const std::string& username, const std::string& password) {
    this->username = username;
    isLoggedIn = true;  // Assume successful login for now
    return "CONNECT\naccept-version:1.2\nhost:" + host + "\nlogin:" + username + "\npasscode:" + password + "\n\n";
}

// Logout from the server
std::string StompProtocol::logout() {
    if (!isLoggedIn) {
        return "The client is not logged in.";
    }
    isLoggedIn = false;
    username = "";
    return "DISCONNECT\nreceipt:" + std::to_string(generateReceiptId()) + "\n\n";
}

// Subscribe to a topic
std::string StompProtocol::subscribe(const std::string& destination) {
    int id = generateSubscriptionId();
    int receiptId = generateReceiptId();
    subscriptions[destination] = id;
    return "SUBSCRIBE\ndestination:/" + destination + "\nid:" + std::to_string(id) + "\nreceipt:" + std::to_string(receiptId) + "\n\n";
}

// Unsubscribe from a topic
std::string StompProtocol::unsubscribe(const std::string& destination) {
    if (subscriptions.find(destination) != subscriptions.end()) {
        int receiptId = generateReceiptId();
        int id = subscriptions[destination];
        subscriptions.erase(destination);
        return "UNSUBSCRIBE\nid:" + std::to_string(id) + "\nreceipt:" + std::to_string(receiptId) + "\n\n";
    }
    return "Error: Subscription ID not found.";
}

// Send a message to a topic
std::string StompProtocol::send(const std::string& destination, const std::string& message) {
    int receiptId = generateReceiptId();
    return "SEND\ndestination:/" + destination + "\nreceipt:" + std::to_string(receiptId) + "\n\n" + message + "";
}

// Process server responses
void StompProtocol::processServerFrame(const std::string& frame) {
    if (startsWith(frame, "CONNECTED")) {
        std::cout << "Login successful." << std::endl;
    } else if (startsWith(frame, "RECEIPT")) {
        std::cout << "Receipt received: " << frame << std::endl;
    } else if (startsWith(frame, "ERROR")) {
        std::cout << "Error from server: " << frame << std::endl;
    } else {
        std::cout << "Unhandled server frame: " << frame << std::endl;
    }
}

// Check if the client is logged in
bool StompProtocol::loggedIn() const {
    return isLoggedIn;
}
