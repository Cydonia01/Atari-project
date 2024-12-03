# Virtual Video Game Console
## Project Overview
The **Virtual Video Game Console** is a shell-based console that runs games written in the **C programming language.** It simulates a classic video game console with the following features:
- A **main menu** to select and launch games.
- **Interactive games** controlled via the keyboard.
- A **virtual disk system** for managing game binaries, enabling plug-and-play functionality.

The console is designed to gracefully handle signals like `SIGINT` and `SIGTERM` and includes shell scripts to manage the disk image.

## Features
### Games
- Each game is a standalone C program.
- Controlled using the `w`, `a`, `s`, `d` keys.
- Universal **exit** key: `q`.
- Smooth exit procedure for all games, including signal handling for `SIGINT` and `SIGTERM`.

There are currently three games available:
- **Snake Game**: A classic snake game  where the player controls a snake to eat food and grow. Hitting walls or itself does not end the game.
- **Alien Invasion**: A space shooter game where the player controls a spaceship to shoot down aliens. The game ends when the player's spaceship is hit by an alien.
- **Excit**: A puzzle game where the player moves an agent to reach the exit. If the agent goes out of bounds, the game restarts.

### Console Main Screen
- Displays a list of available games and an exit button.
- Navigate with `w`, `a`, `s`, `d` keys, and select a game with `Enter`.
- Exits gracefully with `q` or upon receiving `SIGINT/SIGTERM`.
- Simple and intuitive interface for game selection.

### Disk Device
- Implements a **plug-and-play virtual disk system**.
- Includes scripts to create, mount, unmount, and delete a disk image (`storage_vgc.img`).
- Games and the main menu are stored within this disk image.

## Getting Started
### Prerequisites
- Debian 12.8.0 or equivalent Linux environment.
- C compiler
- Termios for input handling.

### Setup
1. Update the package manager and install the required packages:
```bash
sudo apt update
sudo apt install gcc build-essential libc-dev
```
2. Run make to compile all programs:
```bash
make
```
3. Navigate to bin directory and run the main screen program to start the console:
```bash
cd bin
./main-screen
```

## Scripts
- **initialize.sh**: Creates the virtual disk image. Overwrites existing images.
- **startup.sh**: Mounts the virtual disk image, creates a symbolic link to the device file, and prepares the environment.
- **terminate**.sh: Unmounts the virtual disk image, detaches the device file, and removes the symbolic link.
- **purge.sh**: Does the same things as in **terminate.sh**, but also deletes the virtual disk image.
