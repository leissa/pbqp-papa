# Copilot Instructions for pbqp-papa

## Build & Test

```sh
cmake -B build
cmake --build build
ctest --test-dir build                          # all 13 tests
ctest --test-dir build -R MatrixTests           # single test by name regex
ctest --test-dir build --output-on-failure      # show details on failure
```

Test names are path-derived: `test/graph/MatrixTests.cpp` → `graph_MatrixTests`.

Optional features (off by default, require external libs):
```sh
cmake -B build -DPBQP_USE_GVC=ON -DPBQP_USE_GUROBI=ON
```

## Architecture

This is a C++20 library for solving **Partitioned Boolean Quadratic Problems** (PBQP).

### Core graph model (`include/graph/`)

Everything is templated on a cost type `T`. The central types are:

- `PBQPGraph<T>` — owns nodes and edges, supports copy, serialization, and a PEO (Perfect Elimination Ordering)
- `PBQPNode<T>` — holds a cost `Vector<T>` and adjacency lists
- `PBQPEdge<T>` — holds a cost `Matrix<T>` between two nodes
- `Matrix<T>` / `Vector<T>` — hand-rolled owning dense matrix/vector with raw `new T[]` storage (intentionally not `std::vector` to avoid default-initialization overhead). `Vector<T>` inherits from `Matrix<T>` (single-column matrix).

### InfinityWrapper (`include/math/`)

`InfinityWrapper<T>` wraps an unsigned integer type and adds an "infinity" sentinel. Most solvers operate on `PBQPGraph<InfinityWrapper<T>>`, not `PBQPGraph<T>` directly.

### Solving pipeline (`include/solve/`, `include/reduction/`)

`StepByStepSolver<T>` is the main solver. It works by iteratively applying **reductions** (degree-0, degree-1, degree-2) that simplify the graph, then back-substituting solutions. Each reduction produces a `DependentSolution` that knows how to recover its answer once dependencies are resolved.

`BruteForceSolver<T>` exhaustively enumerates all assignments (exponential).

### Reduction framework (`include/reduction/`)

`PBQP_Reduction<T>` is the base class. Reductions return a (possibly split) graph and a `DependentSolution` for back-substitution. Key reductions:
- `DegreeZeroReducer` — trivially solve isolated nodes
- `DegreeOneReducer` — fold degree-1 nodes into their neighbor
- `DegreeTwoReducer` — contract degree-2 nodes
- `ConnectedComponentSeparator` — split disconnected components
- `InfinityFilter` — prune infinite-cost entries

### I/O (`include/io/`, `src/io/`)

`PBQP_Serializer<T>` reads/writes PBQP graphs as JSON files (via nlohmann/json). Test data lives in `test/testData/`.

### C interface (`src/CInterface.cpp`, `include/CInterface.h`)

Macro-generated `extern "C"` API for three type instantiations: `unsigned short`, `unsigned int`, `unsigned long`. The macro expansions live outside the `pbqppapa` namespace.

## Conventions

- **Namespace**: all code lives in `pbqppapa`
- **Header-heavy**: most logic is in headers under `include/`. The `src/` files are thin (explicit template instantiations, C interface, timers).
- **Testing**: [doctest](https://github.com/doctest/doctest) with `DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN`. Each test file is a standalone executable. Tests wrap their code inside `namespace pbqppapa {}`.
- **Formatting**: enforced by `.clang-format` + pre-commit hook (`pre-commit install`). Tabs for indentation, 120-column limit, attached braces.
- **No system-level deps**: all dependencies (nlohmann/json, doctest, graphviz headers) are git submodules under `submodules/`.
- **`[[nodiscard]]`** on all getter methods.
- **`= default`** for trivial destructors; **`= delete`** for non-copyable types.
