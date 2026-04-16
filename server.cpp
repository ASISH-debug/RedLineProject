#include "crow_all.h"
#include <iostream>

int main() {
    crow::SimpleApp app;

    // Home route
    CROW_ROUTE(app, "/")([](){
        return "🚍 Redline Auto Booking Backend (Crow C++) running!";
    });

    // Example POST route for bookings
    CROW_ROUTE(app, "/api/bookings").methods("POST"_method)
    ([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body)
            return crow::response(400, "Invalid JSON");

        std::string source = body["source"].s();
        std::string destination = body["destination"].s();

        crow::json::wvalue res;
        res["message"] = "✅ Booking received";
        res["source"] = source;
        res["destination"] = destination;
        return crow::response(res);
    });

    std::cout << "✅ Crow C++ server running on http://localhost:5050\n";
    app.port(5050).multithreaded().run();
}