# Survival Project – Sci-Fi Roguelike Prototype

This repository now hosts a small **sci-fi roguelike** prototype. The game
features a single test level aboard an abandoned space station. Defeat the
hostile drones and avoid permadeath.

## Building

```bash
cmake -S . -B build
cmake --build build
./build/survival_project
```

## Gameplay

- Control the explorer with **WASD**.
- Move into a drone to destroy it.
- If a drone reaches you, the run ends permanently.

## Project Structure

- `src/main.cpp` – core game loop and level data.
- `data/` – placeholder directory for future content.

This prototype provides a foundation for expanding into a full roguelike
experience with procedural levels and richer gameplay.

