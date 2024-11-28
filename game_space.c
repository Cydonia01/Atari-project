#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>
#include<unistd.h>
#include<time.h>
#include<termios.h>
#include<fcntl.h>
#include<sys/select.h>

#define WIDTH 20
#define HEIGHT 15
#define NUM_OF_ALIENS 24

struct Entity {
    int x;
    int y;
    int alive;
} bullet;

volatile sig_atomic_t exit_code = 0;
void sigint_handler();
void sigterm_handler();
void print_game_screen(struct Entity*);
void run_game();
void reset_terminal_mode();
void set_terminal_mode();
int kbhit();
int get_key();
void update_alien_position(struct Entity*, char*);
void change_position(char);
void shoot();
void check_collision(struct Entity*);
void update_bullet_position();
void change_alien_position(struct Entity*, int*);

int player_pos = WIDTH / 2;


int main() {
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigterm_handler);

    run_game();

    return 0;
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

void run_game() {
    set_terminal_mode();
    
    struct Entity *aliens = (struct Entity*) malloc(NUM_OF_ALIENS * (sizeof(struct Entity)));
    int offset_x = 2;
    int offset_y = 3;
    for (int i = 0; i < NUM_OF_ALIENS; i++) {
        aliens[i].x = (i / 6) + offset_x;
        aliens[i].y = ((i % 6) * 2) - ((i / 6) % 2) + 2 + offset_y;
        aliens[i].alive = 1;    
    }
    int *counter = (int*) malloc(sizeof(int));
    *counter = 0;
    while(1) {
        *counter += 1;
        int game_over = 1;
        printf("\033[1J\033[H\033[?25l");
        int key = get_key();
        if (key != -1) {
            if (key == 'q') {
                break;
            }
            else if (key == 'w') {
                shoot();
            }
            else {
                change_position(key);
            }
        }

        // if (exit_code != 0) {
        //     return exit_code;
        // }

        if (bullet.alive == 1) {
            update_bullet_position();
            check_collision(aliens);
        }
        change_alien_position(aliens, counter);
        print_game_screen(aliens);

        usleep(40000);
    }
}

void print_game_screen(struct Entity* aliens) {
    // printf("\n\n\n\n\n\n\n\n\n\n\n\n\n");
    for (int i = 0; i < HEIGHT; i++) {
        // printf("\t\t\t\t\t\t\t\t\t\t");
        for (int j = 0; j < WIDTH; j++) {
            int alien_exist = 0;
            for (int k = 0; k < NUM_OF_ALIENS; k++) {
                if (aliens[k].x == i && aliens[k].y == j && aliens[k].alive == 1) {
                    printf("\u25BC");
                    alien_exist = 1;
                }
            }
            if (!alien_exist) {
                if (i == HEIGHT - 1) {
                    if(player_pos == j) {
                        printf("\u25B2");
                    }
                    else {
                        printf(" ");
                    }
                }
                else if (bullet.alive == 1 && bullet.x == i && bullet.y == j) {
                    printf("|");
                }
                else {
                    printf(" ");
                }
            }
        }
        printf("\n");
    }
}

void change_position(char key) {
    if (key == 'd' && player_pos < WIDTH - 1) {
        player_pos += 1;
    }
    if (key == 'a' && player_pos > 0) {
        player_pos -= 1;
    }
}

void change_alien_position(struct Entity* aliens, int* counter) {
    switch (*counter) {
        case 25: // 32
        case 50: // 64
        case 225: // 320
        case 250: // 352
            update_alien_position(aliens, "left");
            if (*counter == 250) { // Reset counter when it reaches 352 (11 * 32)
                *counter = 0;
            }
            break;

        case 75: // 96
            update_alien_position(aliens, "up");
            break;

        case 100: // 128
        case 125: // 160
        case 150: // 192
        case 175: // 224
            update_alien_position(aliens, "right");
            break;

        case 200: // 288
            update_alien_position(aliens, "down");
            break;

        default:
            // No action for other counter values
            break;
    }
}

void update_alien_position(struct Entity* aliens, char* direction) {
    if (strcmp(direction, "left") == 0) {
        for (int i = 0; i < NUM_OF_ALIENS; i++) {
            aliens[i].y -= 1;
        }
    }
    if (strcmp(direction, "right") == 0) {
        for (int i = 0; i < NUM_OF_ALIENS; i++) {
            aliens[i].y += 1;
        }
    }
    if (strcmp(direction, "up") == 0) {
        for (int i = 0; i < NUM_OF_ALIENS; i++) {
            aliens[i].x -= 1;
        }
    }
    if (strcmp(direction, "down") == 0) {
        for (int i = 0; i < NUM_OF_ALIENS; i++) {
            aliens[i].x += 1;
        }
    }
}

void shoot() {
    if (!bullet.alive) {
        bullet.x = HEIGHT - 1;
        bullet.y = player_pos;
        bullet.alive = 1;
    }
}

void update_bullet_position() {
    bullet.x -= 1;
    if (bullet.x < 0) {
        bullet.alive = 0;
    }
}

void check_collision(struct Entity* aliens) {
    for (int i = 0; i < NUM_OF_ALIENS; i++) {
        // printf("alive: %d\nalienx: %d\n alieny: %d\nbulletx: %d\n bullety %d\n", aliens[i].alive, aliens[i].x, aliens[i].y, bullet.x, bullet.y);
        if (aliens[i].alive == 1 && aliens[i].x == bullet.x && aliens[i].y == bullet.y) {
            aliens[i].alive = 0;
            bullet.alive = 0;
            break;
        }
    }
}