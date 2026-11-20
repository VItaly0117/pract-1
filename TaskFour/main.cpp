#include <cstring>
#include <random>
#include <string>

#include "raylib.h"

using namespace std;

// Скільки разів можна помилитися
const int MAX_MISSES = 5;

// Матриця слів: кожен рядок це одне слово, решта рядка заповнена нулями
char WORDS[7][7] = {
    {'t', 'h', 'i', 's'},
    {'i', 's'},
    {'a'},
    {'w', 'o', 'r', 'd'},
    {'a', 'r', 'r', 'a', 'y'},
    {'m', 'a', 't', 'r', 'i', 'x'},
    {'l', 'e', 't', 't', 'e', 'r'},
};

char *word;
int length;
char mask[7];
int misses;
int opened;
int selected;
string status;  // що сталося після останнього натискання
Font font;

// Шрифт з кирилицею, бо стандартний шрифт raylib знає тільки латиницю
Font load_font() {
    int codepoints[352];
    int count = 0;
    for (int c = 32; c < 128; c++) codepoints[count++] = c;
    for (int c = 0x400; c < 0x500; c++) codepoints[count++] = c;
    const char *path = "/System/Library/Fonts/Supplemental/Arial.ttf";
    if (!FileExists(path)) path = "C:/Windows/Fonts/arial.ttf";
    return LoadFontEx(path, 48, codepoints, count);
}

void text(string s, float x, float y, float size, Color color) {
    DrawTextEx(font, s.c_str(), {x, y}, size, 0, color);
}

// Беремо випадкове слово і закриваємо всі літери зірочками
void new_game() {
    random_device random;
    word = WORDS[random() % 7];
    length = strlen(word);
    for (int i = 0; i < length; i++) {
        mask[i] = '*';
    }
    misses = 0;
    opened = 0;
    selected = 0;
    status = "Оберіть клітинку і натисніть літеру";
}

int main() {
    InitWindow(640, 400, "Вгадай слово");
    SetTargetFPS(60);
    font = load_font();
    new_game();

    while (!WindowShouldClose()) {
        bool playing = opened < length && misses < MAX_MISSES;
        Vector2 mouse = GetMousePosition();
        bool click = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        // Клік по клітинці обирає позицію
        for (int i = 0; i < length; i++) {
            Rectangle cell = {40.0f + i * 90, 110, 80, 80};
            if (click && CheckCollisionPointRec(mouse, cell)) selected = i;
        }

        // Натиснута клавіша перевіряється на обраній позиції.
        // Помилки користувача (не та клавіша, вже відкрита клітинка) не рахуються як невдала спроба
        int key = GetKeyPressed();
        if (playing && key != 0) {
            char letter = 'a' + (key - KEY_A);
            if (key < KEY_A || key > KEY_Z) {
                status = "Натискайте тільки латинські літери";
            } else if (mask[selected] != '*') {
                status = "Ця клітинка вже відкрита, оберіть іншу";
            } else if (word[selected] == letter) {
                mask[selected] = letter;
                opened++;
                status = "Вгадали!";
            } else {
                misses++;
                status = "Літери " + string(1, letter) + " на цій позиції немає";
            }
        }

        Rectangle restart = {40, 320, 180, 44};
        if (click && CheckCollisionPointRec(mouse, restart)) new_game();

        BeginDrawing();
        ClearBackground(RAYWHITE);
        text("Вгадай слово", 40, 30, 36, DARKGRAY);

        for (int i = 0; i < length; i++) {
            Rectangle cell = {40.0f + i * 90, 110, 80, 80};
            Color color = WHITE;
            if (i == selected) color = GOLD;
            if (mask[i] != '*') color = LIME;
            DrawRectangleRec(cell, color);
            DrawRectangleLinesEx(cell, 2, GRAY);
            text(string(1, mask[i]), cell.x + 28, cell.y + 18, 44, BLACK);
        }

        text("Помилки: " + to_string(misses) + " з " + to_string(MAX_MISSES), 40, 215, 24, DARKGRAY);
        if (opened == length) {
            text("Перемога!", 40, 260, 28, DARKGREEN);
        } else if (misses >= MAX_MISSES) {
            text("Поразка. Слово: " + string(word), 40, 260, 28, MAROON);
        } else {
            text(status, 40, 260, 24, GRAY);
        }

        DrawRectangleRec(restart, SKYBLUE);
        text("Нова гра", restart.x + 40, restart.y + 10, 24, BLACK);
        EndDrawing();
    }
    CloseWindow();
}
