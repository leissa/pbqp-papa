# pbqp-papa
Solver and analysing tool for Partitioned Boolean Quadratic Problems

Made as part of my bachelor thesis at KIT in February 2019


---

## Building instructions:

`git clone --recursive https://github.com/leissa/pbqp-papa.git`

`make` will build the core functionality and run the tests involving it

`make all` will build everything, but requires you to have [graphviz](https://graphviz.gitlab.io/) and [Gurobi](http://www.gurobi.com/) installed

Compiler flags for toggling asserts, gurobi etc. are all located under `include/graph/Matrix.hpp`

## Dependencies

All dependencies are included as git submodules:

- [nlohmann/json](https://github.com/nlohmann/json) — JSON parsing
- [doctest](https://github.com/doctest/doctest) — unit testing
- [graphviz](https://gitlab.com/graphviz/graphviz) — graph visualization (only for `make all`)
