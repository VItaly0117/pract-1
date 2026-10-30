#include "game.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char WORDS[][MAX_LENGTH] = {
    {'t', 'h', 'i', 's'},
    {'i', 's'},
    {'a'},
    {'w', 'o', 'r', 'd'},
    {'a', 'r', 'r', 'a', 'y'},
    {'m', 'a', 't', 'r', 'i', 'x'},
    {'l', 'e', 't', 't', 'e', 'r'},
};

void game_start(Game *game) {
    game->word = WORDS[rand() % (int)(sizeof WORDS / sizeof WORDS[0])];
    game->length = (int)strlen(game->word);
    memset(game->mask, HIDDEN, (size_t)game->length);
    game->misses = 0;
}

bool game_guess(Game *game, char letter, int position) {
    bool hit = game->word[position] == tolower((unsigned char)letter);
    if (hit) {
        game->mask[position] = game->word[position];
    } else {
        game->misses++;
    }
    return hit;
}

bool game_won(const Game *game) {
    return memchr(game->mask, HIDDEN, (size_t)game->length) == NULL;
}

bool game_lost(const Game *game) {
    return game->misses >= MAX_MISSES;
}

void game_print(const Game *game) {
    printf("{ ");
    for (int i = 0; i < game->length; i++) {
        printf("'%c'%s", game->mask[i], i + 1 < game->length ? ", " : " }\n");
    }
}
