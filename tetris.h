#ifndef TETRIS_H
#define TETRIS_H

struct tetris_level {
    int score;
    int nsec;
};

struct tetris_block {
    char data[5][5];
    int w;
    int h;
};

// The aim of this struct is to store the information of tetris game.
struct tetris {
    char **game;
    char w;
    char h;
    char level;
    char gameover;
    int score;
    int x;
    int y;
    struct tetris_block current;
};


/*------------------------------- function prototyeps -------------------------------*/

void tetris_cleanup_io();

void tetris_signal_quit(int);

void tetris_set_ioconfig();

void tetris_init(struct tetris *t, int w, int h);

void tetris_clean(struct tetris *t);

void tetris_print(struct tetris *t);

void tetris_run(int width, int height);

void tetris_new_block(struct tetris *t);

void tetris_new_block(struct tetris *t);

void tetris_print_block(struct tetris *t);

void tetris_rotate(struct tetris *t);

void tetris_gravity(struct tetris *t);

void tetris_fall(struct tetris *t, int l);

void tetris_check_lines(struct tetris *t);

int tetris_level(struct tetris *t);

#endif //TETRIS_H