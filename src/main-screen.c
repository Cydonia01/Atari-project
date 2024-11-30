#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>
#include<unistd.h>
#include<time.h>
#include<termios.h>
#include<fcntl.h>
#include<sys/select.h>
#include<dirent.h>
#include<sys/stat.h>

// function declarations
void signal_handler();
void reset_terminal_mode();
void set_terminal_mode();
int kbhit();
int get_key();
void scan_directory();
void handle_input();
void print_header();
void print_game_screen();
void cleanup();

// global variables
int selected_option = 0;
int selected_game = 0;
char* games[3];
int game_running = 0;
int exited = 0;
struct termios orig_termios;

int main() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    set_terminal_mode();
    scan_directory();

    while(1) {
        printf("\033[1J\033[H\033[?25l");
        handle_input();
        
        if (game_running == 1) {
            char path[] = "./";
            strcat(path, games[selected_game]);
            int run_status = system(path);
            game_running = 0;
        }
        
        if (exited == 1) {
            break;
        }
        
        print_game_screen();
        usleep(60000);
    }
    void cleanup();

    return 0;
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

void scan_directory() {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    int game_count = 0;
    const char *prefix = "game_";

    dir = opendir(".");

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        if (strncmp(entry->d_name, prefix, strlen(prefix)) == 0) {
            if (stat(entry->d_name, &file_stat) == -1) {
                perror("stat");
                continue;
            }
            if (S_ISREG(file_stat.st_mode) && (file_stat.st_mode & S_IXUSR)) {
                games[game_count] = strdup(entry->d_name);
                game_count++;
            }
        }
    }
    closedir(dir);
}

void handle_input() {
    int key = get_key();
    if (key != -1) {
        if (key == 'q') {
            exited = 1;
        }

        if (key == 'w' && selected_option == 0) {
            if (selected_game == 0) {
                selected_game = 2;
            }
            else {
                selected_game = (selected_game - 1) % 3;
            }
        }

        if (key == 's' && selected_option == 0) {
            selected_game = (selected_game + 1) % 3;
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

        if (key == '\n') {
            if (selected_option == 0) {
                game_running = 1;
            }
            if (selected_option == 1) {
                exited = 1;
            }
        }
    }
}

void print_header() {
    printf("____________________________________________________________________________________________________________\n");
    printf("|\t\t\t\t\t\t\t\t\t\t\t\t\t   |\n");
    printf("|\033[32m     ###    #        #######  #     #  #######  #######    ###    #######  #######  ########  ##      #\033[0m   |\n");
    printf("|\033[32m    #   #   #        #         #   #   #           #      #   #      #        #     #      #  # #     #\033[0m   |\n");
    printf("|\033[32m   #     #  #        #          # #    #           #     #     #     #        #     #      #  #  #    #\033[0m   |\n");
    printf("|\033[32m   #######  #        ######      #     #######     #     #######     #        #     #      #  #   #   #\033[0m   |\n");
    printf("|\033[32m   #     #  #        #           #           #     #     #     #     #        #     #      #  #    #  #\033[0m   |\n");
    printf("|\033[32m   #     #  #        #           #           #     #     #     #     #        #     #      #  #     # #\033[0m   |\n");
    printf("|\033[32m   #     #  #######  #######     #     #######     #     #     #     #     #######  ########  #      ##\033[0m   |\n");
}

void print_game_screen() {
    print_header();
    for (int i = 0; i < 4; i++) {
        printf("|\t\t\t\t\t\t\t\t\t\t\t\t\t   |\n");
    }
    printf("|\t\t\t\t\tWelcome to ALEYSTATION\t\t\t\t\t\t   |\n|\t\t\t\t\t Press space to start\t\t\t\t\t\t   |\n");
    printf("|\t\t\t\t\t\t\t\t\t\t\t\t\t   |\n");
    printf("|                                 ");
    for (int i = 0; i < 2; i++) {
        if (selected_option == i) {
            printf("\033[45m");
        }
        if (i == 0) {
            printf("%s\t\t\t", games[selected_game]);
        }
        if (i == 1) {
            printf("%s", "exit");
        }
        printf("\033[0m");
    }
    printf("\t\t\t\t\t   |\n");
    printf("|\t\t\t\t\t\t\t\t\t\t\t\t\t   |\n");
    printf("|\t\t\t\t\t\t\t\t\t\t\t\t\t   |\n");
    printf("|__________________________________________________________________________________________________________|\n");

}

void cleanup() {
    for (int i = 0; i < 3; i++) {
        free(games[i]);
    }
}