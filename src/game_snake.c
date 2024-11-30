#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>
#include<unistd.h>
#include<time.h>
#include<termios.h>
#include<fcntl.h>
#include<sys/select.h>

#define GRID_SIZE 15

struct Coordinates {
    int x;
    int y;
} head;

struct termios orig_termios;

// Function declarations
void signal_handler();
void reset_terminal_mode();
void set_terminal_mode();
int kbhit();
int get_key();
void initialize_player();
void handle_input();
void create_bait(struct Coordinates*);
void change_direction(int);
struct Coordinates find_next_position();
void update_position(struct Coordinates*);
void check_bait_eaten(struct Coordinates*);
void print_game_screen(struct Coordinates*);

// Global variables
int at_border;
char* direction;
int bait_eaten;
struct Coordinates bait_position;
int tail_length;
int exited = 0;

int main() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    set_terminal_mode();

    struct Coordinates* tail = (struct Coordinates*) malloc(GRID_SIZE*GRID_SIZE * sizeof(struct Coordinates));
    initialize_player();

    create_bait(tail);        
    while(1) {
        printf("\033[1J\033[H\033[?25l");
        
        handle_input();
        if (exited == 1) {
            break;
        }
        update_position(tail);
        check_bait_eaten(tail);
        print_game_screen(tail);
        usleep(120000);
    }
    // Deallocate the pointer to prevent memory leaks
    free(tail);
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
        else {
            change_direction(key);
        }
    }
}

void initialize_player() {
    head.x = GRID_SIZE / 2;
    head.y = GRID_SIZE / 2;
    direction = "up";
    tail_length = 0;
    bait_eaten = 0;
    at_border = 0;
}

void create_bait(struct Coordinates* tail) {
    srand(time(NULL));
    while (1) {
        int randX = rand() % 15;
        int randY = rand() % 15;
        bait_position.x = randX;
        bait_position.y = randY;

        // check if the bait is created at the position of the head
        if (head.x == bait_position.x && head.y == bait_position.y) {
            continue;
        }
        
        // check if the bait is created at the position of the tail
        for (int i = 0; i < tail_length; i++) {
            if (bait_position.x == tail[i].x && bait_position.y == tail[i].y) {
                continue;
            }
        }
        break;
    }
}

void change_direction(int key) {
    if (key == 'w') {
        if (strcmp(direction, "down") != 0) {
            direction = "up";
        }
    }
    if (key == 's') {
        if (strcmp(direction, "up") != 0) {
            direction = "down";
        }
    }
    if (key == 'a') {
        if (strcmp(direction, "right") != 0) {
            direction = "left";
        }
    }
    if (key == 'd') {
        if (strcmp(direction, "left") != 0) {
            direction = "right";
        }
    }
    at_border = 0;
}

struct Coordinates find_next_position() {
    struct Coordinates next_position;
    next_position.x = head.x;
    next_position.y = head.y;
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

void update_position(struct Coordinates* tail) {
        struct Coordinates next_position = find_next_position(direction);
        if (next_position.x < 0 || next_position.x > GRID_SIZE - 1 || next_position.y < 0 || next_position.y > GRID_SIZE - 1) {
            at_border = 1;
        }

        int self_collision = 0;
        for (int i = 0; i < tail_length; i++) {
            if (next_position.x == tail[i].x && next_position.y == tail[i].y) {
                self_collision = 1;
            }
        }

        // move snake if it not going into a border and not colliding with itself
        if (!at_border && !self_collision) {
            for (int i = tail_length - 1; i > 0; i--) {
                tail[i].x = tail[i - 1].x;
                tail[i].y = tail[i - 1].y;
            }
            tail[0].x = head.x;
            tail[0].y = head.y;
            head.x = next_position.x;
            head.y = next_position.y;
        }
}

void check_bait_eaten(struct Coordinates *tail) {
    if (head.x == bait_position.x && head.y == bait_position.y) {
        create_bait(tail);

        if (tail_length > 0) {
            tail[tail_length].x = tail[tail_length - 1].x;
            tail[tail_length].y = tail[tail_length - 1].y;
        } else {
            tail[tail_length].x = head.x;
            tail[tail_length].y = head.y;
        }

        tail_length++;
    }
}

void print_game_screen(struct Coordinates *tail)
{
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            int tail_found = 0;
            if (i == head.x && j == head.y) {
                printf("\033[93mO \033[0m");
                continue;
            }

            for (int k = 0; k < tail_length; k++) {
                if (i == tail[k].x && j == tail[k].y) {
                    printf("\033[34m# \033[0m");
                    tail_found = 1;
                    break;
                }
            }

            if (i == bait_position.x && j == bait_position.y) {
                printf("\033[31mX \033[0m");
                continue;
            }
            
            if (!tail_found) {
                printf(". ");
            }
        }
        printf("\n");
    }
}