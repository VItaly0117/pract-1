#include <math.h>
#include <stdio.h>
#include <string.h>

#include "anketa.h"
#include "raylib.h"

#define WIDTH 760
#define HEIGHT 500
#define TEXT 18

static const Color BACKGROUND = {246, 247, 250, 255};
static const Color INK = {33, 37, 41, 255};
static const Color MUTED = {120, 126, 136, 255};
static const Color BORDER = {200, 205, 214, 255};
static const Color ACCENT = {52, 103, 214, 255};
static const Color ACCENT_DARK = {38, 84, 186, 255};
static const Color LIGHT = {232, 235, 241, 255};
static const Color SHADE = {20, 24, 32, 110};

typedef enum { SCREEN_LOGIN, SCREEN_USER, SCREEN_ADMIN } Screen;
typedef enum { PURPOSE_SETUP, PURPOSE_REGISTER, PURPOSE_CREATE } Purpose;

typedef struct {
    Font font;
    int layer;
    int top;
    int focus;
    int fields;
    int field_total;
} Ui;

typedef struct {
    bool open;
    Purpose purpose;
    char login[FIELD_SIZE], password[FIELD_SIZE], name[FIELD_SIZE], surname[FIELD_SIZE], age[8];
    bool is_student, is_admin;
} Dialog;

typedef struct {
    Registry registry;
    Ui ui;
    Screen screen;
    Student *user;
    char login[FIELD_SIZE], password[FIELD_SIZE], filter[FIELD_SIZE];
    Dialog dialog;
    char message[400];
    int scroll;
} App;

static App app;

static Font load_font(int size) {
    static const char *paths[] = {
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    };
    int codepoints[96 + 256 + 5], count = 0;
    for (int c = 32; c < 128; c++) codepoints[count++] = c;
    for (int c = 0x400; c < 0x500; c++) codepoints[count++] = c;
    codepoints[count++] = 0xAB;
    codepoints[count++] = 0xBB;
    codepoints[count++] = 0x2013;
    codepoints[count++] = 0x2014;
    codepoints[count++] = 0x2022;
    for (size_t i = 0; i < sizeof paths / sizeof paths[0]; i++) {
        if (FileExists(paths[i])) {
            Font font = LoadFontEx(paths[i], size, codepoints, count);
            SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
            return font;
        }
    }
    return GetFontDefault();
}

static void label(const char *text, float x, float y, float size, Color color) {
    DrawTextEx(app.ui.font, text, (Vector2){x, y}, size, 0, color);
}

static float text_width(const char *text, float size) {
    return MeasureTextEx(app.ui.font, text, size, 0).x;
}

static bool interactive(void) {
    return app.ui.layer == app.ui.top;
}

static bool hovered(Rectangle rect) {
    return interactive() && CheckCollisionPointRec(GetMousePosition(), rect);
}

static bool button(Rectangle rect, const char *text, bool primary) {
    bool hover = hovered(rect);
    Color fill = primary ? (hover ? ACCENT_DARK : ACCENT) : (hover ? BORDER : LIGHT);
    DrawRectangleRounded(rect, 0.3f, 8, fill);
    label(text, rect.x + (rect.width - text_width(text, TEXT)) / 2, rect.y + (rect.height - TEXT) / 2 - 1, TEXT,
          primary ? WHITE : INK);
    return hover && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}

static void append(char *buffer, int size, const char *text) {
    if (strlen(buffer) + strlen(text) < (size_t)size) strcat(buffer, text);
}

static void erase_last(char *buffer) {
    size_t length = strlen(buffer);
    while (length > 0 && ((unsigned char)buffer[length - 1] & 0xC0) == 0x80) length--;
    buffer[length > 0 ? length - 1 : 0] = '\0';
}

static void field(Rectangle rect, const char *caption, char *buffer, int size, bool secret) {
    int id = interactive() ? app.ui.fields++ : -1;
    if (hovered(rect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) app.ui.focus = id;
    bool focused = id >= 0 && app.ui.focus == id;
    if (focused) {
        for (int c = GetCharPressed(); c; c = GetCharPressed()) {
            int length;
            const char *utf8 = CodepointToUTF8(c, &length);
            char piece[5] = {0};
            memcpy(piece, utf8, (size_t)length);
            append(buffer, size, piece);
        }
        if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) erase_last(buffer);
        bool command = IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER) || IsKeyDown(KEY_LEFT_CONTROL);
        if (command && IsKeyPressed(KEY_V) && GetClipboardText()) append(buffer, size, GetClipboardText());
    }
    label(caption, rect.x - 120, rect.y + 8, TEXT, INK);
    DrawRectangleRounded(rect, 0.2f, 8, WHITE);
    DrawRectangleRoundedLines(rect, 0.2f, 8, focused ? ACCENT : BORDER);
    char shown[FIELD_SIZE * 3] = "";
    if (secret) {
        for (int i = GetCodepointCount(buffer); i > 0; i--) strcat(shown, "•");
    } else {
        strcpy(shown, buffer);
    }
    label(shown, rect.x + 10, rect.y + 8, TEXT, INK);
    if (focused && fmod(GetTime(), 1.0) < 0.5) {
        float x = rect.x + 11 + text_width(shown, TEXT);
        DrawLineEx((Vector2){x, rect.y + 7}, (Vector2){x, rect.y + rect.height - 7}, 1.5f, INK);
    }
}

static void checkbox(float x, float y, const char *text, bool *value) {
    Rectangle box = {x, y, 22, 22};
    if (hovered((Rectangle){x, y, 32 + text_width(text, TEXT), 22}) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        *value = !*value;
    }
    DrawRectangleRounded(box, 0.25f, 6, *value ? ACCENT : WHITE);
    DrawRectangleRoundedLines(box, 0.25f, 6, *value ? ACCENT : BORDER);
    if (*value) {
        DrawLineEx((Vector2){x + 5, y + 11}, (Vector2){x + 9, y + 16}, 2.5f, WHITE);
        DrawLineEx((Vector2){x + 9, y + 16}, (Vector2){x + 17, y + 6}, 2.5f, WHITE);
    }
    label(text, x + 32, y + 1, TEXT, INK);
}

static void show_message(const char *text) {
    snprintf(app.message, sizeof app.message, "%s", text);
}

static void open_dialog(Purpose purpose) {
    app.dialog = (Dialog){.open = true, .purpose = purpose, .is_student = true};
}

static void save_dialog(void) {
    Dialog *d = &app.dialog;
    Student profile = {.age = parse_age(d->age), .is_student = d->is_student};
    profile.role = d->purpose == PURPOSE_SETUP || (d->purpose == PURPOSE_CREATE && d->is_admin) ? ROLE_ADMIN : ROLE_USER;
    snprintf(profile.login, FIELD_SIZE, "%s", d->login);
    snprintf(profile.name, FIELD_SIZE, "%s", d->name);
    snprintf(profile.surname, FIELD_SIZE, "%s", d->surname);
    const char *error = registry_add(&app.registry, &profile, d->password);
    if (error) {
        show_message(error);
        return;
    }
    d->open = false;
    if (d->purpose == PURPOSE_REGISTER) {
        char card[320], text[400];
        student_card(&app.registry.items[app.registry.count - 1], card, sizeof card);
        snprintf(text, sizeof text, "Анкету збережено\n\n%s", card);
        show_message(text);
    }
}

static void login(void) {
    Student *user = registry_authenticate(&app.registry, app.login, app.password);
    if (!user) {
        show_message("Невірний логін або пароль");
        return;
    }
    app.user = user;
    app.screen = user->role == ROLE_ADMIN ? SCREEN_ADMIN : SCREEN_USER;
    app.password[0] = app.filter[0] = '\0';
    app.scroll = 0;
    app.ui.focus = 0;
}

static void logout(void) {
    app.screen = SCREEN_LOGIN;
    app.user = NULL;
    app.login[0] = app.password[0] = '\0';
    app.ui.focus = 0;
}

static bool header(const char *title) {
    label(title, 30, 28, 26, INK);
    return button((Rectangle){WIDTH - 130, 26, 100, 36}, "Вийти", false);
}

static void draw_login(void) {
    label("Вхід до системи", (WIDTH - text_width("Вхід до системи", 30)) / 2, 120, 30, INK);
    field((Rectangle){300, 190, 260, 38}, "Логін", app.login, FIELD_SIZE, false);
    field((Rectangle){300, 240, 260, 38}, "Пароль", app.password, FIELD_SIZE, true);
    if (button((Rectangle){300, 300, 125, 38}, "Реєстрація", false)) open_dialog(PURPOSE_REGISTER);
    if (button((Rectangle){435, 300, 125, 38}, "Увійти", true)) login();
}

static void draw_user(void) {
    char title[96], age[8];
    snprintf(title, sizeof title, "Вітаємо, %s!", app.user->name);
    if (header(title)) {
        logout();
        return;
    }
    label("Мої дані", 30, 96, 22, INK);
    snprintf(age, sizeof age, "%d", app.user->age);
    const char *rows[][2] = {
        {"Логін:", app.user->login}, {"Ім'я:", app.user->name}, {"Прізвище:", app.user->surname},
        {"Вік:", age}, {"Студент:", app.user->is_student ? "так" : "ні"}, {"Роль:", role_name(app.user->role)},
    };
    for (int i = 0; i < 6; i++) {
        label(rows[i][0], 30, 140 + 36.0f * (float)i, TEXT, MUTED);
        label(rows[i][1], 150, 140 + 36.0f * (float)i, TEXT, INK);
    }
}

static void draw_admin(void) {
    char title[160], age[8];
    snprintf(title, sizeof title, "Адміністратор: %s %s", app.user->name, app.user->surname);
    if (header(title)) {
        logout();
        return;
    }
    field((Rectangle){190, 90, 220, 38}, "", app.filter, FIELD_SIZE, false);
    label("Фільтр за ім'ям:", 30, 98, TEXT, INK);
    if (button((Rectangle){WIDTH - 230, 90, 200, 38}, "Створити користувача", true)) open_dialog(PURPOSE_CREATE);
    static const char *columns[] = {"Логін", "Ім'я", "Прізвище", "Вік", "Студент", "Роль"};
    static const float x[] = {40, 160, 290, 450, 520, 620};
    Rectangle table = {30, 146, WIDTH - 60, HEIGHT - 176};
    DrawRectangleRounded(table, 0.03f, 8, WHITE);
    DrawRectangleRoundedLines(table, 0.03f, 8, BORDER);
    for (int c = 0; c < 6; c++) label(columns[c], x[c], table.y + 10, TEXT, MUTED);
    DrawLineEx((Vector2){table.x, table.y + 40}, (Vector2){table.x + table.width, table.y + 40}, 1, BORDER);
    int visible = (int)((table.height - 48) / 30), shown = 0, matches = 0;
    for (int i = 0; i < app.registry.count; i++) {
        const Student *s = &app.registry.items[i];
        if (!student_name_matches(s, app.filter) || matches++ < app.scroll || shown >= visible) continue;
        float y = table.y + 48 + 30.0f * (float)shown++;
        snprintf(age, sizeof age, "%d", s->age);
        const char *cells[] = {s->login, s->name, s->surname, age, s->is_student ? "так" : "ні", role_name(s->role)};
        for (int c = 0; c < 6; c++) label(cells[c], x[c], y, TEXT, INK);
    }
    if (hovered(table)) {
        app.scroll -= (int)GetMouseWheelMove();
        if (app.scroll > matches - visible) app.scroll = matches - visible;
        if (app.scroll < 0) app.scroll = 0;
    }
    if (matches == 0) label("Анкет не знайдено", x[0], table.y + 48, TEXT, MUTED);
}

static void draw_dialog(void) {
    static const char *titles[] = {"Створення адміністратора", "Реєстрація", "Новий користувач"};
    Dialog *d = &app.dialog;
    Rectangle box = {(WIDTH - 440) / 2.0f, 30, 440, 440};
    DrawRectangle(0, 0, WIDTH, HEIGHT, SHADE);
    DrawRectangleRounded(box, 0.05f, 8, BACKGROUND);
    label(titles[d->purpose], box.x + 30, box.y + 24, 24, INK);
    float x = box.x + 150, w = 260;
    field((Rectangle){x, box.y + 76, w, 36}, "Логін", d->login, FIELD_SIZE, false);
    field((Rectangle){x, box.y + 122, w, 36}, "Пароль", d->password, FIELD_SIZE, true);
    field((Rectangle){x, box.y + 168, w, 36}, "Ім'я", d->name, FIELD_SIZE, false);
    field((Rectangle){x, box.y + 214, w, 36}, "Прізвище", d->surname, FIELD_SIZE, false);
    field((Rectangle){x, box.y + 260, w, 36}, "Вік", d->age, sizeof d->age, false);
    checkbox(x, box.y + 310, "Студент", &d->is_student);
    if (d->purpose == PURPOSE_CREATE) checkbox(x, box.y + 342, "Адміністратор", &d->is_admin);
    if (d->purpose != PURPOSE_SETUP && button((Rectangle){box.x + 150, box.y + 384, 125, 36}, "Скасувати", false)) {
        d->open = false;
    }
    if (button((Rectangle){box.x + 285, box.y + 384, 125, 36}, "Зберегти", true)) save_dialog();
}

static void draw_message(void) {
    Vector2 size = MeasureTextEx(app.ui.font, app.message, TEXT, 0);
    float width = fmaxf(320, size.x + 60), height = size.y + 110;
    Rectangle box = {(WIDTH - width) / 2, (HEIGHT - height) / 2, width, height};
    DrawRectangle(0, 0, WIDTH, HEIGHT, SHADE);
    DrawRectangleRounded(box, 0.08f, 8, WHITE);
    label(app.message, box.x + 30, box.y + 28, TEXT, INK);
    if (button((Rectangle){box.x + box.width - 130, box.y + box.height - 54, 100, 36}, "OK", true)) {
        app.message[0] = '\0';
    }
}

static void handle_keys(void) {
    if (app.ui.top != app.ui.layer) {
        app.ui.layer = app.ui.top;
        app.ui.focus = 0;
    }
    if (IsKeyPressed(KEY_TAB) && app.ui.field_total > 0) {
        bool back = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
        app.ui.focus = (app.ui.focus + (back ? app.ui.field_total - 1 : 1)) % app.ui.field_total;
    }
    if (!IsKeyPressed(KEY_ENTER)) return;
    if (app.ui.top == 2) {
        app.message[0] = '\0';
    } else if (app.ui.top == 1) {
        save_dialog();
    } else if (app.screen == SCREEN_LOGIN) {
        login();
    }
}

int main(void) {
    bool created = registry_load(&app.registry);
    SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT);
    InitWindow(WIDTH, HEIGHT, "Анкетування");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);
    app.ui.font = load_font(48);
    if (!registry_has_admin(&app.registry)) open_dialog(PURPOSE_SETUP);
    if (created) show_message("Файл " DATA_FILE " не знайдено — створено новий");
    while (!WindowShouldClose()) {
        app.ui.top = app.message[0] ? 2 : app.dialog.open ? 1 : 0;
        handle_keys();
        BeginDrawing();
        ClearBackground(BACKGROUND);
        app.ui.fields = 0;
        app.ui.layer = 0;
        if (app.screen == SCREEN_LOGIN) draw_login();
        if (app.screen == SCREEN_USER) draw_user();
        if (app.screen == SCREEN_ADMIN) draw_admin();
        if (app.dialog.open) {
            app.ui.layer = 1;
            draw_dialog();
        }
        if (app.message[0]) {
            app.ui.layer = 2;
            draw_message();
        }
        app.ui.field_total = app.ui.fields;
        app.ui.layer = app.ui.top;
        EndDrawing();
    }
    UnloadFont(app.ui.font);
    CloseWindow();
    return 0;
}
