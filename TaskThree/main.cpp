#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// Одна анкета
struct Student {
    string login;
    string password;
    string role;
    string name;
    string surname;
    int age;
    bool is_student;
};

// Тут лежать усі анкети, поки програма працює
vector<Student> students;

// Пароль у файл не пишемо, тільки його хеш. Логін додаємо, щоб однакові паролі давали різний хеш
string hash_password(string login, string password) {
    return to_string(hash<string>{}(login + password));
}

// Читаємо анкети з файлу. Якщо файлу ще немає, створюємо порожній
void load_file() {
    ifstream file("students.csv");
    if (!file) {
        cout << "Файл students.csv не знайдено, створено новий\n";
        ofstream("students.csv");
        return;
    }
    // Кожен рядок файлу це одна анкета, поля розділені комами
    string line;
    while (getline(file, line)) {
        stringstream row(line);
        Student s;
        string age, is_student;
        getline(row, s.login, ',');
        getline(row, s.password, ',');
        getline(row, s.role, ',');
        getline(row, s.name, ',');
        getline(row, s.surname, ',');
        getline(row, age, ',');
        getline(row, is_student, ',');
        s.age = stoi(age);
        s.is_student = is_student == "1";
        students.push_back(s);
    }
}

// Перезаписуємо файл усіма анкетами
void save_file() {
    ofstream file("students.csv");
    for (Student s : students) {
        file << s.login << "," << s.password << "," << s.role << "," << s.name << "," << s.surname << ","
             << s.age << "," << s.is_student << "\n";
    }
}

// Виводимо одну анкету в один рядок
void print_student(Student s) {
    cout << s.name << " " << s.surname << ", вік: " << s.age << ", студент: " << (s.is_student ? "так" : "ні")
         << ", роль: " << s.role << "\n";
}

// Питаємо дані нової анкети і одразу зберігаємо у файл
void register_student(string role) {
    Student s;
    string password, answer;
    cout << "Логін: ";
    cin >> s.login;
    cout << "Пароль: ";
    cin >> password;
    cout << "Ім'я: ";
    cin >> s.name;
    cout << "Прізвище: ";
    cin >> s.surname;
    cout << "Вік: ";
    cin >> s.age;
    cout << "Чи є студентом? (так/ні): ";
    cin >> answer;
    s.is_student = answer == "так";
    s.role = role;
    s.password = hash_password(s.login, password);
    students.push_back(s);
    save_file();
    cout << "Анкету збережено: ";
    print_student(s);
}

// Показуємо анкети з таким ім'ям. Якщо ім'я порожнє, показуємо всі
void find_by_name(string name) {
    for (Student s : students) {
        if (name == "" || s.name == name) {
            print_student(s);
        }
    }
}

// Меню, яке бачить тільки адміністратор
void admin_menu() {
    string choice, name;
    while (true) {
        cout << "\n1. Усі анкети\n2. Пошук за ім'ям\n3. Створити користувача\n0. Вийти\n> ";
        cin >> choice;
        if (choice == "1") {
            find_by_name("");
        } else if (choice == "2") {
            cout << "Ім'я: ";
            cin >> name;
            find_by_name(name);
        } else if (choice == "3") {
            register_student("user");
        } else if (choice == "0") {
            return;
        }
    }
}

// Вхід: шукаємо анкету з таким логіном і порівнюємо хеш пароля
void login() {
    string login, password;
    cout << "Логін: ";
    cin >> login;
    cout << "Пароль: ";
    cin >> password;
    for (Student s : students) {
        if (s.login == login && s.password == hash_password(login, password)) {
            if (s.role == "admin") {
                admin_menu();
            } else {
                print_student(s);
            }
            return;
        }
    }
    cout << "Невірний логін або пароль\n";
}

int main() {
    load_file();
    // Якщо анкет ще немає, спочатку створюємо адміністратора
    if (students.empty()) {
        cout << "Створіть адміністратора\n";
        register_student("admin");
    }
    string choice;
    while (true) {
        cout << "\n1. Увійти\n2. Зареєструватися\n0. Завершити\n> ";
        cin >> choice;
        if (choice == "1") {
            login();
        } else if (choice == "2") {
            register_student("user");
        } else if (choice == "0") {
            return 0;
        }
    }
}
