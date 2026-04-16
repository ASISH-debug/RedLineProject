#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <ctime>
#include <cstdlib>
using namespace std;

class Seat {
    int number;
    bool booked;
    bool selected;

public:
    Seat(int num, bool isBooked) : number(num), booked(isBooked), selected(false) {}

    int getNumber() { return number; }
    bool isBooked() { return booked; }
    bool isSelected() { return selected; }

    void selectSeat() { selected = true; }
};

class Bus {
    string name;
    int price;

public:
    Bus(string n, int p) : name(n), price(p) {}

    string getName() { return name; }
    int getPrice() { return price; }
};

class SeatManager {
    vector<Seat> seats;

public:
    SeatManager(int totalSeats) {
        srand(time(0));
        for (int i = 1; i <= totalSeats; i++)
            seats.push_back(Seat(i, rand() % 5 == 0));
    }

    void displaySeats() {
        cout << "\nSeat Layout (X=booked, S=selected, O=available):\n";
        for (int i = 0; i < seats.size(); i++) {
            if (seats[i].isBooked())
                cout << setw(3) << "X";
            else if (seats[i].isSelected())
                cout << setw(3) << "S";
            else
                cout << setw(3) << "O";

            if ((i + 1) % 5 == 0) cout << endl;
        }
    }

    bool selectSeat(int seatNum) {
        if (seatNum < 1 || seatNum > seats.size()) return false;
        if (seats[seatNum - 1].isBooked()) return false;
        if (seats[seatNum - 1].isSelected()) return false;
        seats[seatNum - 1].selectSeat();
        return true;
    }
};

class Payment {
    string name, cardNumber, expiry, cvv;

public:
    void collectPayment() {
        cout << "\n=== Payment Section ===\n";
        cout << "Enter name on card: ";
        cin.ignore();
        getline(cin, name);
        cout << "Enter card number: ";
        cin >> cardNumber;
        cout << "Enter expiry (MM/YY): ";
        cin >> expiry;
        cout << "Enter CVV: ";
        cin >> cvv;
        cout << "\nProcessing payment...\n";
    }
};

class BookingSystem {
    vector<string> cities;
    vector<Bus> buses;

public:
    BookingSystem() {
        cities = {"Hyderabad", "Bangalore", "Chennai", "Vijayawada", "Visakhapatnam",
                  "Delhi", "Mumbai", "Pune", "Kolkata", "Ahmedabad"};

        buses = { Bus("Morning Express", 600),
                  Bus("Night Rider", 800) };
    }

    void displayCities() {
        for (int i = 0; i < cities.size(); i++)
            cout << i + 1 << ". " << cities[i] << endl;
    }

    void displayBuses() {
        cout << "\nAvailable Buses:\n";
        for (int i = 0; i < buses.size(); i++)
            cout << i + 1 << ". " << buses[i].getName() << " - ₹" << buses[i].getPrice() << endl;
    }

    void start() {
        cout << "🚌 Welcome to RedLine Bus Booking System 🚌\n\n";

        cout << "Available Cities:\n";
        displayCities();

        int from, to;
        cout << "\nEnter source city number: ";
        cin >> from;
        cout << "Enter destination city number: ";
        cin >> to;

        if (from == to || from < 1 || to < 1 || from > cities.size() || to > cities.size()) {
            cout << "Invalid city selection.\n";
            return;
        }

        string date;
        cout << "Enter date (DD/MM/YYYY): ";
        cin >> date;

        displayBuses();
        int busChoice;
        cout << "Select a bus: ";
        cin >> busChoice;

        if (busChoice < 1 || busChoice > buses.size()) {
            cout << "Invalid bus selection.\n";
            return;
        }

        Bus selected = buses[busChoice - 1];
        cout << "\nYou selected: " << selected.getName()
             << " (₹" << selected.getPrice() << " per seat)\n";

        SeatManager seatManager(30);
        int totalFare = 0;
        char more; 
        int seatNum;

        do {
            seatManager.displaySeats();
            cout << "\nEnter seat number: ";
            cin >> seatNum;

            if (seatManager.selectSeat(seatNum)) {
                totalFare += selected.getPrice();
                cout << "Seat " << seatNum << " selected.\n";
            } else {
                cout << "Invalid seat.\n";
            }

            cout << "Select another seat? (y/n): ";
            cin >> more;

        } while (tolower(more) == 'y');

        cout << "\nTotal Fare: ₹" << totalFare << endl;

        Payment payment;
        payment.collectPayment();

        cout << "✅ Booking Successful! Thank you for choosing RedLine Bus 🚌\n";
    }
};

int main() {
    BookingSystem system;
    system.start();
    return 0;
}