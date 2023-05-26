#include "grid.h"
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>

const int grid_width = 30;
const int grid_height = 30;
static pthread_t **threads = NULL;

void freeThreadArray() {
    if (threads != NULL) {
        for (int y = 0; y < grid_height; y++) {
            free(threads[y]);
        }
        free(threads);
        threads = NULL;
    }
}

char *create_grid() {
    return malloc(sizeof(char) * grid_width * grid_height);
}

void destroy_grid(char *grid) {
    free(grid);
    freeThreadArray();
}




void draw_grid(char *grid) {
    for (int i = 0; i < grid_height; ++i) {
        // Two characters for more uniform spaces (vertical vs horizontal)
        for (int j = 0; j < grid_width; ++j) {
            if (grid[i * grid_width + j]) {
                mvprintw(i, j * 2, "■");
                mvprintw(i, j * 2 + 1, " ");
            } else {
                mvprintw(i, j * 2, " ");
                mvprintw(i, j * 2 + 1, " ");
            }
        }
    }

    refresh();
}

void init_grid(char *grid) {
    for (int i = 0; i < grid_width * grid_height; ++i)
        grid[i] = rand() % 2 == 0;
}

bool is_alive(int row, int col, char *grid) {

    int count = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) {
                continue;
            }
            int r = row + i;
            int c = col + j;
            if (r < 0 || r >= grid_height || c < 0 || c >= grid_width) {
                continue;
            }
            if (grid[grid_width * r + c]) {
                count++;
            }
        }
    }

    if (grid[row * grid_width + col]) {
        if (count == 2 || count == 3)
            return true;
        else
            return false;
    } else {
        if (count == 3)
            return true;
        else
            return false;
    }
}

void update_grid(char *src, char *dst) {
    for (int i = 0; i < grid_height; ++i) {
        for (int j = 0; j < grid_width; ++j) {
            dst[i * grid_width + j] = is_alive(i, j, src);
        }
    }
}

typedef struct {
    char *src;
    char *dst;
    int x;
    int y;
} CellThread;

void handler(int signo, siginfo_t *info, void *context) {}

void *updateCell(void *arg) {
    CellThread *now = (CellThread *) arg;

    while (1) {
        now->dst[now->x * grid_width + now->y] = is_alive(now->x, now->y, now->src);
        pause();
        char *tmp = now->src;
        now->src = now->dst;
        now->dst = tmp;
    }
}

void createMultiThread(char *src, char *dst){
    threads = (pthread_t **) malloc(grid_height * sizeof(pthread_t * ));
    for (int y = 0; y < grid_height; y++) {
        threads[y] = (pthread_t *) malloc(grid_width * sizeof(pthread_t));
    }

    for (int x = 0; x < grid_height; ++x) {
        for (int y = 0; y < grid_width; ++y) {
            CellThread *cell = malloc(sizeof(CellThread));
            cell->src = src;
            cell->dst = dst;
            cell->x = x;
            cell->y = y;

            pthread_create(&threads[x][y], NULL, updateCell, (void *) cell);
        }
    }
}


void updateMultiThread() {

    struct sigaction action;
    sigemptyset(&action.sa_mask);
    action.sa_sigaction = handler;
    sigaction(SIGUSR1, &action, NULL);

    for (int x = 0; x < grid_height; ++x) {
        for (int y = 0; y < grid_width; ++y) {
            pthread_kill(threads[x][y], SIGUSR1);
        }
    }
}
