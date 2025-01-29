#pragma once
#include "../include/ConnectionHandler.h"
#include <string>
#include <map>

class StompProtocol {
private:
    int subscriptionCounter;  // Tracks subscription IDs
    int receiptCounter;       // Tracks receipt IDs
    bool isLoggedIn;          // Tracks login state
    std::string username;     // Stores username
    std::map<std::string, int> subscriptions;  // Maps subscription IDs to topics

    // Generate a unique subscription ID
    int generateSubscriptionId();

    // Generate a unique receipt ID
    int generateReceiptId();

public:
    StompProtocol();

    // Login and logout methods
    std::string login(const std::string& host, const std::string& username, const std::string& password);
    std::string logout();

    // Subscription management
    std::string subscribe(const std::string& destination);
    std::string unsubscribe(const std::string& destination);

    // Send messages
    std::string send(const std::string& destination, const std::string& message);

    // Process server responses
    void processServerFrame(const std::string& frame);

    // Helper methods
    bool loggedIn() const;
};
