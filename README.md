# pbqp-papa

[![CI](https://github.com/leissa/pbqp-papa/actions/workflows/ci.yml/badge.svg)](https://github.com/leissa/pbqp-papa/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE.txt)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)

Solver and analysing tool for Partitioned Boolean Quadratic Problems

## Building Instructions

`git clone --recursive https://github.com/leissa/pbqp-papa.git`

```sh
cmake -B build
cmake --build build
ctest --test-dir build
```

To build with Graphviz and Gurobi support:

```sh
cmake -B build -DPBQP_USE_GVC=ON -DPBQP_USE_GUROBI=ON
cmake --build build
```

## Development Setup

Install the [pre-commit](https://pre-commit.com/) hook to enforce consistent
formatting via `clang-format`:

```sh
pip install pre-commit   # if not already installed
pre-commit install
```

Staged C/C++ files are automatically formatted on each commit.

## Dependencies

All dependencies are included as git submodules:

- [nlohmann/json](https://github.com/nlohmann/json) — JSON parsing
- [doctest](https://github.com/doctest/doctest) — unit testing
- [graphviz](https://gitlab.com/graphviz/graphviz) — graph visualization (only for `make all`)

## Credits

Original solver by [Max Baumstark](https://github.com/Maxopoly).
See [*Development of a library for solving and analyzing PBQP*](https://pp.ipd.kit.edu/uploads/publikationen/mbaumstark19bachelorarbeit.pdf).
