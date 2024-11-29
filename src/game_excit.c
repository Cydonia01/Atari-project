#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>
#include<unistd.h>
#include<time.h>
#include<termios.h>
#include<fcntl.h>
#include<sys/select.h>


#define GRID_SIZE 40
#define NUM_OF_WALLS 10

struct Coordinates {
    int x;
    int y;
} player;

volatile sig_atomic_t exit_code = 0;
void sigint_handler();
void sigterm_handler();
void print_game_screen(char[][GRID_SIZE]);
int run_game();
void create_walls(char[][GRID_SIZE]);
void change_direction(int);
void reset_terminal_mode();
void set_terminal_mode();
int kbhit();
int get_key();
void update_position(char*, struct Coordinates*, int);
struct Coordinates find_next_position(char*);
int at_border = 0;
char* direction = "up";


int main() {
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigterm_handler);

    int run_status = run_game();
    if (run_status != 0) {
        return run_status;
    }
    return 0;
}

int run_game() {
    set_terminal_mode();
    player.x = GRID_SIZE / 2;
    player.y = GRID_SIZE / 2;
    char walls[GRID_SIZE][GRID_SIZE] = {0};
    create_walls(walls);
    while(1) {
        // system("clear");
        printf("\033[1J\033[H\033[?25l");
        int key = get_key();
        if (key != -1) {
            if (key == 'q') {
                break;
            }
            // change_direction(key);
        }
        
        if (exit_code != 0) {
            return exit_code;
        }

        // if (head.x == bait_position.x && head.y == bait_position.y) {
        //     bait_position = create_bait();
        //     tail_length++;
        // }
        // if (!at_border) {
        //     update_position(direction, tail, tail_length);
        // }
        
        print_game_screen(walls);
        usleep(120000);
    }
}

struct termios orig_termios;

// Function to restore terminal settings
void reset_terminal_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

// Function to set terminal to non-canonical mode
void set_terminal_mode() {
    struct termios new_termios;

    // Save original terminal settings
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(reset_terminal_mode); // Ensure settings are restored on exit

    // Copy original settings to modify
    new_termios = orig_termios;

    // Set terminal to non-canonical mode and disable echo
    new_termios.c_lflag &= ~(ICANON | ECHO);

    // Apply new settings
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
}

// Function to check if a key is pressed
int kbhit() {
    struct timeval tv = {0, 0}; // No waiting
    fd_set fds;

    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);

    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
}

int get_key() {
    int ch;
    if (kbhit()) {
        ch = getchar();
        return ch;
    }
    return -1;
}

void sigint_handler()
{
    exit_code = 2;
}

void sigterm_handler()
{
    exit_code = 15;
}

void print_game_screen(char walls[][GRID_SIZE]) {
    // printf("\n\n\n\n\n\n\n\n\n\n\n\n\n");
    for (int i = 0; i < GRID_SIZE; i++) {
        // printf("\t\t\t\t\t\t\t\t\t\t");
        for (int j = 0; j < GRID_SIZE; j++) {
            if (walls[i][j] == '|') {
                printf("| ");
            }
            else if (walls[i][j] == '_') {
                if (walls[i][j + 1] == '_') {
                    printf("__");
                }
                else {
                    printf("_ ");
                }
            }
            else {
                printf(". ");
            }
        }
        printf("\n");
    }
}

// void change_direction(int key) {
//     if (key == 'w') {
//         direction = "up";
//     }
//     if (key == 's') {
//         direction = "down";
//     }
//     if (key == 'a') {
//         direction = "left";
//     }
//     if (key == 'd') {
//         direction = "right";
//     }
//     at_border = 0;
// }

// void update_position(char* direction, struct Coordinates* tail, int tail_length) {
//     struct Coordinates next_position = find_next_position(direction);
//     if (next_position.x < 0 || next_position.x > GRID_SIZE - 1 || next_position.y < 0 || next_position.y > GRID_SIZE - 1) {
//         at_border = 1;
//     }
//     else {
//         at_border = 0;
//     }

//     int self_collision = 0;
//     for (int i = 0; i < tail_length; i++) {
//         if (next_position.x == tail[i].x && next_position.y == tail[i].y) {
//             self_collision = 1;
//         }
//     }
//     if (!at_border && !self_collision) {
//         for (int i = tail_length - 1; i > 0; i--) {
//             tail[i].x = tail[i - 1].x;
//             tail[i].y = tail[i - 1].y;
//         }
//         tail[0].x = head.x;
//         tail[0].y = head.y;
//         head.x = next_position.x;
//         head.y = next_position.y;
//     }
// }

// struct Coordinates find_next_position(char* direction) {
//     struct Coordinates next_position;
//     next_position.x = head.x;
//     next_position.y = head.y;
//     if (strcmp(direction, "up") == 0) {
//         next_position.x -= 1;
//     }
//     if (strcmp(direction, "down") == 0) {
//         next_position.x += 1;

//     }
//     if (strcmp(direction, "left") == 0) {
//         next_position.y -= 1;
//     }
//     if (strcmp(direction, "right") == 0) {
//         next_position.y += 1;
//     }
//     return next_position;
// }

void create_walls(char walls[][GRID_SIZE]) {
    srand(time(NULL));
    for (int i = 0; i < NUM_OF_WALLS; i++) {
        int length = rand() % 15 + 1;
        int randX = rand() % GRID_SIZE;
        int randY = rand() % GRID_SIZE;
        int direction = rand() % 2;
        if (direction == 0) { // hotizontal
            if (randY + length > GRID_SIZE) {
                length = GRID_SIZE - randY;
            }
            for (int j = 0; j < length; j++) {
                walls[randX][randY + j] = '_';
            }
        } else { // vertical
            if (randX + length > GRID_SIZE) {
                length = GRID_SIZE - randX;
            }
            for (int j = 0; j < length; j++) {
                walls[randX + j][randY] = '|';
            }
        }
    }
}
