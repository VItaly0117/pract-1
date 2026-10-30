#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "game.h"

static void read_line(const char *prompt, char *buffer, int size) {
    printf("%s", prompt);
    if (!fgets(buffer, size, stdin)) {
        exit(EXIT_SUCCESS);
    }
    buffer[strcspn(buffer, "\n")] = '\0';
}

static char ask_letter(void) {
    char line[32];
    for (;;) {
        read_line("Літера: ", line, sizeof line);
        if (strlen(line) == 1 && isalpha((unsigned char)line[0])) {
            return (char)tolower((unsigned char)line[0]);
        }
        puts("Введіть одну латинську літеру");
    }
}

static int ask_position(int length) {
    char line[32], prompt[32];
    snprintf(prompt, sizeof prompt, "Позиція (0-%d): ", length - 1);
    for (;;) {
        read_line(prompt, line, sizeof line);
        char *end;
        long position = strtol(line, &end, 10);
        if (end != line && *end == '\0' && position >= 0 && position < length) {
            return (int)position;
        }
        printf("Введіть число від 0 до %d\n", length - 1);
    }
}

int main(void) {
    srand((unsigned)time(NULL) * 2654435761u);
    Game game;
    game_start(&game);
    printf("Загадано слово з %d літер. Дозволено невдалих спроб: %d\n", game.length, MAX_MISSES);
    game_print(&game);
    while (!game_won(&game) && !game_lost(&game)) {
        char letter = ask_letter();
        if (game_guess(&game, letter, ask_position(game.length))) {
            puts("Вгадали!");
        } else {
            printf("Не вгадали. Залишилось спроб: %d\n", MAX_MISSES - game.misses);
        }
        game_print(&game);
    }
    if (game_won(&game)) {
        printf("Перемога! Невдалих спроб: %d\n", game.misses);
    } else {
        printf("Спроби вичерпано. Загадане слово: %s\n", game.word);
    }
    return 0;
}
