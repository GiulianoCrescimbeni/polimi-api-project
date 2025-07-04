# Polimi PFAPI 2023-2024 – Bakery Manager

This repository contains the solution for the **"Prova Finale di Algoritmi e Principi dell'Informatica"** (PFAPI) project, academic year 2023–2024, at Politecnico di Milano.  
The objective of the project is to implement a bakery order and supply management system in C, with particular attention to efficiency and correctness.

## Project Description

The program simulates a bakery operating in discrete time. It handles:
- Recipe catalog management
- Inventory of ingredient batches with expiration dates
- Order processing based on ingredient availability and expiration
- Periodic dispatch of prepared orders with courier constraints

Orders and commands are processed from `stdin`, and results are printed to `stdout`.

All functionalities have been implemented in compliance with the project specifications provided by the course.

## Features

- Add / remove recipes with arbitrary ingredients and quantities
- Restock the inventory with ingredient batches and expiration dates
- Handle customer orders, possibly placing them on hold
- Periodic truck dispatch with constraints on weight and loading policy
- Input parsing and efficient data structures for recipe lookup and ingredient selection

## Data Structures

The implementation heavily relies on **hash tables** to ensure average constant-time access to key components of the system:

- **Recipe Hash Table**: maps recipe names to their corresponding list of ingredients and quantities.
- **Inventory Hash Table**: maps ingredient names to a priority queue (min-heap) of batches, sorted by expiration date.
- **Pending Orders List**: stores orders that cannot be fulfilled immediately.
- **Order Management**: fulfilled orders are queued and sorted based on weight and timestamp for optimal truck dispatch.

The use of hash tables improves performance especially for:
- Fast lookups and updates during order processing
- Efficient removal and addition of ingredients and recipes
- Scalable handling of a large number of commands

All hash tables are implemented using **open addressing** with **linear probing** for collision resolution.

### Build

Use the provided `Makefile`:

```bash
make
```

This will compile the project and generate the executable main.

### Run

To run the program, feed it with an input file:

```bash
./main < input.txt
```

### Example

```bash
t: 0
aggiungi_ricetta torta farina 50 uova 10 zucchero 20
=> aggiunta

t: 5
ordine torta 1
=> accettato
```

## Testing and Analysis Tools

To ensure both correctness and efficiency, the following tools were used during development:

### Memory Leak Detection

- **Valgrind – Memcheck**  
  Used to detect memory leaks, use-after-free errors, double-frees, and uninitialized memory reads.

  ```bash
  valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./main < input.txt
  ```

### Performance Profiling

- **Valgrind – Callgrind
  Used to profile function calls and identify performance bottlenecks.
  ```bash
  valgrind --tool=callgrind ./main < input.txt
  kcachegrind callgrind.out.<pid>  # View call graph in GUI
  ```

- **Valgrind – Massif
  Used to analyze dynamic memory usage over time.
  ```bash
  valgrind --tool=massif ./main < input.txt
  massif-visualizer massif.out.<pid>  # View memory timeline in GUI
  ```

### Runtime Debugging

- ***GDB and AddressSanitizer (ASAN)
  Used to debug runtime errors and catch out-of-bounds memory accesses.
  ```bash
  gcc -g3 -fsanitize=address -o main main.c
  ./main < input.txt
  ```

These tools helped ensure that the program is memory-safe, performant, and fully compliant with the testing framework provided by the PFAPI course.
