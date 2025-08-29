#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>

// This minimal entry point demonstrates how the engine
// can enumerate JSON data files. In a full engine,
// additional subsystems (e.g. entity/component system,
// rendering, game loop) would be added here.

int main() {
    std::cout << "Survival Project skeleton loaded." << std::endl;
    namespace fs = std::filesystem;
    fs::path data_path = fs::path("data/json");
    if (!fs::exists(data_path)) {
        std::cerr << "Data directory not found: " << data_path << std::endl;
        return 1;
    }
    // Iterate through JSON files and print their names
    for (const auto &entry : fs::directory_iterator(data_path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            std::cout << "Found JSON file: " << entry.path() << std::endl;
        }
    }
    return 0;
}
