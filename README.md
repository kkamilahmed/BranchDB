# BranchDB

**BranchDB** is a lightweight database engine built from scratch in **C**, designed to explore database internals, indexing, and query execution.  
It provides a simple SQL-like interface, persistent storage, and an interactive REPL for executing queries.

BranchDB focuses on performance, modularity, and teaching the core concepts of database systems, including indexing, parsing, and execution pipelines.

---

## Features

- **Custom SQL-Like Query Language**  
  Supports basic SQL operations including `CREATE`, `INSERT`, `SELECT`, `DELETE`, and `UPDATE`.

- **B-Tree Indexing**  
  Optimizes key lookups and range queries with a B-tree implementation.

- **Persistent Storage**  
  Databases are modeled as directories, and tables as files, enabling persistent state across program restarts.

- **Interactive REPL**  
  Provides a command-line interface for executing queries and inspecting database state in real-time.

- **Full Compiler Pipeline**  
  Implements a lexer, recursive-descent parser, abstract syntax tree (AST) construction, and execution engine.

- **Modular Design**  
  Components like parser, executor, and storage are separate, allowing easy experimentation and extension.

---

## System Architecture

```mermaid
flowchart TD
    REPL[Interactive REPL]
    Lexer[Lexer]
    ParserNode[Parser AST]
    Executor[Executor]
    Storage[Storage Engine]
    BTree[B-Tree Indexing]
    Disk[Disk Persistence]

    REPL --> Lexer
    Lexer --> ParserNode
    ParserNode --> Executor
    Executor --> Storage
    Storage --> BTree
    Storage --> Disk
