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

struct termios orig_termios;


// function declarations
void signal_handler();
void reset_terminal_mode();
void set_terminal_mode();
int kbhit();
int get_key();
void initialize_game(struct Entity*);
void handle_input();
void shoot();
void update_bullet_position();
void check_game_over(struct Entity*);
void alien_shoot(struct Entity*, int*);
void update_alien_bullet_position();
void check_collision(struct Entity*);
void change_position(char);
void change_alien_position(struct Entity*, int*);
void update_alien_position(struct Entity*, char*);
void print_game_screen(struct Entity*);

// global variables
int player_pos;
int exited = 0;
struct Entity alien_bullet;


int main() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    set_terminal_mode();
    
    struct Entity *aliens = (struct Entity*) malloc(NUM_OF_ALIENS * (sizeof(struct Entity)));
    int *alien_move_counter = (int*) malloc(sizeof(int));
    *alien_move_counter = 0;
    int *alien_bullet_counter = (int*) malloc(sizeof(int));
    *alien_bullet_counter = 0;

    initialize_game(aliens);

    while(1) {
        *alien_move_counter += 1;
        *alien_bullet_counter += 1;
        printf("\033[1J\033[H\033[?25l");

        handle_input();

        if (exited != 0) {
            break;
        }

        if (bullet.alive == 1) {
            update_bullet_position();
            check_collision(aliens);
        }

        check_game_over(aliens);
        alien_shoot(aliens, alien_bullet_counter);

        if (alien_bullet.alive == 1) {
            update_alien_bullet_position();
        }

        change_alien_position(aliens, alien_move_counter);
        print_game_screen(aliens);

        usleep(40000);
    }

    // Deallocate the pointers to prevent memory leaks
    free(aliens);
    free(alien_move_counter);
    free(alien_bullet_counter);
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

void initialize_game(struct Entity* aliens) {
    int offset_x = 2;
    int offset_y = 3;
    player_pos = WIDTH / 2;
    for (int i = 0; i < NUM_OF_ALIENS; i++) {
        aliens[i].x = (i / 6) + offset_x;
        aliens[i].y = ((i % 6) * 2) - ((i / 6) % 2) + 2 + offset_y;
        aliens[i].alive = 1;    
    }
}

void handle_input() {
    int key = get_key();
    if (key != -1) {
        if (key == 'q') {
            exited = 1;
        }
        else if (key == 'w') {
            shoot();
        }
        else {
            change_position(key);
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

void check_game_over(struct Entity* aliens) {
    int all_dead = 1;
    for (int i = 0; i < NUM_OF_ALIENS; i++)  {
        if (aliens[i].alive == 1) {
            all_dead = 0;
        }
    }
    if (all_dead) {
        exited = 1;
    }
}

void alien_shoot(struct Entity* aliens, int* alien_bullet_counter) {
    if (!alien_bullet.alive && *alien_bullet_counter == 36 && !exited) {
        int random_alien;
        srand(time(NULL));
        do {
            random_alien = rand() % NUM_OF_ALIENS; 
        } while (aliens[random_alien].alive == 0);

        alien_bullet.x = aliens[random_alien].x + 1;
        alien_bullet.y = aliens[random_alien].y;
        alien_bullet.alive = 1;
        *alien_bullet_counter = 0;
    }
}

void update_alien_bullet_position() {
    if (alien_bullet.alive) {
        alien_bullet.x += 1;
        if (alien_bullet.x >= HEIGHT) {
            alien_bullet.alive = 0;
        } else if (alien_bullet.x == HEIGHT - 1 && alien_bullet.y == player_pos) {
            exited = 1; // if the bullet collides with the player, the game is over.
        }
    }
}

void check_collision(struct Entity* aliens) {
    for (int i = 0; i < NUM_OF_ALIENS; i++) {
        if (aliens[i].alive == 1 && aliens[i].x == bullet.x && aliens[i].y == bullet.y) {
            aliens[i].alive = 0;
            bullet.alive = 0;
            break;
        }
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

// Aliens move over time in a predefined sequence.
void change_alien_position(struct Entity* aliens, int* alien_move_counter) {
    switch (*alien_move_counter) {
        case 25:
        case 50:
        case 225:
        case 250:
            update_alien_position(aliens, "left");
            if (*alien_move_counter == 250) {
                *alien_move_counter = 0;
            }
            break;

        case 75:
            update_alien_position(aliens, "up");
            break;

        case 100:
        case 125:
        case 150:
        case 175:
            update_alien_position(aliens, "right");
            break;

        case 200:
            update_alien_position(aliens, "down");
            break;

        default:
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

void print_game_screen(struct Entity* aliens) {
    printf("______________________\n");
    for (int i = 0; i < HEIGHT; i++) {
        printf("|");
        for (int j = 0; j < WIDTH; j++) {
            int alien_exist = 0;
            for (int k = 0; k < NUM_OF_ALIENS; k++) {
                if (aliens[k].x == i && aliens[k].y == j && aliens[k].alive == 1) {
                    printf("\033[32m\u25BC\033[0m");
                    alien_exist = 1;
                }
            }
            if (!alien_exist) {
                if (i == HEIGHT - 1) {
                    if(player_pos == j) {
                        printf("\033[33m\u25B2\033[0m");
                    }
                    else {
                        printf(" ");
                    }
                }
                else if (bullet.alive == 1 && bullet.x == i && bullet.y == j) {
                    printf("\033[31m|\033[0m");
                }
                else if (alien_bullet.alive && alien_bullet.x == i && alien_bullet.y == j) {
                    printf("\033[31m|\033[0m");
                }
                else {
                    printf(" ");
                }
            }
        }
        printf("|\n");
    }
    printf("----------------------\n");
}