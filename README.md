# pbqp-papa
Solver and analysing tool for Partitioned Boolean Quadratic Problems

Made as part of my bachelor thesis at KIT in February 2019


---

## Building instructions:

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

## Dependencies

All dependencies are included as git submodules:

- [nlohmann/json](https://github.com/nlohmann/json) — JSON parsing
- [doctest](https://github.com/doctest/doctest) — unit testing
- [graphviz](https://gitlab.com/graphviz/graphviz) — graph visualization (only for `make all`)
