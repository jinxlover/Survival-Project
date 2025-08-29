// Simple sci-fi roguelike prototype
// Implements a single test level with basic enemy drones and permadeath.

#include <algorithm>
#include <iostream>
#include <random>
#include <string>
#include <vector>

struct Entity {
    int x;
    int y;
    char symbol;
};

class Game {
  public:
    Game();
    void run();

  private:
    static constexpr int width = 10;
    static constexpr int height = 10;

    std::vector<std::string> map; // level layout
    Entity player;
    std::vector<Entity> enemies;
    std::mt19937 rng;
    bool player_alive = true;

    void draw() const;
    bool is_walkable(int x, int y) const;
    void move_player(char input);
    void move_enemies();
};

Game::Game()
    : map({
          "##########",
          "#........#",
          "#..##....#",
          "#........#",
          "#........#",
          "#........#",
          "#....##..#",
          "#........#",
          "#........#",
          "##########"}),
      player{1, 8, '@'},
      enemies{{8, 1, 'D'}, {5, 4, 'D'}},
      rng(std::random_device{}()) {}

void Game::draw() const {
    std::vector<std::string> buffer = map;
    buffer[player.y][player.x] = player.symbol;
    for (const auto &e : enemies) {
        buffer[e.y][e.x] = e.symbol;
    }
    for (const auto &line : buffer) {
        std::cout << line << '\n';
    }
}

bool Game::is_walkable(int x, int y) const {
    return x >= 0 && x < width && y >= 0 && y < height && map[y][x] == '.';
}

void Game::move_player(char input) {
    int nx = player.x;
    int ny = player.y;
    switch (input) {
    case 'w':
    case 'W':
        ny--;
        break;
    case 's':
    case 'S':
        ny++;
        break;
    case 'a':
    case 'A':
        nx--;
        break;
    case 'd':
    case 'D':
        nx++;
        break;
    default:
        return;
    }
    if (!is_walkable(nx, ny)) {
        return;
    }

    // Check if an enemy occupies the destination.
    auto it = std::find_if(enemies.begin(), enemies.end(), [&](const Entity &e) {
        return e.x == nx && e.y == ny;
    });
    if (it != enemies.end()) {
        std::cout << "You blast the drone!\n";
        enemies.erase(it);
    }

    player.x = nx;
    player.y = ny;
}

void Game::move_enemies() {
    std::uniform_int_distribution<int> dir(0, 3);
    for (auto &e : enemies) {
        int nx = e.x;
        int ny = e.y;
        switch (dir(rng)) {
        case 0:
            ny--;
            break;
        case 1:
            ny++;
            break;
        case 2:
            nx--;
            break;
        case 3:
            nx++;
            break;
        }
        if (!is_walkable(nx, ny)) {
            continue;
        }
        if (nx == player.x && ny == player.y) {
            std::cout << "A drone caught you!\n";
            player_alive = false;
            return;
        }
        bool occupied = false;
        for (const auto &other : enemies) {
            if (&other != &e && other.x == nx && other.y == ny) {
                occupied = true;
                break;
            }
        }
        if (!occupied) {
            e.x = nx;
            e.y = ny;
        }
    }
}

void Game::run() {
    std::cout << "You awaken on an abandoned space station."
                 " Defeat the rogue drones to survive.\n";
    while (player_alive) {
        draw();
        std::cout << "Move with WASD: ";
        char input;
        std::cin >> input;
        move_player(input);
        if (enemies.empty()) {
            std::cout << "All drones defeated. You escape... for now.\n";
            break;
        }
        move_enemies();
        if (!player_alive) {
            std::cout << "You are dead. Game over.\n";
            break;
        }
    }
}

int main() {
    Game g;
    g.run();
    return 0;
}

