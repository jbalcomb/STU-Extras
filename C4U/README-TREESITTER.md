FuncExtract Tree-sitter variant
================================

This directory includes `FuncExtract_treesitter.c`, a minimal example that uses the Tree-sitter C API
to parse a C file and print top-level function names.

Building
--------

- If your system provides `pkg-config` packages `tree-sitter` and `tree-sitter-c`, the top-level CMake will
  pick them up and build the `FuncExtract_treesitter` target when `USE_TREESITTER` is ON (default).
- If not, install Tree-sitter development packages or build the runtime and the C language parser.

Example build (Linux/macOS with pkg-config):

```sh
mkdir build && cd build
cmake .. -DUSE_TREESITTER=ON
cmake --build .
```

If CMake reports Tree-sitter not found, install the Tree-sitter runtime and the C grammar (packaged as
`tree-sitter-c` or `tree-sitter-c` library) or set `TS_INCLUDE_DIR`, `TS_LIB` and `TS_C_LIB` manually in CMake.

Usage
-----

Run the built executable with a C source file:

```sh
./FuncExtract_treesitter path/to/source.c
```
