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
    if (isLoggedIn) {
        return "The client is already logged in, log out before trying again.";
    }
    this->username = username;
    isLoggedIn = true;  // Assume successful login for now
    return "CONNECT\naccept-version:1.2\nhost:" + host + "\nlogin:" + username + "\npasscode:" + password + "\n\n\0";
}

// Logout from the server
std::string StompProtocol::logout() {
    if (!isLoggedIn) {
        return "The client is not logged in.";
    }
    isLoggedIn = false;
    username = "";
    return "DISCONNECT\nreceipt:" + std::to_string(generateReceiptId()) + "\n\n\0";
}

// Subscribe to a topic
std::string StompProtocol::subscribe(const std::string& destination) {
    int id = generateSubscriptionId();
    int receiptId = generateReceiptId();
    subscriptions[id] = destination;
    return "SUBSCRIBE\ndestination:/" + destination + "\nid:" + std::to_string(id) + "\nreceipt:" + std::to_string(receiptId) + "\n\n\0";
}

// Unsubscribe from a topic
std::string StompProtocol::unsubscribe(int id) {
    if (subscriptions.find(id) != subscriptions.end()) {
        int receiptId = generateReceiptId();
        subscriptions.erase(id);
        return "UNSUBSCRIBE\nid:" + std::to_string(id) + "\nreceipt:" + std::to_string(receiptId) + "\n\n\0";
    }
    return "Error: Subscription ID not found.";
}

// Send a message to a topic
std::string StompProtocol::send(const std::string& destination, const std::string& message) {
    int receiptId = generateReceiptId();
    return "SEND\ndestination:/" + destination + "\nreceipt:" + std::to_string(receiptId) + "\n\n" + message + "\0";
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
