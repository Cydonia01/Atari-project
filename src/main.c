#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>
#include<unistd.h>
#include<time.h>
#include<termios.h>
#include<fcntl.h>
#include<sys/select.h>

void signal_handler();
void print_game_screen();
int run_game();
void reset_terminal_mode();
void set_terminal_mode();
int kbhit();
int get_key();
void print_header();
void handle_input(int);
int selected_option = 0;
int selected_game = 0;
char* games[] = {"game_snake", "game_space"};
int game_running = 0;
int exited = 0;


int main() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    set_terminal_mode();

    while(1) {
        printf("\033[1J\033[H\033[?25l");
        int key = get_key();
        if (key != -1) {
            if (key == 'q') {
                exited = 1;
            }
            else {
                handle_input(key);
            }
        }
        
        if (game_running == 1) {
            char path[] = "../bin/";
            strcat(path, games[selected_game]);
            int run_status = system(path);
            game_running = 0;
        }
        
        if (exited != 0) {
            exit(0);
        }
        

        print_game_screen();
        usleep(60000);
    }

    return 0;
}

void print_header() {
    printf("   ###    #        #######  #     #  #######  #######    ###    #######  #######  ########  ##      #\n");
    printf("  #   #   #        #         #   #   #           #      #   #      #        #     #      #  # #     #\n");
    printf(" #     #  #        #          # #    #           #     #     #     #        #     #      #  #  #    #\n");
    printf(" #######  #        ######      #     #######     #     #######     #        #     #      #  #   #   #\n");
    printf(" #     #  #        #           #           #     #     #     #     #        #     #      #  #    #  #\n");
    printf(" #     #  #        #           #           #     #     #     #     #        #     #      #  #     # #\n");
    printf(" #     #  #######  #######     #     #######     #     #     #     #     #######  ########  #      ##\n");
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

void signal_handler()
{
    if (game_running) {
        char command[256];
        snprintf(command, sizeof(command), 
        "ps -e | grep %s | grep -v grep | awk '{print $1}' | while read pid; do kill $pid; done", games[selected_game]);
        system(command);
    }
    exited = 1;
}
void print_game_screen() {
    print_header();

    for (int i = 0; i < 2; i++) {
        if (selected_option == i) {
            printf("\033[42m");
        }
        if (i == 0) {
            printf("%s\t\t", games[selected_game]);
        }
        if (i == 1) {
            printf("%s", "exit");
        }
        printf("\033[0m");
    }
    printf("\n");

}


void handle_input(int key) {
    if (key == 'w' && selected_option == 0) {
        if (selected_game == 0) {
            selected_game = 1;
        }
        else {
            selected_game = (selected_game - 1) % 2;
        }
    }
    if (key == 's' && selected_option == 0) {
        selected_game = (selected_game + 1) % 2;
    }
    if (key == 'a') {
        if (selected_option > 0) {
            selected_option -= 1;
        }
    }
    if (key == 'd') {
        if (selected_option < 1) {
            selected_option += 1;
        }
    }
    if (key == ' ') {
        if (selected_option == 0) {
            game_running = 1;
        }
        if (selected_option == 1) {
            exited = 1;
        }
    }
}