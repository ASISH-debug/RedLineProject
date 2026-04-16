#include "httplib.h"
#include "json.hpp"
#include <fstream>
#include <mutex>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <filesystem>

using json = nlohmann::json;
using namespace httplib;
std::mutex mu;

const std::string BUSES_FILE = "buses.json";
const std::string BOOKINGS_FILE = "bookings.json";

struct Seat {
    int number;
    bool booked;
};

struct Bus {
    int id;
    std::string name;
    int fare;
    int seatCount;
    std::vector<Seat> seats;
};

json readJsonFile(const std::string &path) {
    if (!std::filesystem::exists(path)) return json::array();
    std::ifstream in(path);
    json j;
    in >> j;
    return j;
}

void writeJsonFile(const std::string &path, const json &j) {
    std::ofstream out(path);
    out << j.dump(2);
}

void ensureSampleData() {
    std::lock_guard<std::mutex> lock(mu);
    if (!std::filesystem::exists(BUSES_FILE)) {
        json buses = json::array();
        buses.push_back({
            {"id", 1},
            {"name", "RedLine Express"},
            {"fare", 650},
            {"seatCount", 30},
            {"seats", json::array()}
        });
        buses.push_back({
            {"id", 2},
            {"name", "Metro Deluxe"},
            {"fare", 590},
            {"seatCount", 24},
            {"seats", json::array()}
        });
        for (auto &b: buses) {
            int sc = b["seatCount"].get<int>();
            for (int i=1;i<=sc;i++) b["seats"].push_back({{"number", i}, {"booked", false}});
        }
        writeJsonFile(BUSES_FILE, buses);
    }
    if (!std::filesystem::exists(BOOKINGS_FILE)) {
        writeJsonFile(BOOKINGS_FILE, json::array());
    }
}

int main() {
    ensureSampleData();
    Server svr;

    auto set_cors = [](Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
    };

    svr.Options(".*", [&](const Request &req, Response &res) {
        set_cors(res);
        res.status = 200;
    });

    svr.Get("/api/buses", [&](const Request &req, Response &res) {
        std::lock_guard<std::mutex> lock(mu);
        json buses = readJsonFile(BUSES_FILE);
        set_cors(res);
        res.set_content(buses.dump(), "application/json");
    });

    svr.Get(R"(/api/buses/(\d+)/seats)", [&](const Request &req, Response &res) {
        std::lock_guard<std::mutex> lock(mu);
        int id = std::stoi(req.matches[1]);
        json buses = readJsonFile(BUSES_FILE);
        for (auto &b : buses) {
            if (b["id"].get<int>() == id) {
                set_cors(res);
                res.set_content(b["seats"].dump(), "application/json");
                return;
            }
        }
        set_cors(res);
        res.status = 404;
        res.set_content(R"({"error":"bus not found"})", "application/json");
    });

    svr.Post("/api/bookings", [&](const Request &req, Response &res) {
        set_cors(res);
        try {
            json body = json::parse(req.body);
            int busId = body.value("busId", 0);
            std::vector<int> seatNumbers = body.value("seatNumbers", std::vector<int>{});
            std::string name = body.value("name", std::string("Passenger"));
            if (busId == 0 || seatNumbers.empty()) {
                res.status = 400;
                res.set_content(R"({"error":"Missing busId or seatNumbers"})", "application/json");
                return;
            }

            std::lock_guard<std::mutex> lock(mu);
            json buses = readJsonFile(BUSES_FILE);
            bool found=false;
            for (auto &b : buses) {
                if (b["id"].get<int>() == busId) {
                    found=true;
                    for (int s : seatNumbers) {
                        bool ok=false;
                        for (auto &seat : b["seats"]) {
                            if (seat["number"].get<int>() == s) {
                                if (seat["booked"].get<bool>()) {
                                    res.status = 409;
                                    json err = { {"error","seat already booked"}, {"seat", s} };
                                    res.set_content(err.dump(), "application/json");
                                    return;
                                } else ok=true;
                            }
                        }
                        if (!ok) {
                            res.status = 400;
                            json err = { {"error","invalid seat"}, {"seat", s} };
                            res.set_content(err.dump(), "application/json");
                            return;
                        }
                    }

                    for (int s : seatNumbers) {
                        for (auto &seat : b["seats"]) {
                            if (seat["number"].get<int>() == s) seat["booked"] = true;
                        }
                    }

                    writeJsonFile(BUSES_FILE, buses);

                    json bookings = readJsonFile(BOOKINGS_FILE);
                    int nextId = 1;
                    if (!bookings.empty()) nextId = bookings.back().value("id",0) + 1;

                    json booking = {
                        {"id", nextId},
                        {"busId", busId},
                        {"busName", b["name"]},
                        {"seatNumbers", seatNumbers},
                        {"name", name},
                        {"fare", body.value("fare", 0)},
                        {"source", body.value("source", "")},
                        {"destination", body.value("destination", "")},
                        {"travelDate", body.value("travelDate", "")}
                    };

                    bookings.push_back(booking);
                    writeJsonFile(BOOKINGS_FILE, bookings);

                    json resp = { {"success", true}, {"bookingId", nextId}, {"booking", booking} };
                    res.set_content(resp.dump(), "application/json");
                    return;
                }
            }
            if (!found) {
                res.status = 404;
                res.set_content(R"({"error":"bus not found"})", "application/json");
            }
        } catch (std::exception &e) {
            res.status = 500;
            json err = { {"error", "exception"}, {"msg", e.what()} };
            res.set_content(err.dump(), "application/json");
        }
    });

    svr.Get("/api/bookings", [&](const Request &req, Response &res) {
        std::lock_guard<std::mutex> lock(mu);
        json bookings = readJsonFile(BOOKINGS_FILE);
        set_cors(res);
        res.set_content(bookings.dump(), "application/json");
    });

    std::cout << "Server listening on http://localhost:5050\n";
    svr.listen("0.0.0.0", 5050);
    return 0;
}