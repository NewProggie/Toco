# Toco
Toco is a small toy compiler using Flex (Lexer), Bison (Parser) and LLVM for 
generating the Assembly

## Installation requirements
In order to build Toco you need Flex, Bison, LLVM and a recent C++ compiler. If
you're on macOS you may use homebrew for installing all the dependencies:

    $ brew install flex bison llvm
    $ export PATH="/usr/local/Cellar/flex/2.6.3/bin:$PATH"
    $ export PATH="/usr/local/Cellar/bison/3.0.4/bin:$PATH"

## Basic compiler toolchain
Following are the different parts of Toco which will parse source code and
transform it into a binary

    +----------+    +----------+    +----------+
    | Lexical  |    | Semantic |    | Assembly |
    | Analysis |--->| Parsing  |--->|          |
    | (Flex)   |    | (Bison)  |    |  (LLVM)  |
    +----------+    +----------+    +----------+

The lexer (compiler frontend) takes some source code (text) as input and
generates a sequence of tokens (strings with an assigned and identified
meaning, for instance identifiers, keywords, numbers, brackets etc.)

The semantic parsing part takes the sequence of tokens and generates an
abstract syntax tree (AST). We will use Bison for this part.

Finally we traverse our previously generated AST and generate byte code /
machine readable code for each node using LLVM.

## Defining the grammar of Toco
As a first step, we will use the well known C-like syntax for Toco, since this
is easy to parse and fairly simple. For instance, the following snippet will be
valid code for Toco

    int add_two_numbers(int a, int b) {
        int res = a + b;
        return res;
    }

