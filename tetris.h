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
    struct tetris_block *current;
};


#endif //TETRIS_H