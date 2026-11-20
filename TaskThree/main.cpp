#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "raylib.h"

using namespace std;

// Кожне поле анкети лежить в окремому векторі: i-та анкета це елемент i у кожному з них
vector<string> logins, passwords, roles, names, surnames;
vector<int> ages;
vector<bool> students;

// Що зараз введено у поля вікна
string login_text, password_text, name_text, surname_text, age_text, search_text;
bool is_student = false;

int focus = -1;  // номер поля, в яке зараз пишемо
int user = -1;   // номер анкети того, хто увійшов, -1 якщо ніхто
string message;
Font font;

// Пароль у файл не пишемо, тільки його хеш. Логін додаємо, щоб однакові паролі давали різний хеш
string hash_password(string login, string password) {
    return to_string(hash<string>{}(login + password));
}

// Читаємо анкети з файлу. Якщо файлу ще немає, створюємо порожній
void load_file() {
    ifstream file("students.csv");
    if (!file) {
        ofstream("students.csv");
        message = "Файл students.csv не знайдено, створено новий. Перший зареєстрований стане адміністратором";
        return;
    }
    string line;
    while (getline(file, line)) {
        stringstream row(line);
        string login, password, role, name, surname, age, student;
        getline(row, login, ',');
        getline(row, password, ',');
        getline(row, role, ',');
        getline(row, name, ',');
        getline(row, surname, ',');
        getline(row, age, ',');
        getline(row, student, ',');
        logins.push_back(login);
        passwords.push_back(password);
        roles.push_back(role);
        names.push_back(name);
        surnames.push_back(surname);
        ages.push_back(atoi(age.c_str()));
        students.push_back(student == "1");
    }
    message = "Завантажено анкет: " + to_string(logins.size());
}

// Перезаписуємо файл усіма анкетами
void save_file() {
    ofstream file("students.csv");
    for (int i = 0; i < (int)logins.size(); i++) {
        file << logins[i] << "," << passwords[i] << "," << roles[i] << "," << names[i] << "," << surnames[i] << ","
             << ages[i] << "," << students[i] << "\n";
    }
}

// Одна анкета одним рядком
string student_info(int i) {
    return names[i] + " " + surnames[i] + ", вік: " + to_string(ages[i]) + ", студент: " +
           (students[i] ? "так" : "ні") + ", роль: " + roles[i];
}

void register_student() {
    int age = atoi(age_text.c_str());
    if (login_text == "" || password_text == "" || name_text == "" || surname_text == "") {
        message = "Заповніть логін, пароль, ім'я та прізвище";
        return;
    }
    if (age < 1 || age > 120) {
        message = "Вік має бути числом від 1 до 120";
        return;
    }
    for (string login : logins) {
        if (login == login_text) {
            message = "Такий логін вже зайнятий";
            return;
        }
    }
    logins.push_back(login_text);
    passwords.push_back(hash_password(login_text, password_text));
    roles.push_back(logins.size() == 1 ? "admin" : "user");
    names.push_back(name_text);
    surnames.push_back(surname_text);
    ages.push_back(age);
    students.push_back(is_student);
    save_file();
    message = "Анкету збережено: " + student_info(logins.size() - 1);
    name_text = surname_text = age_text = "";
    is_student = false;
}

// Шукаємо анкету з таким логіном і порівнюємо хеш пароля
void login() {
    for (int i = 0; i < (int)logins.size(); i++) {
        if (logins[i] == login_text && passwords[i] == hash_password(login_text, password_text)) {
            user = i;
            message = "Ви увійшли як " + names[i] + " (" + roles[i] + ")";
            return;
        }
    }
    message = "Невірний логін або пароль";
}

// Шрифт з кирилицею, бо стандартний шрифт raylib знає тільки латиницю
Font load_font() {
    int codepoints[352];
    int count = 0;
    for (int c = 32; c < 128; c++) codepoints[count++] = c;
    for (int c = 0x400; c < 0x500; c++) codepoints[count++] = c;
    const char *path = "/System/Library/Fonts/Supplemental/Arial.ttf";
    if (!FileExists(path)) path = "C:/Windows/Fonts/arial.ttf";
    return LoadFontEx(path, 40, codepoints, count);
}

void text(string s, float x, float y) {
    DrawTextEx(font, s.c_str(), {x, y}, 20, 0, DARKGRAY);
}

bool clicked(Rectangle r) {
    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), r);
}

bool button(string label, float x, float y, float width) {
    Rectangle r = {x, y, width, 36};
    DrawRectangleRec(r, SKYBLUE);
    text(label, x + 12, y + 8);
    return clicked(r);
}

// Поле введення: клік по ньому вмикає введення, пароль показуємо зірочками
void input(string label, string &value, int id, float x, float y, bool hidden = false) {
    Rectangle r = {x, y + 24, 220, 36};
    if (clicked(r)) focus = id;
    if (focus == id) {
        int c;
        while ((c = GetCharPressed()) > 0) {
            int size;
            const char *bytes = CodepointToUTF8(c, &size);
            value += string(bytes, size);
        }
        // Українська літера займає 2 байти, тому стираємо всі байти останньої літери
        if (IsKeyPressed(KEY_BACKSPACE) && value != "") {
            while ((value.back() & 0xC0) == 0x80) value.pop_back();
            value.pop_back();
        }
    }
    text(label, x, y);
    DrawRectangleRec(r, WHITE);
    DrawRectangleLinesEx(r, 2, focus == id ? BLUE : LIGHTGRAY);
    text(hidden ? string(value.size(), '*') : value, x + 8, y + 32);
}

int main() {
    InitWindow(900, 640, "Анкетування");
    SetTargetFPS(60);
    font = load_font();
    load_file();

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        input("Логін", login_text, 0, 30, 20);
        input("Пароль", password_text, 1, 270, 20, true);
        if (button("Увійти", 510, 44, 150)) login();
        if (button("Вийти", 680, 44, 150)) {
            user = -1;
            message = "Ви вийшли";
        }

        input("Ім'я", name_text, 2, 30, 100);
        input("Прізвище", surname_text, 3, 270, 100);
        input("Вік", age_text, 4, 510, 100);

        Rectangle box = {30, 186, 26, 26};
        if (clicked(box)) is_student = !is_student;
        DrawRectangleLinesEx(box, 2, GRAY);
        if (is_student) DrawRectangle(36, 192, 14, 14, BLUE);
        text("Студент", 66, 188);
        if (button("Зареєструватися", 270, 180, 220)) register_student();

        DrawLine(30, 236, 870, 236, LIGHTGRAY);
        if (user == -1) {
            text("Увійдіть, щоб побачити анкети", 30, 256);
        } else if (roles[user] == "user") {
            text("Ваші дані:", 30, 256);
            text(student_info(user), 30, 290);
        } else {
            // Адміністратор бачить усі анкети або тільки з введеним ім'ям
            input("Пошук за ім'ям (порожньо = усі)", search_text, 5, 30, 250);
            float y = 330;
            for (int i = 0; i < (int)logins.size(); i++) {
                if (search_text == "" || names[i] == search_text) {
                    text(student_info(i), 30, y);
                    y += 28;
                }
            }
        }

        text(message, 30, 600);
        EndDrawing();
    }
    CloseWindow();
}
