# Survival Project

Welcome to **Survival Project**, a skeleton framework for a data‑driven survival game inspired by *Cataclysm: Dark Days Ahead*. This repository lays out a simple C++ project with a JSON content layer and a basic development workflow, intended as a starting point for your own game systems.

## Getting Started

### Prerequisites

- A C++17‑compatible compiler (e.g. GCC, Clang, or MSVC)
- [CMake](https://cmake.org/) 3.10 or newer
- Python 3 for running scripts

### Building

Clone this repository and build the project using CMake:

```bash
cmake -S . -B build
cmake --build build

# Run the resulting binary
./build/survival_project
```

Running the binary will enumerate JSON files under `data/json` as a demonstration. As you add game systems, you can expand this entry point into a full engine loop.

## Project Structure

- **src/** – C++ source files. Currently contains a minimal `main.cpp`.
- **data/json/** – Core JSON data files defining items, monsters, recipes, etc.
- **data/mods/** – Add‑on content packaged as mods. Each mod has its own folder with a `modinfo.json`.
- **scripts/** – Utility scripts for formatting and validating JSON data.
- **.github/workflows/** – Continuous integration configuration (builds the project and validates JSON on each push or pull request).

## Modding

Mods live under the `data/mods` directory. Each mod should have a `modinfo.json` file describing the mod, and any number of additional JSON files defining new items, recipes, monsters, or other content. Mods can be enabled by the game engine at load time. For examples, see `data/mods/example_mod`.

## JSON Format

Content is defined in JSON for ease of modification and contribution. Each top‑level file should contain an array of objects. The shape of each object depends on its `type`. For example, items of type `GENERIC` might include `id`, `name`, `weight`, `volume`, `description`, and `material` fields. See the files in `data/json` for simple examples.

You can use the provided `scripts/format_json.py` to pretty‑print your JSON files, and `scripts/validate_json.py` to ensure that all JSON in the repository is syntactically valid.

## Continuous Integration

GitHub Actions are configured to build the project and run JSON validation on each push and pull request targeting the `main` branch. This helps catch build errors or malformed JSON early in the development process.

## Next Steps

This skeleton is deliberately minimal. To turn it into a playable game you could:

1. Implement a core game loop and entity/component system in C++.
2. Add map generation and world simulation systems.
3. Expand the JSON schema to cover additional game elements (mutations, professions, factions, etc.).
4. Provide hooks so that JSON content can drive new mechanics without recompiling the engine.

As you develop new features, consider maintaining a clear separation between the engine (C++) and data (JSON) so that contributors can extend the game via JSON alone whenever possible.
