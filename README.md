# MiniShell: A UNIX-Like Command Line Interpreter

## Overview

MiniShell is a lightweight UNIX-like shell built in C, designed to execute commands, manage processes, and support inter-process communication. 

## Features

- **Command Execution** – Runs standard UNIX commands using `fork()` and `execvp()`.
- **Background Process Support** – Enables commands to run in the background using `&`.
- **Input & Output Redirection** – Implements file redirection with `<` and `>`.
- **Piping (`|`)** – Supports simple and multiple pipes for inter-process communication.
- **Process Management** – Lists and tracks background processes.

## Technologies Used

- **C** (GCC Compiler)
- **Linux System Calls**: `fork()`, `execvp()`, `wait()`, `pipe()`, `dup2()`, `open()`, `close()`
- **POSIX APIs**

## Compiling and Running the Shell

1. **Compile the Shell**

   ```bash
   cmake Cmakelists.txt
   make

2. **Run the Shell**
   ```bash
   ./unix_shell
