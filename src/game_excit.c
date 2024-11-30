#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>
#include<unistd.h>
#include<time.h>
#include<termios.h>
#include<fcntl.h>
#include<sys/select.h>

#define HEIGHT 20
#define WIDTH  22

struct Coordinates {
    int x;
    int y;
} player;

struct termios orig_termios;

// Function declarations
void signal_handler();
void reset_terminal_mode();
void set_terminal_mode();
int kbhit();
int get_key();
void handle_input();
void initialize_player();
void create_walls();
void change_direction(int);
struct Coordinates find_next_position();
void update_position();
void print_game_screen();

// Global variables
char* direction;
int moving;
int won = 0;
int exited = 0;
int walls[HEIGHT][WIDTH] = {0};

int main() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    set_terminal_mode();

    initialize_player();
    create_walls();
    
    while(1) {
        printf("\033[1J\033[H\033[?25l");
        
        if (exited == 1) {
            break;
        }
        
        handle_input();
        update_position(walls);
        print_game_screen(walls);
        if (won) {
            usleep(400000);
        }
        usleep(40000);
    }
    return 0;
}

void signal_handler() {
    exited = 1;
}

void reset_terminal_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

// Function to set terminal to non-canonical mode
void set_terminal_mode() {
    struct termios new_termios;

    // Save original terminal settings
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(reset_terminal_mode); // Ensure settings are restored on exit

    new_termios = orig_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);

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

void handle_input() {
    int key = get_key();
    if (key != -1) {
        if (key == 'q') {
            exited = 1;
        }
        change_direction(key);
    }
}

// reset player upon losing
void initialize_player() {
    player.x = 14;
    player.y = 11;
    moving = 0;
    direction = "";
}

void change_direction(int key) {
    if (!moving) {
        if (key == 'w') {
            direction = "up";
        }
        if (key == 's') {
            direction = "down";
        }
        if (key == 'a') {
            direction = "left";
        }
        if (key == 'd') {
            direction = "right";
        }
        moving = 1;
    }
}

struct Coordinates find_next_position() {
    struct Coordinates next_position;
    next_position.x = player.x;
    next_position.y = player.y;
    if (strcmp(direction, "up") == 0) {
        next_position.x -= 1;
    }
    if (strcmp(direction, "down") == 0) {
        next_position.x += 1;

    }
    if (strcmp(direction, "left") == 0) {
        next_position.y -= 1;
    }
    if (strcmp(direction, "right") == 0) {
        next_position.y += 1;
    }
    return next_position;
}

// moves player until it touches a wall or loses the game
void update_position() {
    struct Coordinates next_position = find_next_position();
    
    // game boundaries check
    if (next_position.x < 0 || next_position.x > HEIGHT - 1 || next_position.y < 0 || next_position.y > WIDTH - 1) {
        initialize_player();
    }
    else {
        // exit condition check
        if (player.x == 2 && player.y == 3) {
            moving = 0;
            won = 1;
            exited = 1;
        }

        // stops if hits a wall
        else if (moving && walls[next_position.x][next_position.y] == 1) {
            direction = "";
            moving = 0;
        }

        // wall collision check
        else {
            player.x = next_position.x;
            player.y = next_position.y;
        }
    }

}

// creates the predefined walls and exit
void create_walls() {
    walls[1][16]  = 1;
    walls[2][10]  = 1;
    walls[3][18]  = 1;
    walls[4][11]  = 1;
    walls[6][2]   = 1;
    walls[7][17]  = 1;
    walls[8][10]  = 1;
    walls[9][3]   = 1;
    walls[10][12] = 1;
    walls[11][13] = 1;
    walls[12][11] = 1;
    walls[13][2]  = 1;
    walls[14][9]  = 1;
    walls[14][13] = 1;
    walls[15][17] = 1;
    walls[16][11] = 1;
    walls[17][3]  = 1;
    walls[18][16] = 1;
    walls[2][3]   = 2; // exit
}

void print_game_screen() {
    printf("________________________\n");
    for (int i = 0; i < HEIGHT; i++) {
        printf("|");
        for (int j = 0; j < WIDTH; j++) {
            if (player.x == i && player.y == j) {
                if (won) {
                    printf("\033[42m\033[1m+\033[0m");
                }
                else {
                    printf("\033[1m+\033[0m");
                }
                continue;
            }
            if (walls[i][j] == 2) {
                printf("\033[42m \033[0m");
            }
            if (walls[i][j] == 1) {
                printf("\033[43m \033[0m");
            }
            if (walls[i][j] == 0) {
                printf(" ");
            }
            if (j == WIDTH - 1) {
                printf("|");
            }
        }
        printf("\n");
    }
    printf("------------------------\n");
}