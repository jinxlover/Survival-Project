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
#include <random>

struct Item {
    std::string id;
    std::string name;
};

/**
 * Structure representing a monster definition loaded from JSON.
 * Monsters have an identifier, a display name, hit points (hp) and
 * simple combat attributes.  This struct is intentionally simple and
 * supports only the fields parsed by load_monsters() below.
 */
struct Monster {
    std::string id;
    std::string name;
    int hp = 0;
    int melee_dice = 0;
    int melee_dice_sides = 0;
    int armor = 0;
};

/**
 * Simple Player structure that holds an inventory of Item objects.
 * The player can pick up items from the world and drop them back.
 */
struct Player {
    std::vector<Item> inventory;

    /**
     * Hit points representing the player's health in combat. The player
     * starts with 100 hp and loses hp when taking damage from monsters.
     */
    int hp = 100;

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
 * Recipe structure representing a craftable recipe loaded from JSON.
 * Each recipe has an id, a resulting item id, and a list of
 * component requirements (item id and quantity).
 */
struct Recipe {
    std::string id;
    std::string result;
    std::vector<std::pair<std::string, int>> components;
};

/**
 * Load recipes from a JSON file. This parser is simplistic and only
 * extracts the "id", "result", and first level of components
 * (assumes each component entry is a two‑element array [ [ "id", qty ] ]).
 */
std::vector<Recipe> load_recipes(const std::string &filename) {
    std::vector<Recipe> recipes;
    std::ifstream f(filename);
    if (!f) {
        std::cerr << "Failed to open " << filename << std::endl;
        return recipes;
    }
    Recipe current;
    std::string line;
    while (std::getline(f, line)) {
        // Trim leading spaces
        auto pos = line.find_first_not_of(" \t");
        if (pos == std::string::npos) continue;
        std::string trimmed = line.substr(pos);
        // New recipe when encountering '{'
        if (trimmed.find("{") != std::string::npos) {
            current = Recipe();
        }
        // Parse id
        auto id_pos = trimmed.find("\"id\"");
        if (id_pos != std::string::npos) {
            auto colon = trimmed.find(':', id_pos);
            auto q1 = trimmed.find('"', colon + 1);
            auto q2 = trimmed.find('"', q1 + 1);
            if (q1 != std::string::npos && q2 != std::string::npos) {
                current.id = trimmed.substr(q1 + 1, q2 - q1 - 1);
            }
        }
        // Parse result
        auto res_pos = trimmed.find("\"result\"");
        if (res_pos != std::string::npos) {
            auto colon = trimmed.find(':', res_pos);
            auto q1 = trimmed.find('"', colon + 1);
            auto q2 = trimmed.find('"', q1 + 1);
            if (q1 != std::string::npos && q2 != std::string::npos) {
                current.result = trimmed.substr(q1 + 1, q2 - q1 - 1);
            }
        }
        // Parse components entry lines like [ [ "id", qty ] ]
        // We'll look for two quotes and a comma separating quantity
        if (trimmed.find("[ [") != std::string::npos) {
            auto q1 = trimmed.find('"');
            auto q2 = trimmed.find('"', q1 + 1);
            if (q1 != std::string::npos && q2 != std::string::npos) {
                std::string comp_id = trimmed.substr(q1 + 1, q2 - q1 - 1);
                // Find quantity after comma
                auto comma = trimmed.find(',', q2);
                if (comma != std::string::npos) {
                    std::string qty_str = trimmed.substr(comma + 1);
                    int qty = std::stoi(qty_str);
                    current.components.emplace_back(comp_id, qty);
                }
            }
        }
        // When encountering '}', push current recipe if it has id and result
        if (trimmed.find("}") != std::string::npos) {
            if (!current.id.empty() && !current.result.empty()) {
                recipes.push_back(current);
                current = Recipe();
            }
        }
    }
    return recipes;
}

/**
 * Load monsters from a JSON file. This parser reads each monster
 * definition and extracts basic combat attributes. It is line‑based
 * and similar to load_items and load_recipes, so it should be easy
 * to extend if more fields are needed. Each monster requires an
 * "id" field, a "name" object with a "str" subfield, and an "hp"
 * field. Optional fields include "melee_dice", "melee_dice_sides"
 * and "armor".
 */
std::vector<Monster> load_monsters(const std::string &filename) {
    std::vector<Monster> monsters;
    std::ifstream f(filename);
    if (!f) {
        std::cerr << "Failed to open " << filename << std::endl;
        return monsters;
    }
    Monster current;
    bool in_object = false;
    std::string line;
    auto trim = [](const std::string &s) {
        size_t start = s.find_first_not_of(" \t\n\r");
        size_t end = s.find_last_not_of(" \t\n\r");
        if (start == std::string::npos || end == std::string::npos) return std::string();
        return s.substr(start, end - start + 1);
    };
    auto extract_string_value = [&](const std::string &s) -> std::string {
        auto colon = s.find(':');
        if (colon == std::string::npos) return "";
        std::string value = s.substr(colon + 1);
        // remove commas
        value.erase(std::remove(value.begin(), value.end(), ','), value.end());
        // find first and last quote
        size_t q1 = value.find('"');
        size_t q2 = value.find_last_of('"');
        if (q1 != std::string::npos && q2 != std::string::npos && q2 > q1) {
            return value.substr(q1 + 1, q2 - q1 - 1);
        }
        // fallback: trim
        return trim(value);
    };
    auto extract_int_value = [&](const std::string &s) -> int {
        auto colon = s.find(':');
        if (colon == std::string::npos) return 0;
        std::string value = s.substr(colon + 1);
        value.erase(std::remove(value.begin(), value.end(), ','), value.end());
        value.erase(0, value.find_first_not_of(" \t"));
        try {
            return std::stoi(value);
        } catch (...) {
            return 0;
        }
    };
    while (std::getline(f, line)) {
        std::string t = trim(line);
        if (t.empty() || t == "[" || t == "]") continue;
        if (t.find('{') != std::string::npos) {
            in_object = true;
            current = Monster();
            continue;
        }
        if (t.find('}') != std::string::npos) {
            if (in_object) {
                // push only if id and name have been set
                if (!current.id.empty() && !current.name.empty()) {
                    monsters.push_back(current);
                }
                in_object = false;
            }
            continue;
        }
        if (!in_object) continue;
        if (t.find("\"id\"") != std::string::npos) {
            current.id = extract_string_value(t);
        } else if (t.find("\"name\"") != std::string::npos && t.find("\"str\"") != std::string::npos) {
            current.name = extract_string_value(t);
        } else if (t.find("\"hp\"") != std::string::npos) {
            current.hp = extract_int_value(t);
        } else if (t.find("\"melee_dice_sides\"") != std::string::npos) {
            current.melee_dice_sides = extract_int_value(t);
        } else if (t.find("\"melee_dice\"") != std::string::npos) {
            current.melee_dice = extract_int_value(t);
        } else if (t.find("\"armor\"") != std::string::npos) {
            current.armor = extract_int_value(t);
        }
    }
    return monsters;
}

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
    // Load recipes from JSON
    std::vector<Recipe> recipes = load_recipes("data/json/recipes.json");
    // Load monsters from JSON. These creatures are available to fight.
    std::vector<Monster> monsters = load_monsters("data/json/monsters.json");
    std::cout << "Loaded " << monsters.size() << " monster(s)." << std::endl;
    for (const auto &m : monsters) {
        std::cout << " - " << m.id << ": " << m.name << " (hp=" << m.hp << ")" << std::endl;
    }
    // Create the player
    Player player;
    // Command loop
    std::cout << "\nAvailable commands:\n"
              << " - list items      : list items available in the world\n"
              << " - inventory       : list items in your inventory\n"
              << " - take <id>       : pick up an item from the world\n"
              << " - drop <id>       : drop an item from your inventory\n"
              << " - craft <recipe>  : craft an item using a recipe\n"
              << " - list monsters   : list monsters in the world\n"
              << " - fight <id>      : fight a monster\n"
              << " - quit            : exit the game\n";
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
        std::istringstream iss(line);
        std::string command;
        std::string arg;
        iss >> command;
        std::getline(iss, arg);
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
        } else if (command == "craft") {
            if (arg.empty()) {
                std::cout << "Usage: craft <recipe id>" << std::endl;
                continue;
            }
            // Find recipe by id
            const Recipe *selected = nullptr;
            for (const auto &rec : recipes) {
                if (rec.id == arg) {
                    selected = &rec;
                    break;
                }
            }
            if (!selected) {
                std::cout << "Recipe '" << arg << "' not found." << std::endl;
                continue;
            }
            // Check if player has required components
            bool can_craft = true;
            std::vector<Item> removed_items;
            for (const auto &req : selected->components) {
                const std::string &comp_id = req.first;
                int qty_needed = req.second;
                int qty_found = 0;
                // Remove items up to qty_needed
                for (int i = 0; i < qty_needed; ++i) {
                    Item removed;
                    if (player.remove_item(comp_id, removed)) {
                        removed_items.push_back(removed);
                        qty_found++;
                    } else {
                        break;
                    }
                }
                if (qty_found < qty_needed) {
                    can_craft = false;
                    // Return removed items back to inventory
                    for (const auto &itm : removed_items) {
                        player.add_item(itm);
                    }
                    break;
                }
            }
            if (!can_craft) {
                std::cout << "You don't have the required components to craft '" << selected->id << "'." << std::endl;
            } else {
                // Add result item to inventory. Try to find an existing item definition
                auto it = std::find_if(world_items.begin(), world_items.end(), [&](const Item &itm) {
                    return itm.id == selected->result;
                });
                Item crafted;
                if (it != world_items.end()) {
                    crafted = *it;
                } else {
                    // If not found in world_items, create a generic item with id as name
                    crafted.id = selected->result;
                    crafted.name = selected->result;
                }
                player.add_item(crafted);
                std::cout << "You craft a " << crafted.name << "!" << std::endl;
            }
        } else if (command == "list" && arg == "monsters") {
            if (monsters.empty()) {
                std::cout << "There are no monsters in the world." << std::endl;
            } else {
                std::cout << "Monsters:" << std::endl;
                for (const auto &m : monsters) {
                    std::cout << " - " << m.id << ": " << m.name << " (hp=" << m.hp << ")" << std::endl;
                }
            }
        } else if (command == "fight") {
            if (arg.empty()) {
                std::cout << "Usage: fight <monster id>" << std::endl;
                continue;
            }
            // Find monster by id
            auto it_mon = std::find_if(monsters.begin(), monsters.end(), [&](const Monster &m) {
                return m.id == arg;
            });
            if (it_mon == monsters.end()) {
                std::cout << "Monster '" << arg << "' not found." << std::endl;
                continue;
            }
            Monster enemy = *it_mon;
            std::cout << "You engage the " << enemy.name << "!" << std::endl;
            // Simple combat loop
            while (player.hp > 0 && enemy.hp > 0) {
                // Player attacks first
                int damage = 1;
                std::string weapon_name = "fists";
                if (!player.inventory.empty()) {
                    const Item &weapon = player.inventory.front();
                    // Determine damage by summing simple fields. We don't have bashing/cutting separate,
                    // so assign a default of 5 per item as an example. In a full game this would come
                    // from item data. Here we check if the id contains "knife" or other hints.
                    damage = 5;
                    weapon_name = weapon.name;
                }
                enemy.hp -= damage;
                std::cout << "You hit the " << enemy.name << " with your " << weapon_name
                          << ", dealing " << damage << " damage. (monster hp=" << (enemy.hp > 0 ? enemy.hp : 0) << ")" << std::endl;
                if (enemy.hp <= 0) {
                    std::cout << "You defeated the " << enemy.name << "!" << std::endl;
                    break;
                }
                // Monster attacks
                int monster_damage = enemy.melee_dice * enemy.melee_dice_sides;
                if (monster_damage <= 0) {
                    monster_damage = 1;
                }
                player.hp -= monster_damage;
                std::cout << "The " << enemy.name << " hits you, dealing " << monster_damage
                          << " damage. (your hp=" << (player.hp > 0 ? player.hp : 0) << ")" << std::endl;
                if (player.hp <= 0) {
                    std::cout << "You were killed by the " << enemy.name << "..." << std::endl;
                    break;
                }
            }
            if (player.hp <= 0) {
                // Game over
                break;
            }
        } else {
            std::cout << "Unknown command. Type 'list items', 'list monsters', 'inventory', 'take <id>', 'drop <id>', 'craft <recipe>', 'fight <id>' or 'quit'." << std::endl;
        }
    }
    std::cout << "Goodbye!" << std::endl;
    return 0;
}