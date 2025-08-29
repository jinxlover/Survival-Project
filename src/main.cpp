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
#include <sstream>

struct Item {
    std::string id;
    std::string name;
};

/**
 * Simple Player structure that holds an inventory of Item objects.
 * The player can pick up items from the world and drop them back.
 */
struct Player {
    std::vector<Item> inventory;

    /**
     * Add an item to the player's inventory.
     */
    void add_item(const Item &item) {
        inventory.push_back(item);
    }

    /**
     * Remove an item by id from the player's inventory.
     * Returns true if removed, false if not found.
     */
    bool remove_item(const std::string &item_id, Item &out_item) {
        for (auto it = inventory.begin(); it != inventory.end(); ++it) {
            if (it->id == item_id) {
                out_item = *it;
                inventory.erase(it);
                return true;
            }
        }
        return false;
    }
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
    // Load items from the default JSON file. These items represent
    // the available objects in the world that the player can pick up.
    std::vector<Item> world_items = load_items("data/json/items.json");
    std::cout << "Loaded " << world_items.size() << " item(s)." << std::endl;
    for (const auto &item : world_items) {
        std::cout << " - " << item.id << ": " << item.name << std::endl;
    }
    // Create the player
    Player player;
    // Command loop
    std::cout << "\nAvailable commands:\n"
              << " - list items    : list items available in the world\n"
              << " - inventory     : list items in your inventory\n"
              << " - take <id>     : pick up an item from the world\n"
              << " - drop <id>     : drop an item from your inventory\n"
              << " - quit          : exit the game\n";
    std::string line;
    while (true) {
        std::cout << "\nEnter command: ";
        if (!std::getline(std::cin, line)) {
            break;
        }
        // Trim leading spaces
        size_t start = line.find_first_not_of(' ');
        if (start == std::string::npos) {
            continue;
        }
        std::string command;
        std::string arg;
        std::istringstream iss(line);
        iss >> command;
        std::getline(iss, arg);
        // Remove leading space from arg
        if (!arg.empty() && arg[0] == ' ') arg.erase(0, 1);
        if (command == "quit") {
            break;
        } else if (command == "list" && arg == "items") {
            if (world_items.empty()) {
                std::cout << "There are no items in the world." << std::endl;
            } else {
                std::cout << "World items:" << std::endl;
                for (const auto &item : world_items) {
                    std::cout << " - " << item.id << ": " << item.name << std::endl;
                }
            }
        } else if (command == "inventory") {
            if (player.inventory.empty()) {
                std::cout << "Your inventory is empty." << std::endl;
            } else {
                std::cout << "Inventory:" << std::endl;
                for (const auto &item : player.inventory) {
                    std::cout << " - " << item.id << ": " << item.name << std::endl;
                }
            }
        } else if (command == "take") {
            if (arg.empty()) {
                std::cout << "Usage: take <item id>" << std::endl;
                continue;
            }
            bool found = false;
            for (auto it = world_items.begin(); it != world_items.end(); ++it) {
                if (it->id == arg) {
                    player.add_item(*it);
                    std::cout << "You pick up the " << it->name << "." << std::endl;
                    world_items.erase(it);
                    found = true;
                    break;
                }
            }
            if (!found) {
                std::cout << "Item '" << arg << "' not found in the world." << std::endl;
            }
        } else if (command == "drop") {
            if (arg.empty()) {
                std::cout << "Usage: drop <item id>" << std::endl;
                continue;
            }
            Item removed;
            if (player.remove_item(arg, removed)) {
                world_items.push_back(removed);
                std::cout << "You drop the " << removed.name << "." << std::endl;
            } else {
                std::cout << "Item '" << arg << "' not found in your inventory." << std::endl;
            }
        } else {
            std::cout << "Unknown command. Type 'list items', 'inventory', 'take <id>', 'drop <id>', or 'quit'." << std::endl;
        }
    }
    std::cout << "Goodbye!" << std::endl;
    return 0;
}