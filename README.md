# DPLL SAT Solver

A SAT solver implemented entirely in C for solving Boolean satisfiability problems represented in conjunctive normal form (CNF).

The project models each dish as a Boolean variable and each employee's preferences as a CNF clause. The solver determines whether there exists a menu configuration satisfying all active employee constraints.

## Algorithm

The solver implements a DPLL-style recursive search with:

- Unit propagation
- Depth-first search
- Backtracking
- Tautology detection
- Clause-based variable selection
- Sparse clause representation

Unit clauses are propagated before branching. When propagation cannot derive further assignments, the solver selects an unassigned variable and recursively explores both possible assignments.

## Data Representation

Each clause is stored as a `Clausola` structure containing:

- Variable indices
- Literal signs
- Number of literals
- Tautology status

The solver uses dynamically allocated arrays instead of a dense Boolean matrix, storing only the literals that actually occur in each clause.

## Input

The first line contains the list of available dishes.

Each following line represents one employee's preferences:

- `dish` means the dish is preferred and corresponds to a positive literal.
- `-dish` means the dish is not preferred and corresponds to a negative literal.

For example:

```text
pizza pasta salad
pizza -salad
-pizza pasta
```

The first line defines the variables. Each subsequent line is a CNF clause.

## Output

The program prints:

- `OK` if the current set of constraints is satisfiable.
- `KO` followed by the number of failed attempts if the constraints are unsatisfiable.

## Compilation

Compile with a C11-compatible compiler:

```bash
gcc -std=c11 -Wall -Wextra -pedantic progetto_api.c -o sat_solver
```

Run with:

```bash
./sat_solver
```

The program reads the input from standard input.

## Project Structure

```text
dpll-sat-solver/
├── progetto_api.c
└── README.md
```

## Context

Developed as a project for the Algorithms and Principles of Computer Science course at Politecnico di Milano.
