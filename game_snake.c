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
void update_frame();
char* change_direction(int);
void reset_terminal_mode();
void set_terminal_mode();
int kbhit();
int get_key();
void update_position(char*);
void grow_up(struct Coordinates*, int);


int main() {
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigterm_handler);

    run_game();

    return 0;
}

void run_game() {
    set_terminal_mode();
    char* direction = "up";
    struct Coordinates* tail = (struct Coordinates*) malloc(225 * sizeof(struct Coordinates));
    int bait_eaten;
    head.x = GRID_SIZE / 2;
    head.y = GRID_SIZE / 2;
    struct Coordinates bait_position = create_bait();
    int tail_length = 0;
    while (head.x == bait_position.x && head.y == bait_position.y) {
        bait_position = create_bait();
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
            grow_up(tail, tail_length);
            tail_length++;
        }
        update_position(direction);
        
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
            if (i == head.x && j == head.y) {
                printf("O ");
            }
            else if (tail_length > 0) {
                for (int k = 0; k < tail_length; k++) {
                    if (i == tail[k].x && j == tail[k].y) {
                        printf("# ");
                    }
                }
            }
            else if (i == bait_position.x && j == bait_position.y) {
                printf("X ");
            }
            else {
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
    return direction;
}

void update_position(char* direction) {
    if (strcmp(direction, "up") == 0) {
        head.x -= 1;
    }
    if (strcmp(direction, "down") == 0) {
        head.x += 1;
    }
    if (strcmp(direction, "left") == 0) {
        head.y -= 1;
    }
    if (strcmp(direction, "right") == 0) {
        head.y += 1;
    }
    if (head.x < 0) {
        head.x += 1;
    }
    if (head.x > GRID_SIZE - 1) {
        head.x -= 1;
    }
    if (head.y < 0) {
        head.y += 1;
    }
    if (head.y > GRID_SIZE - 1) {
        head.y -= 1;
    }
}

struct Coordinates create_bait() {
    srand(time(NULL));
    int randX = rand() % 15;
    int randY = rand() % 15;
    struct Coordinates bait_position = {randX, randY};
    return bait_position;
}


void grow_up(struct Coordinates* tail, int tail_length) {
    for (int i = tail_length - 1; i > 0; i++) {
        tail[i + 1].x = tail[i].x;
        tail[i + 1].y = tail[i].y;
    }
    tail[0].x = head.x;
    tail[0].y = head.y;
}