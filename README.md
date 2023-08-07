# RemoteEnvLib

![License](https://img.shields.io/badge/license-MIT-blue)

RemoteEnvLib is a C++ library designed to monitor source code changes, transfer them to a remote FTP client, and execute Telnet scripts to rebuild the environment. It can be utilized via command-line tools or integrated into graphical user interfaces.

## Table of Contents
- [Features](#features)
- [Installation](#installation)

## Features
- **Source Monitoring:** Detection of source code changes.
- **FTP Integration:** Secure file transfers to remote servers.
- **Environment Rebuilding:** Automated execution of Telnet scripts for environment setup.

## Installation
### Dependencies
- CMake 3.10 or higher

### Building from Source
```bash
git clone https://github.com/username/RemoteEnvLib.git
cd RemoteEnvLib
mkdir build && cd build
cmake ..
make
make install
