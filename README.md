# Custom Shell in C++

## Project Description

This project is a simple shell (command-line interface) written in C++. It allows users to execute basic shell commands in an environment that mimics a Unix/Linux terminal. The shell supports standard commands like navigating directories, listing files, and running executable programs, as well as handling custom-built commands.

## Features

- **Basic shell commands**: 
  - Supports commands like `cd`, `ls`, `pwd`, and `exit`.
  - Execute system commands and programs.
  
- **Input parsing**: 
  - Tokenizes and processes user input to understand different commands.
  
- **Process management**: 
  - Supports both foreground and background process execution.
  - Uses system calls to fork processes and execute commands.
  
- **Custom built-in commands**:
  - `cd`: Change directories.
  - `pwd`: Print working directory.
  - `exit`: Exit the shell.

## Getting Started

### Prerequisites

- **C++ Compiler**: Ensure you have `g++` or another C++ compiler installed.
- **Linux/Unix environment**: The shell is designed to work on Linux-based systems.

### Installation

1. Clone the repository:
   ```bash
   git clone <repository-url>
