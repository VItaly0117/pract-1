#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

#define MAX_MISSES 5
#define MAX_LENGTH 7
#define HIDDEN '*'

typedef struct {
    const char *word;
    int length;
    char mask[MAX_LENGTH];
    int misses;
} Game;

void game_start(Game *game);
bool game_guess(Game *game, char letter, int position);
bool game_won(const Game *game);
bool game_lost(const Game *game);
void game_print(const Game *game);

#endif
