#include "anketa.h"
#include "raylib.h"

enum Screen { LOGIN, FORM, MAIN };

Registry registry;
Font font;
Screen screen = LOGIN;
Student user, draft;
std::string login_text, password, age, filter, status;
bool make_admin = false;
int focus = 0, fields = 0;

Font load_font() {
    std::vector<int> codepoints;
    for (int c = 32; c < 0x500; c++) codepoints.push_back(c);
    codepoints.push_back(0x2014);
    for (const char *path : {"/System/Library/Fonts/Supplemental/Arial.ttf", "C:/Windows/Fonts/arial.ttf"}) {
        if (FileExists(path)) return LoadFontEx(path, 40, codepoints.data(), (int)codepoints.size());
    }
    return GetFontDefault();
}

void write(const std::string &text, float x, float y, float size, Color color) {
    DrawTextEx(font, text.c_str(), {x, y}, size, 0, color);
}

bool clicked(Rectangle box) {
    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), box);
}

bool button(Rectangle box, const std::string &label) {
    DrawRectangleRec(box, CheckCollisionPointRec(GetMousePosition(), box) ? DARKBLUE : BLUE);
    Vector2 measure = MeasureTextEx(font, label.c_str(), 18, 0);
    write(label, box.x + (box.width - measure.x) / 2, box.y + (box.height - measure.y) / 2, 18, WHITE);
    return clicked(box);
}

void field(Rectangle box, const std::string &label, std::string &value, bool secret = false) {
    int id = fields++;
    if (clicked(box)) focus = id;
    if (focus == id) {
        for (int c = GetCharPressed(); c; c = GetCharPressed()) {
            int size;
            const char *bytes = CodepointToUTF8(c, &size);
            value.append(bytes, size);
        }
        if (IsKeyPressed(KEY_BACKSPACE) && !value.empty()) {
            while ((value.back() & 0xC0) == 0x80) value.pop_back();
            value.pop_back();
        }
    }
    write(label, box.x, box.y - 22, 16, GRAY);
    DrawRectangleRec(box, WHITE);
    DrawRectangleLinesEx(box, 2, focus == id ? BLUE : LIGHTGRAY);
    write((secret ? std::string(value.size(), '*') : value) + (focus == id ? "|" : ""), box.x + 10, box.y + 10, 20, BLACK);
}

void checkbox(float x, float y, const std::string &label, bool &value) {
    if (clicked({x, y, 200, 24})) value = !value;
    DrawRectangleLinesEx({x, y, 24, 24}, 2, GRAY);
    if (value) DrawRectangle((int)x + 6, (int)y + 6, 12, 12, BLUE);
    write(label, x + 34, y + 2, 18, BLACK);
}

void open_form() {
    draft = Student();
    password = age = "";
    make_admin = false;
    focus = 0;
    screen = FORM;
}

void back() {
    screen = user.login.empty() ? LOGIN : MAIN;
    password.clear();
    focus = 0;
}

void draw_login() {
    write("Анкетування студентів", 220, 60, 32, DARKGRAY);
    field({220, 160, 320, 44}, "Логін", login_text);
    field({220, 240, 320, 44}, "Пароль", password, true);
    if (button({220, 310, 150, 44}, "Увійти") || IsKeyPressed(KEY_ENTER)) {
        Student *found = registry.login(login_text, password);
        status = found ? "" : "Невірний логін або пароль";
        if (found) user = *found;
        back();
    }
    if (button({390, 310, 150, 44}, "Реєстрація")) {
        status.clear();
        open_form();
    }
}

void draw_form() {
    bool setup = !registry.has_admin();
    write(setup ? "Створіть адміністратора" : "Нова анкета", 40, 30, 28, DARKGRAY);
    field({40, 110, 320, 44}, "Логін", draft.login);
    field({400, 110, 320, 44}, "Пароль", password, true);
    field({40, 190, 320, 44}, "Ім'я", draft.name);
    field({400, 190, 320, 44}, "Прізвище", draft.surname);
    field({40, 270, 320, 44}, "Вік", age);
    checkbox(400, 270, "Студент", draft.is_student);
    if (user.role == "admin") checkbox(400, 310, "Адміністратор", make_admin);
    if (button({40, 370, 160, 44}, "Зберегти") || IsKeyPressed(KEY_ENTER)) {
        draft.role = setup || make_admin ? "admin" : "user";
        draft.age = parse_age(age);
        status = registry.add(draft, password);
        if (status.empty()) {
            status = "Анкету збережено: " + draft.name + " " + draft.surname;
            back();
        }
    }
    if (!setup && button({220, 370, 160, 44}, "Назад")) back();
}

void draw_main() {
    bool admin = user.role == "admin";
    write("Вітаємо, " + user.name + "! (" + user.role + ")", 40, 30, 26, DARKGRAY);
    if (button({600, 26, 120, 40}, "Вийти")) {
        user = Student();
        login_text = status = "";
        back();
        return;
    }
    std::vector<Student> list = {user};
    if (admin) {
        field({40, 110, 320, 40}, "Пошук за ім'ям", filter);
        if (button({480, 110, 240, 40}, "Створити користувача")) open_form();
        list = registry.filter(filter);
        if (list.empty()) write("Анкет не знайдено", 40, 180, 20, GRAY);
    }
    float y = admin ? 180 : 100;
    for (const Student &s : list) {
        write(card(s), 40, y, 18, BLACK);
        y += 30;
    }
}

int main() {
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(760, 500, "Анкетування");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);
    font = load_font();
    status = registry.load() ? "Файл " + DATA_FILE + " не знайдено — створено новий"
                             : "Завантажено анкет: " + std::to_string(registry.students.size());
    if (!registry.has_admin()) open_form();
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_TAB) && fields) focus = (focus + 1) % fields;
        fields = 0;
        BeginDrawing();
        ClearBackground(RAYWHITE);
        if (screen == LOGIN) {
            draw_login();
        } else if (screen == FORM) {
            draw_form();
        } else {
            draw_main();
        }
        write(status, 40, 460, 18, status.rfind("Анкету", 0) == 0 || status.rfind("Файл", 0) == 0 ? DARKGREEN : MAROON);
        EndDrawing();
    }
    CloseWindow();
}
