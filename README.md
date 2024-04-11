# CFG Parser Project

## Overview

This C++ program is designed to parse descriptions of context-free grammars (CFGs), enabling a range of analyses and transformations based on the grammar provided. Depending on the command line arguments, it can perform various tasks including listing terminals and non-terminals, removing useless symbols, calculating FIRST and FOLLOW sets, and determining the existence of a predictive parser.

## Features

- **List Terminals and Non-terminals**: Prints the list of terminals and non-terminals in their appearance order in the grammar rules.
- **Remove Useless Symbols**: Identifies and removes rules with symbols that do not contribute to generating the language of the grammar.
- **Calculate FIRST Sets**: Computes the FIRST sets for non-terminal symbols to aid in parsing decision-making.
- **Calculate FOLLOW Sets**: Determines FOLLOW sets, assisting in the construction of parsing tables for non-terminal symbols.
- **Predictive Parser Determination**: Analyzes the grammar to determine if it is suitable for predictive parsing.

## Getting Started

### Prerequisites

- A C++ compiler (e.g., GCC, Clang)
- Basic knowledge of command line operations and C++ programming

### Installation

Clone the repository to your local machine:

```bash
git clone https://github.com/hayesZach/cfgparser.git
```

### Usage

To run the program, first compile it using your preferred C++ compiler. For example,

```bash
g++ inputbuf.cc lexer.cc parser.cc -o cfgparser
```

Then, you can run the program by specifying the task number as a command line argument and providing the input grammar through standard input:

```bash
./cfgparser 1 < input.txt
```
Here, `1` represents the task number you wish to execute (1-5), and `input.txt` is the file containing your CFG description.

### Input Format
The input should be a context-free grammar specified in the following format:

```mathematica
Grammar -> Rule-list HASH
Rule-list -> Rule Rule-list | Rule
Id-list -> ID Id-list | ID
Rule -> ID ARROW Right-hand-side STAR
Right-hand-side -> Id-list | EPSILON
```

Where:
```mathematica
ID = letter (letter + digit)*
STAR = *
HASH = #
EPSILON = STAR
ARROR = ->

letter = a-z | A-Z
digit = 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9
```

The input consists of a series of grammar rules followed by a hash (`#`) symbol. Each rule is defined by an identifier (`ID`), followed by an arrow (`->`), a right-hand side consisting of zero or more IDs, and ends with a star (`*`). The input terminates with a `#` symbol.

Example:
```mathematica
decl -> idList colon ID *
idList -> ID idList1 *
idList1 -> *
idList1 -> COMMA ID idList1 *
#
```

Which represents the following grammar:
```mathematica
decl -> idList colon ID
idList -> ID idList1
idList1 -> ε
idList1 -> COMMA ID idList1
```

### Output Specifications

The output varies depending on the task specified through the command line argument:

- **Task 1**: Lists the terminals and non-terminals in their appearance order in the grammar rules.
- **Task 2**: Outputs the grammar with useless symbols removed.
- **Task 3**: Shows the FIRST sets for all non-terminals in the grammar.
- **Task 4**: Displays the FOLLOW sets for all non-terminals.
- **Task 5**: Indicates whether the grammar can be parsed by a predictive parser (`YES` or `NO`).

### Testing

This program includes a set of predefined test cases to validate each feature. To test a specific functionality, you can use the test script provided with the project. For example, to test task 3:

```bash
./test_p3.sh 3
```
Replace `3` with the appropriate task number you wish to test.
