![License](https://img.shields.io/badge/license-MIT-blue)

# RemoteEnvTool

RemoteEnvTool is a powerful command-line tool designed to monitor local source code changes, transfer them to remote FTP servers, and execute Telnet scripts to rebuild environments. It comes with an interactive mode, providing a user-friendly way to manage and synchronize code across different environments.

## Table of Contents
- [Features](#features)
- [Configuration](#configuration)
- [Usage](#usage)
- [Interactive Mode](#interactive-mode)
- [Installation](#installation)

## Features
- **Source Monitoring:** Real-time detection of local source code modifications via github status parsing.
- **FTP Integration:** Securely transfer updated source files to remote servers.
- **Telnet Execution:** Automated Telnet script execution to set up and rebuild environments.
- **Interactive Mode:** Engaging command-line interface for managing and synchronizing code.

## Configuration
RemoteEnvTool uses a configuration file to determine various operational parameters:

- `DEFAULT_HOST`: Specifies the default host the tool should connect to if no host is provided in command arguments.
- `LOCAL_PATH`: Defines the path on your local machine where the source code resides.
- `DIFFTOOL`: Specifies the tool to be used for showing differences in code.

For each remote environment you want to manage, define a host configuration:

- `HOST`: Name or IP of the remote server.
- `USERNAME`: FTP username for accessing the remote server.
- `PASSWORD`: FTP password for the above username.
- `REMOTE_PATH`: Directory path on the remote server where the code should be transferred.
- `SCRIPT`: Telnet script that the tool should run after connecting for the first time. ('.' dot will be added)
- `PORT`: Port on which Telnet service runs, default is 23.

:warning: Paths have to be without any whitespaces.

Configuration file example:

```
DEFAULT_HOST: devcoo69
LOCAL_PATH: C:\User\Desktop\work
DIFFTOOL: meld

HOST: devcoo69
USERNAME: username
PASSWORD: password
REMOTE_PATH: /home/work
SCRIPT: SCRIPT.sh
PORT: 23

and hosts so on...
```

> **Security Concern**: The host data, including passwords, is in plain text in the configuration file. This setup is intended for environments with restricted external access or controlled development settings.

## Usage
```
RemoveEnvTool --help
```
- `--help`: Display help message.
- `--host [HOST_NAME]`: Set host for connection. Uses default from config if not provided.
- `--interactive`: Launch in interactive mode.
- `--list-file`: List locally changed files.
- `--transfer [TYPE]`: Send files to host. Types: `added`, `deleted`, `updated`, `all`.
- `--transfer-branch [BRANCH_NAME]`: List and send files modified between current branch and the specified branch.
- `--script [SCRIPT_NAME]`: Execute telnet script (prefix with a dot).
- `--restart [TARGET]`: Restart target. Options: `env` (whole domain), `retux` (adapter), or `SERV-NAME` (specific server).
- `--tlog [FILENAME]`: Log output to a file. Uses current date as filename if not provided.
- `--no-difftool`: Skip using the difftool during file transfer.

## Interactive-mode
```
RemoteEnvTool --interactive
```
![image](https://github.com/Nelnir/RemoteEnvLib/assets/22370292/6f99e506-5a55-4174-bfcb-6d92945ee8b0)

In this mode, you'll be guided through a series of prompts to manage and synchronize your source code. The available options are:

- **List Changed Files**: Display a list of files that have changed in your local directory.
- **Transfer Changed Files**: Transfer the modified files to the designated remote host.
- **Change Default Host**: Modify the default host as set in your configuration.
- **Restart Command**: Restart a specific target, such as the entire environment, a particular adapter, or a designated server.
- **Tlog Command**: Start logging output to a designated file or, if no filename is provided, use the current date as the filename.
- **Script Command**: Execute a specified Telnet script on the remote server.
- **Transfer Branch Command**: List and transfer all files that have been modified between the current branch and a specified branch.

## Installation
### Dependencies
- CMake 3.10 or higher
- [SFML](https://github.com/SFML/SFML)

### Building from Source
```bash
git clone https://github.com/username/RemoteEnvTool.git
cd RemoteEnvTool
mkdir build && cd build
cmake ..
```

Happy using it, feel free to make any changes :)
