#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "game.h"
#include "raylib.h"

#define WIDTH 640
#define HEIGHT 380
#define CELL 64
#define GAP 10

static const Color BACKGROUND = {246, 247, 250, 255};
static const Color INK = {33, 37, 41, 255};
static const Color MUTED = {120, 126, 136, 255};
static const Color BORDER = {200, 205, 214, 255};
static const Color SELECTED = {255, 224, 138, 255};
static const Color OPENED = {198, 239, 198, 255};
static const Color MISSED = {246, 196, 196, 255};
static const Color ACCENT = {52, 103, 214, 255};

typedef struct {
    Game game;
    int selected;
    char status[96];
} App;

static Font load_font(int size) {
    static const char *paths[] = {
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    };
    int codepoints[96 + 256 + 4], count = 0;
    for (int c = 32; c < 128; c++) codepoints[count++] = c;
    for (int c = 0x400; c < 0x500; c++) codepoints[count++] = c;
    codepoints[count++] = 0xAB;
    codepoints[count++] = 0xBB;
    codepoints[count++] = 0x2013;
    codepoints[count++] = 0x2014;
    for (size_t i = 0; i < sizeof paths / sizeof paths[0]; i++) {
        if (FileExists(paths[i])) {
            Font font = LoadFontEx(paths[i], size, codepoints, count);
            SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
            return font;
        }
    }
    return GetFontDefault();
}

static void centered(Font font, const char *text, float y, float size, Color color) {
    Vector2 measure = MeasureTextEx(font, text, size, 0);
    DrawTextEx(font, text, (Vector2){(WIDTH - measure.x) / 2, y}, size, 0, color);
}

static Rectangle cell_rect(const App *app, int position) {
    float total = (float)(app->game.length * CELL + (app->game.length - 1) * GAP);
    return (Rectangle){(WIDTH - total) / 2 + (float)(position * (CELL + GAP)), 120, CELL, CELL};
}

static bool over(const App *app) {
    return game_won(&app->game) || game_lost(&app->game);
}

static void restart(App *app) {
    game_start(&app->game);
    app->selected = 0;
    app->status[0] = '\0';
}

static void guess(App *app, char letter) {
    if (game_guess(&app->game, letter, app->selected)) {
        snprintf(app->status, sizeof app->status, "Вгадали!");
        for (int step = 1; step < app->game.length; step++) {
            int next = (app->selected + step) % app->game.length;
            if (app->game.mask[next] == HIDDEN) {
                app->selected = next;
                break;
            }
        }
    } else {
        snprintf(app->status, sizeof app->status, "Літери «%c» немає на позиції %d", letter, app->selected);
    }
    if (game_won(&app->game)) {
        snprintf(app->status, sizeof app->status, "Перемога! Слово вгадано");
    } else if (game_lost(&app->game)) {
        snprintf(app->status, sizeof app->status, "Спроби вичерпано. Загадане слово: %s", app->game.word);
    }
}

static void update(App *app, Rectangle restart_button) {
    bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    Vector2 mouse = GetMousePosition();
    if ((clicked && CheckCollisionPointRec(mouse, restart_button)) || (over(app) && IsKeyPressed(KEY_ENTER))) {
        restart(app);
        return;
    }
    if (over(app)) return;
    int length = app->game.length;
    if (IsKeyPressed(KEY_LEFT)) app->selected = (app->selected + length - 1) % length;
    if (IsKeyPressed(KEY_RIGHT)) app->selected = (app->selected + 1) % length;
    for (int position = 0; clicked && position < length; position++) {
        if (CheckCollisionPointRec(mouse, cell_rect(app, position))) app->selected = position;
    }
    for (int key = GetKeyPressed(); key && !over(app); key = GetKeyPressed()) {
        if (key >= KEY_A && key <= KEY_Z) guess(app, (char)('a' + key - KEY_A));
    }
}

static void draw(const App *app, Font font, Rectangle restart_button) {
    char text[64];
    centered(font, "Вгадай слово", 28, 34, INK);
    centered(font, "Оберіть клітинку мишею або стрілками та натисніть літеру", 74, 18, MUTED);
    for (int position = 0; position < app->game.length; position++) {
        Rectangle rect = cell_rect(app, position);
        char shown[2] = {app->game.mask[position], '\0'};
        Color fill = WHITE;
        if (shown[0] != HIDDEN) {
            fill = OPENED;
        } else if (game_lost(&app->game)) {
            fill = MISSED;
            shown[0] = app->game.word[position];
        } else if (position == app->selected) {
            fill = SELECTED;
        }
        DrawRectangleRounded(rect, 0.18f, 8, fill);
        DrawRectangleRoundedLines(rect, 0.18f, 8, BORDER);
        Vector2 size = MeasureTextEx(font, shown, 40, 0);
        DrawTextEx(font, shown, (Vector2){rect.x + (CELL - size.x) / 2, rect.y + (CELL - size.y) / 2}, 40, 0, INK);
        snprintf(text, sizeof text, "%d", position);
        size = MeasureTextEx(font, text, 16, 0);
        DrawTextEx(font, text, (Vector2){rect.x + (CELL - size.x) / 2, rect.y + CELL + 6}, 16, 0, MUTED);
    }
    snprintf(text, sizeof text, "Невдалі спроби: %d з %d", app->game.misses, MAX_MISSES);
    centered(font, text, 228, 20, INK);
    centered(font, app->status, 258, 20, game_lost(&app->game) ? (Color){190, 40, 40, 255} : ACCENT);
    bool hover = CheckCollisionPointRec(GetMousePosition(), restart_button);
    DrawRectangleRounded(restart_button, 0.3f, 8, hover ? (Color){38, 84, 186, 255} : ACCENT);
    Vector2 size = MeasureTextEx(font, "Нова гра", 18, 0);
    DrawTextEx(font, "Нова гра", (Vector2){restart_button.x + (restart_button.width - size.x) / 2,
               restart_button.y + (restart_button.height - size.y) / 2}, 18, 0, WHITE);
}

int main(void) {
    srand((unsigned)time(NULL) * 2654435761u);
    SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT);
    InitWindow(WIDTH, HEIGHT, "Вгадай слово");
    SetTargetFPS(60);
    Font font = load_font(64);
    App app;
    restart(&app);
    Rectangle restart_button = {(WIDTH - 140) / 2.0f, 304, 140, 40};
    while (!WindowShouldClose()) {
        update(&app, restart_button);
        BeginDrawing();
        ClearBackground(BACKGROUND);
        draw(&app, font, restart_button);
        EndDrawing();
    }
    UnloadFont(font);
    CloseWindow();
    return 0;
}
