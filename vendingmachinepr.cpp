#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <iomanip>
#include <math.h>
#include <algorithm>
#include <cctype>

using std::cin;
using std::cout;
using std::endl;
using std::string;

static inline string trim(const string &s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}

struct Item {
    string code;       // for examplee "A1"
    string name;       // for example "Coke"
    string category;   // for example "Cold Drinks"
    int pricePence;    // price in pence
    int stock;         // number of items available

    Item() : code(""), name(""), category(""), pricePence(0), stock(0) {}
    Item(const string& c, const string& n, const string& cat, int p, int s)
        : code(c), name(n), category(cat), pricePence(p), stock(s) {}
};

// Vending machine class
class VendingMachine {
private:
    std::map<string, Item> items;
    std::map<string, std::vector<string>> categoryMap;

public:
    VendingMachine() {
        seedItems();
    }

    void seedItems() {
        addItem(Item("A1", "Coke (330ml)", "Cold Drinks", 120, 5));
        addItem(Item("A2", "Diet Coke (330ml)", "Cold Drinks", 120, 3));
        addItem(Item("A3", "Water (500ml)", "Cold Drinks", 80, 10));
        addItem(Item("B1", "Chocolate Bar", "Snacks", 95, 6));
        addItem(Item("B2", "Biscuits Pack", "Snacks", 150, 4));
        addItem(Item("C1", "Tea (Hot)", "Hot Drinks", 110, 8));
        addItem(Item("C2", "Coffee (Hot)", "Hot Drinks", 140, 7));
        addItem(Item("D1", "Sandwich", "Food", 250, 2));
    }

    void addItem(const Item& it) {
        items[it.code] = it;
        categoryMap[it.category].push_back(it.code);
    }

    void showMenu() const {
        cout << "========== VENDING MACHINE MENU ==========" << endl;
        for (const auto& catPair : categoryMap) {
            cout << "\n-- " << catPair.first << " --" << endl;
            for (const string& code : catPair.second) {
                const Item& it = items.at(code);
                cout << std::left << std::setw(4) << it.code << " "
                     << std::setw(22) << it.name << "  "
                     << std::fixed << std::setprecision(2)
                     << (it.pricePence / 100.0)
                     << "  (stock: " << it.stock << ")" << endl;
            }
        }
        cout << "==========================================" << endl;
    }

    bool validCode(const string& code) const {
        return items.find(code) != items.end();
    }

    // Returns true if purchase succeeded
    bool purchaseFlow() {
        showMenu();
        cout << "\nType item code to add to basket (or type DONE to finish):" << endl;
        std::vector<string> basket;
        while (true) {
            cout << "> ";
            string input;
            if (!std::getline(cin, input)) return false;
            input = trim(input);
            std::transform(input.begin(), input.end(), input.begin(), ::toupper);

            if (input == "DONE") break;
            if (!validCode(input)) {
                cout << "Invalid code. Please try again." << endl;
                continue;
            }
            if (items.at(input).stock <= 0) {
                cout << "Sorry, " << items.at(input).name << " is out of stock." << endl;
                continue;
            }
            basket.push_back(input);
            cout << items.at(input).name << " added to basket." << endl;
            // Suggest related item (simple)
            suggestRelated(input);
        }

        if (basket.empty()) {
            cout << "No items selected. Returning to main menu.\n";
            return false;
        }

        int totalPence = 0;
        cout << "\nYour basket:" << endl;
        for (const string& c : basket) {
            const Item& it = items.at(c);
            cout << "- " << it.name << " ( " << (it.pricePence / 100.0) << ")\n";
            totalPence += it.pricePence;
        }
        cout << "Total:  " << std::fixed << std::setprecision(2) << (totalPence / 100.0) << endl;

        // Capture money
        int paidPence = captureMoney(totalPence);
        if (paidPence < totalPence) {
            cout << "Transaction cancelled. Returning inserted money." << endl;
            return false;
        }

        // Dispense items & update stock
        cout << "\nDispensing items..." << endl;
        for (const string& c : basket) {
            Item& it = items[c];
            it.stock -= 1;
            cout << "Dispensed: " << it.name << endl;
        }

        int change = paidPence - totalPence;
        if (change > 0) {
            cout << "Returning change:  " << std::fixed << std::setprecision(2) << (change / 100.0) << endl;
            printChangeBreakdown(change);
        } else {
            cout << "No change to return." << endl;
        }

        cout << "Thank you for your purchase!\n" << endl;
        return true;
    }

    // Suggest a related product (simple map)
    void suggestRelated(const string& code) const {
        // Very simple suggestions based on category
        const Item& it = items.at(code);
        if (it.category == "Hot Drinks") {
            cout << "Suggestion: Would you like biscuits? (type B2 to add)" << endl;
        } else if (it.category == "Cold Drinks") {
            cout << "Suggestion: A snack pairs well. Try B1 (Chocolate Bar)." << endl;
        } else if (it.category == "Snacks") {
            cout << "Suggestion: Add a drink from C1/C2 or A1." << endl;
        }
    }

    // Read money from user
    int captureMoney(int requiredPence) const {
        cout << "\nPlease insert money. You may input any amount (e.g. 2.50 for  2.50)." << endl;
        int totalInserted = 0;
        while (totalInserted < requiredPence) {
            cout << "Inserted so far:  " << std::fixed << std::setprecision(2)
                 << (totalInserted / 100.0) << "  |  Remaining:  "
                 << std::fixed << std::setprecision(2) << ((requiredPence - totalInserted) / 100.0)
                 << "\nEnter amount (or type CANCEL): ";
            string s;
            if (!std::getline(cin, s)) return 0;
            s = trim(s);
            std::transform(s.begin(), s.end(), s.begin(), ::toupper);
            if (s == "CANCEL") {
                return 0;
            }
            try {
                // Convert to pence safely
                double val = std::stod(s);
                if (val <= 0) {
                    cout << "Please enter positive amounts only." << endl;
                    continue;
                }
                int pence = static_cast<int>(std::round(val * 100.0));
                totalInserted += pence;
            } catch (...) {
                cout << "Invalid number format. Use e.g. 1.20 or 0.50" << endl;
            }
        }
        return totalInserted;
    }

    void printChangeBreakdown(int changePence) const {
        std::vector<int> coins = {200, 100, 50, 20, 10, 5, 2, 1}; // pence
        cout << "Change breakdown:" << endl;
        for (int c : coins) {
            int count = changePence / c;
            if (count > 0) {
                cout << count << " x ";
                if (c >= 100) cout << " " << (c / 100);
                else cout << c << "p";
                cout << endl;
                changePence -= count * c;
            }
        }
    }

    // Admin: restock an item
    void restock() {
        cout << "Enter item code to restock: ";
        string code; std::getline(cin, code);
        code = trim(code);
        std::transform(code.begin(), code.end(), code.begin(), ::toupper);
        if (!validCode(code)) { cout << "Invalid code.\n"; return; }
        cout << "Enter quantity to add: ";
        string qs; std::getline(cin, qs);
        try {
            int q = std::stoi(trim(qs));
            if (q <= 0) { cout << "Must be positive.\n"; return; }
            items[code].stock += q;
            cout << "Restocked " << items[code].name << ". New stock: " << items[code].stock << endl;
        } catch (...) { cout << "Bad number.\n"; }
    }

    void showStock() const {
        cout << "\nCurrent stock levels:\n";
        for (const auto& p : items) {
            cout << p.first << " - " << p.second.name << " : " << p.second.stock << endl;
        }
    }
};

int main() {
    VendingMachine vm;
    while (true) {
        cout << "\n=== VENDING MACHINE ===\n";
        cout << "1. Buy items\n2. Show menu\n3. Show stock (admin)\n4. Restock (admin)\n5. Exit\nChoose an option: ";
        string opt;
        if (!std::getline(cin, opt)) break;
        opt = trim(opt);
        if (opt == "1") {
            vm.purchaseFlow();
        } else if (opt == "2") {
            vm.showMenu();
        } else if (opt == "3") {
            vm.showStock();
        } else if (opt == "4") {
            vm.restock();
        } else if (opt == "5" || opt == "exit" || opt == "q") {
            cout << "Goodbye!\n";
            break;
        } else {
            cout << "Invalid choice. Please select 1-5.\n";
        }
    }
    return 0;
}