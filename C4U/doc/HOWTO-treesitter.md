


https://github.com/tree-sitter/tree-sitter/blob/master/docs/src/6-contributing.md

git clone https://github.com/tree-sitter/tree-sitter
cd tree-sitter


https://tree-sitter.github.io/tree-sitter/using-parsers/1-getting-started.html


Building the Library
To build the library on a POSIX system, just run make in the Tree-sitter directory. This will create a static library called libtree-sitter.a as well as dynamic libraries.
Alternatively, you can incorporate the library in a larger project's build system by adding one source file to the build. This source file needs two directories to be in the include path when compiled:
source file:
tree-sitter/lib/src/lib.c
include directories:
tree-sitter/lib/src
tree-sitter/lib/include

An Example Program
Here's an example of a simple C program that uses the Tree-sitter JSON parser.
