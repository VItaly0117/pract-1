#include <vector>

#include "game.h"
#include "raylib.h"

const int WIDTH = 640;
Font font;

Font load_font() {
    std::vector<int> codepoints;
    for (int c = 32; c < 0x500; c++) codepoints.push_back(c);
    for (const char *path : {"/System/Library/Fonts/Supplemental/Arial.ttf", "C:/Windows/Fonts/arial.ttf"}) {
        if (FileExists(path)) return LoadFontEx(path, 48, codepoints.data(), (int)codepoints.size());
    }
    return GetFontDefault();
}

void centered(const std::string &text, Rectangle box, float size, Color color) {
    Vector2 measure = MeasureTextEx(font, text.c_str(), size, 0);
    DrawTextEx(font, text.c_str(), {box.x + (box.width - measure.x) / 2, box.y + (box.height - measure.y) / 2}, size, 0, color);
}

int main() {
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(WIDTH, 360, "Вгадай слово");
    SetTargetFPS(60);
    font = load_font();
    Game game;
    int selected = 0;
    std::string status = "Оберіть клітинку мишею або стрілками та натисніть літеру";
    Rectangle restart = {WIDTH / 2 - 70, 290, 140, 40};
    while (!WindowShouldClose()) {
        bool over = game.won() || game.lost();
        bool click = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        Vector2 mouse = GetMousePosition();
        if ((click && CheckCollisionPointRec(mouse, restart)) || (over && IsKeyPressed(KEY_ENTER))) {
            game = Game();
            selected = 0;
            status = "Нова гра";
        }
        int length = (int)game.word.size();
        auto cell = [&](int i) { return Rectangle{(WIDTH - length * 74 + 10) / 2.0f + i * 74, 110, 64, 64}; };
        int key = GetKeyPressed();
        if (!over) {
            if (key == KEY_LEFT) selected = (selected + length - 1) % length;
            if (key == KEY_RIGHT) selected = (selected + 1) % length;
            for (int i = 0; i < length; i++) {
                if (click && CheckCollisionPointRec(mouse, cell(i))) selected = i;
            }
            if (key >= KEY_A && key <= KEY_Z) {
                char letter = (char)('a' + key - KEY_A);
                bool hit = game.guess(letter, selected);
                status = hit ? "Вгадали!" : "Літери «" + std::string(1, letter) + "» немає на позиції " + std::to_string(selected);
                if (game.won()) status = "Перемога! Слово вгадано";
                if (game.lost()) status = "Спроби вичерпано. Загадане слово: " + game.word;
            }
        }
        BeginDrawing();
        ClearBackground(RAYWHITE);
        centered("Вгадай слово", {0, 30, WIDTH, 40}, 34, DARKGRAY);
        for (int i = 0; i < length; i++) {
            bool open = game.mask[i] != HIDDEN;
            Color fill = open ? GREEN : game.lost() ? PINK : i == selected ? GOLD : WHITE;
            DrawRectangleRec(cell(i), fill);
            DrawRectangleLinesEx(cell(i), 2, LIGHTGRAY);
            centered(std::string(1, open || game.lost() ? game.word[i] : HIDDEN), cell(i), 40, BLACK);
            centered(std::to_string(i), {cell(i).x, 180, 64, 16}, 16, GRAY);
        }
        centered("Невдалі спроби: " + std::to_string(game.misses) + " з " + std::to_string(MAX_MISSES), {0, 210, WIDTH, 24}, 20, DARKGRAY);
        centered(status, {0, 245, WIDTH, 24}, 20, game.lost() ? MAROON : BLUE);
        DrawRectangleRec(restart, BLUE);
        centered("Нова гра", restart, 20, WHITE);
        EndDrawing();
    }
    CloseWindow();
}
