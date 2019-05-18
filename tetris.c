#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include "tetris.h"

/*------------------------------- global variables -------------------------------*/

struct tetris_block blocks[] =
{
    {{"##", "##"}, 2, 2},
    {{" X ","XXX"}, 3, 2},
    {{"@@@@"}, 4, 1},
    {{"OO", "O ", "O "}, 2, 3},
    {{"&&", " &", " &"}, 2, 3},
    {{"ZZ ",  " ZZ"}, 3, 2}
};

struct tetris_level levels[]=
{
    {0, 1200000},
    {1500, 900000},
    {8000, 700000},
    {20000, 500000},
    {40000, 400000},
    {75000, 300000},
    {100000, 200000}
};

struct termios save;


/*------------------------------- macro definitions -------------------------------*/

#define TETRIS_PIECES (sizeof(blocks) / sizeof(struct tetris_block))
#define TETRIS_LEVELS (sizeof(levels) / sizeof(struct tetris_level))

#define TIME_ZERO 0
#define TIME_COUNT_NANO 1000000

#define EMPTY_CHAR ' '

#define MOVE_L 'a'
#define MOVE_R 'd'
#define MOVE_D 's'

#define BASE 0

#define SMALL_COUNT_LIMIT 50
#define LARGE_COUNT_LIMIT 350

/*------------------------------- functions ---------------------------------------*/

void tetris_cleanup_io() {
    tcsetattr(fileno(stdin), TCSANOW, &save);
}

/**
 * Quit the tetris game if the signal comes in.
 */
void tetris_signal_quit(int s) {
    tetris_cleanup_io();
}

void tetris_set_ioconfig() {
    struct termios custom;
    int fd = fileno(stdin);
    tcgetattr(fd, &save);
    custom = save;
    custom.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(fd, TCSANOW, &custom);
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

/**
 * Init the game.
 *
 * @param {t} the tetris block object.
 * @param {w} width
 * @param {h} height
 */
void tetris_init(struct tetris *t, int w, int h) {
    int x, y;
    t->level = 1;
    t->score = 0;
    t->gameover = 0;
    t->w = w;
    t->h = h;
    t->game = malloc(sizeof(char *) * w);

    for (x = 0; x < w; x++) {
        t->game[x] = malloc(sizeof(char) * h);

        for (y = 0; y < h; y++) {
            t->game[x][y] = EMPTY_CHAR;
        }
    }
}

void tetris_clean(struct tetris *t) {
    int x;

    for (x = 0; x < t->w; x++) {
        free(t->game[x]);
    }
    free(t->game);
}

void tetris_print(struct tetris *t) {
    int x, y;

    for (x = 0; x < 30; x++)
        printf("\n");

    printf("[LEVEL: %d | SCORE: %d]\n", t->level, t->score);

    for (x = 0; x < 2 * t->w + 2; x++)
        printf("~");
    printf("\n");

    for (y = 0; y < t->h; y++) {
        printf("!");

        for (x = 0; x < t->w; x++) {

            if (x >= t->x && y >= t->y && x < (t->x + t->current.w) && y < (t->y + t->current.h) && t->current.data[y - t->y][x - t->x] != EMPTY_CHAR)
                printf("%c ", t->current.data[y - t->y][x - t->x]);
            else
                printf("%c ", t->game[x][y]);
        }

        printf("!\n");

    }

    for (x = 0; x < 2 * t->w + 2; x++)
        printf("~");

    printf("\n");

}

int tetris_hittest(struct tetris *t) {
    int x, y, X, Y;
    struct tetris_block b = t->current;

    for (x = 0; x < b.w; x++) {

        for (y = 0; y < b.h; y++) {
            X = t->x + x;
            Y = t->y + y;

            if (X < 0 || X >= t->w)
                return 1;

            if (b.data[y][x] != EMPTY_CHAR) {

                if ((Y >= t->h) || (X >= BASE && X < t->w && Y >= BASE && t->game[X][Y] != EMPTY_CHAR)) {
                    return 1;
                }

            }

        }

    }

    return 0;
}


/**
 * New block for tetris game.
 *
 * @param {t} the tetris block object.
 */
void tetris_new_block(struct tetris *t) {
    t->current = blocks[random() % TETRIS_PIECES];
    t->x = (t->w / 2) - (t->current.w / 2);
    t->y = BASE;

    if (tetris_hittest(t)) {
        t->gameover = 1;
    }
}

/**
 * Print out the given block.
 *
 * @param {t} the tetris block object.
 */
void tetris_print_block(struct tetris *t) {
    int x, y, X, Y;
    struct tetris_block b = t->current;

    for (x = 0; x < b.w; x++) {

        for (y = 0; y < b.h; y++) {

            if (b.data[y][x] != EMPTY_CHAR)
                t->game[t->x + x][t->y + y] = b.data[y][x];
        }
    }
}

void tetris_rotate(struct tetris *t) {
    struct tetris_block b = t->current;
    struct tetris_block s = b;
    int x, y;

    b.w = s.h;
    b.h = s.w;

    for (x = 0; x < s.w; x++) {
        for (y = 0; y < s.h; y++) {
            b.data[x][y] = s.data[s.h - y - 1][x];
        }
    }

    x = t->x;
    y = t->y;
    t->x -= (b.w - s.w) / 2;
    t->y -= (b.h - s.h) / 2;
    t->current = b;

    if (tetris_hittest(t)) {
        t->current = s;
        t->x = x;
        t->y = y;
    }
}

void tetris_gravity(struct tetris *t) {
    int x, y;
    t->y++;

    if (tetris_hittest(t)) {
        t->y--;
        tetris_print_block(t);
        tetris_new_block(t);
    }
}

void tetris_fall(struct tetris *t, int l) {
    int x, y;

    for (y = l; y > 0; y--) {
        for (x = 0; x < t->w; x++) {
            t->game[x][y] = t->game[x][y - 1];
        }
    }

    for (x = 0; x < t->w; x++) {
        t->game[x][BASE] = EMPTY_CHAR;
    }
}

void checkLines(struct tetris *t) {
    int x, y, l;
    int p = 100;

    for (y = t->h - 1; y >= 0; y--) {
        l = 1;

        for (x = 0; x < t->w && l; x++) {
            if (t->game[x][y] == EMPTY_CHAR) {
                l = BASE;
            }
        }

        if (l) {
            t->score += p;
            p <<= 1;
            tetris_fall(t, y);
            y++;
        }
    }
}

/**
 * Set the tetris level
 */
int tetrisLevel(struct tetris *t) {
    int i;

    for (i = 0; i < TETRIS_LEVELS; i++) {

        if (t->score >= levels[i].score) {
            t->level = i + 1;
        } else {
            break;
        }

    }

    return levels[t->level - 1].nsec;
}


/**
 * Run the tetris game.
 *
 * @param {w} width
 * @param {h} height
 */
void tetris_run(int w, int h) {
    struct timespec tm;
    struct tetris t;
    char cmd;
    int count = 0;

    tetris_set_ioconfig();
    tetris_init(&t, w, h);
    srand(time(NULL));

    tm.tv_sec = TIME_ZERO;
    tm.tv_nsec = TIME_COUNT_NANO;

    tetris_new_block(&t);

    while (!t.gameover) {
        nanosleep(&tm, NULL);
        count++;

        if (count % SMALL_COUNT_LIMIT == 0) {
            tetris_print(&t);
        }

        if (count % LARGE_COUNT_LIMIT == 0) {
            tetris_gravity(&t);
            checkLines(&t);
        }

        while ((cmd = getchar()) > 0) {
            switch (cmd) {
                case MOVE_L:
                    t.x--;
                    if (tetris_hittest(&t)) {
                        t.x++;
                    }
                    break;
                case MOVE_R:
                    t.x++;
                    if (tetris_hittest(&t)) {
                        t.x--;
                    }
                    break;
                case MOVE_D:
                    tetris_gravity(&t);
                    break;
                case EMPTY_CHAR:
                    tetris_rotate(&t);
                    break;
            }
        }

        tm.tv_nsec = tetrisLevel(&t);
    }

    tetris_print(&t);
    printf("*** GAME OVER ***\n");

    tetris_clean(&t);
    tetris_cleanup_io();
}
