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

volatile sig_atomic_t exit_code = 0;
void sigint_handler();
void sigterm_handler();
void print_game_screen(struct Coordinates, struct Coordinates*, int);
void run_game();
struct Coordinates create_bait();
char* change_direction(int);
void reset_terminal_mode();
void set_terminal_mode();
int kbhit();
int get_key();
void update_position(char*, struct Coordinates*, int);
struct Coordinates find_next_position(char*);
int at_border = 0;


int main() {
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigterm_handler);

    run_game();

    return 0;
}

void run_game() {
    set_terminal_mode();
    char* direction = "up";
    struct Coordinates* tail = (struct Coordinates*) malloc(GRID_SIZE*GRID_SIZE * sizeof(struct Coordinates));
    int bait_eaten;
    head.x = GRID_SIZE / 2;
    head.y = GRID_SIZE / 2;
    struct Coordinates bait_position = create_bait();
    int tail_length = 0;
    while (1) {
        bait_position = create_bait();
        int overlap = 0;

        if (head.x == bait_position.x && head.y == bait_position.y) {
            overlap = 1;
        }
        
        for (int i = 0; i < tail_length; i++) {
            if (bait_position.x == tail[0].x && bait_position.y == tail[0].y) {
                overlap = 1;
            }
        }
        
        if (!overlap) {
            break;
        }
    }
    while(1) {
        // system("clear");
        printf("\033[1J\033[H\033[?25l");
        bait_eaten = 0;
        int key = get_key();
        if (key != -1) {
            if (key == 'q') {
                break;
            }
            direction = change_direction(key);
        }

        
        // if (exit_code != 0) {
        //     return exit_code;
        // }

        if (head.x == bait_position.x && head.y == bait_position.y) {
            bait_position = create_bait();
            tail_length++;
        }
        if (!at_border) {
            update_position(direction, tail, tail_length);
        }
        
        print_game_screen(bait_position, tail, tail_length);
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
    printf("signal handled");
    exit_code = 2;
}

void sigterm_handler()
{
    printf("process killed");
    exit_code = 15;
}

void print_game_screen(struct Coordinates bait_position, struct Coordinates* tail, int tail_length) {
    // printf("\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("%d %d\n", tail[1].x, tail[1].y);
    for (int i = 0; i < GRID_SIZE; i++) {
        // printf("\t\t\t\t\t\t\t\t\t\t");
        for (int j = 0; j < GRID_SIZE; j++) {
            int tail_found = 0;
            if (i == head.x && j == head.y) {
                printf("O ");
                continue;
            }
            for (int k = 0; k < tail_length; k++) {
                if (i == tail[k].x && j == tail[k].y) {
                    printf("# ");
                    tail_found = 1;
                    break;
                }
            }
            if (i == bait_position.x && j == bait_position.y) {
                printf("X ");
                continue;
            }
            if (!tail_found) {
                printf(". ");
            }
        }
        printf("\n");
    }
}



char* change_direction(int key) {
    char* direction = "";
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
    at_border = 0;
    return direction;
}

void update_position(char* direction, struct Coordinates* tail, int tail_length) {
    struct Coordinates next_position = find_next_position(direction);
    if (next_position.x < 0 || next_position.x > GRID_SIZE - 1 || next_position.y < 0 || next_position.y > GRID_SIZE - 1) {
        at_border = 1;
    }
    else {
        at_border = 0;
    }

    int self_collision = 0;
    for (int i = 0; i < tail_length; i++) {
        if (next_position.x == tail[i].x && next_position.y == tail[i].y) {
            self_collision = 1;
        }
    }
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

struct Coordinates find_next_position(char* direction) {
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

struct Coordinates create_bait() {
    srand(time(NULL));
    int randX = rand() % 15;
    int randY = rand() % 15;
    struct Coordinates bait_position = {randX, randY};
    return bait_position;
}
