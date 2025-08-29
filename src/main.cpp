/*
 * Main entry point for the Survival Project.
 *
 * This file implements a very simple game loop that loads item
 * definitions from a JSON file and allows the player to list
 * the available items or quit the game. The JSON is parsed
 * using a rudimentary line‑based parser that looks for
 * "id" and "str" fields. This avoids the need for an
 * external JSON library and keeps the example self‑contained.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

struct Item {
    std::string id;
    std::string name;
};

/**
 * Load items from the given JSON file. This function performs a very
 * simplistic parse that extracts the value of the "id" field and
 * the "str" field under the "name" object. Each complete item is
 * appended to the returned vector. If the file cannot be opened,
 * an empty vector is returned and an error is printed to stderr.
 */
std::vector<Item> load_items(const std::string &filename) {
    std::vector<Item> items;
    std::ifstream f(filename);
    if (!f) {
        std::cerr << "Failed to open " << filename << std::endl;
        return items;
    }
    Item current;
    std::string line;
    while (std::getline(f, line)) {
        // Look for "id": "value"
        auto id_pos = line.find("\"id\"");
        if (id_pos != std::string::npos) {
            auto colon = line.find(':', id_pos);
            auto q1 = line.find('"', colon + 1);
            auto q2 = line.find('"', q1 + 1);
            if (q1 != std::string::npos && q2 != std::string::npos) {
                current.id = line.substr(q1 + 1, q2 - q1 - 1);
            }
        }
        // Look for "str": "value" (the item name)
        auto name_pos = line.find("\"str\"");
        if (name_pos != std::string::npos) {
            auto colon = line.find(':', name_pos);
            auto q1 = line.find('"', colon + 1);
            auto q2 = line.find('"', q1 + 1);
            if (q1 != std::string::npos && q2 != std::string::npos) {
                current.name = line.substr(q1 + 1, q2 - q1 - 1);
                // When we find a name we assume the item record is complete
                items.push_back(current);
                current = Item();
            }
        }
    }
    return items;
}

int main() {
    std::cout << "Welcome to the Survival Project!" << std::endl;
    // Load items from the default JSON file
    std::vector<Item> items = load_items("data/json/items.json");
    std::cout << "Loaded " << items.size() << " item(s)." << std::endl;
    for (const auto &item : items) {
        std::cout << " - " << item.id << ": " << item.name << std::endl;
    }
    // Simple command loop
    std::string command;
    while (true) {
        std::cout << "\nEnter command (type 'list items' or 'quit'): ";
        if (!std::getline(std::cin, command)) {
            break;
        }
        if (command == "quit") {
            break;
        } else if (command == "list items") {
            for (const auto &item : items) {
                std::cout << " - " << item.id << ": " << item.name << std::endl;
            }
        } else {
            std::cout << "Unknown command. Available commands: 'list items', 'quit'." << std::endl;
        }
    }
    std::cout << "Goodbye!" << std::endl;
    return 0;
}