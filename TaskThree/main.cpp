#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

const string FILE_NAME = "students.csv";

struct Student {
    string login;
    string password;
    string role;
    string name;
    string surname;
    int age;
    bool is_student;
};

vector<Student> students;

string hash_password(string login, string password) {
    return to_string(hash<string>{}(login + password));
}

bool load_file() {
    ifstream file(FILE_NAME);
    if (!file) {
        return false;
    }
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
    return true;
}

void save_file() {
    ofstream file(FILE_NAME);
    for (Student s : students) {
        file << s.login << "," << s.password << "," << s.role << "," << s.name << "," << s.surname << ","
             << s.age << "," << s.is_student << "\n";
    }
}

int find_student(string login) {
    for (int i = 0; i < (int)students.size(); i++) {
        if (students[i].login == login) {
            return i;
        }
    }
    return -1;
}

void print_student(Student s) {
    cout << s.name << " " << s.surname << ", вік: " << s.age << ", студент: " << (s.is_student ? "так" : "ні")
         << ", роль: " << s.role << ", логін: " << s.login << "\n";
}

int read_age() {
    int age;
    while (!(cin >> age) || age < 1 || age > 120) {
        if (cin.eof()) {
            exit(0);
        }
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Вік має бути числом від 1 до 120: ";
    }
    return age;
}

void register_student(string role) {
    Student s;
    string password, answer;
    cout << "Логін: ";
    cin >> s.login;
    if (find_student(s.login) != -1) {
        cout << "Такий логін вже зайнятий\n";
        return;
    }
    cout << "Пароль: ";
    cin >> password;
    if (password.size() < 4) {
        cout << "Пароль має містити щонайменше 4 символи\n";
        return;
    }
    cout << "Ім'я: ";
    cin >> s.name;
    cout << "Прізвище: ";
    cin >> s.surname;
    cout << "Вік: ";
    s.age = read_age();
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

void show_all() {
    for (Student s : students) {
        print_student(s);
    }
}

void find_by_name() {
    string name;
    cout << "Ім'я: ";
    cin >> name;
    int found = 0;
    for (Student s : students) {
        if (s.name == name) {
            print_student(s);
            found++;
        }
    }
    if (found == 0) {
        cout << "Анкет не знайдено\n";
    }
}

void admin_menu() {
    string choice;
    while (true) {
        cout << "\n1. Усі анкети\n2. Пошук за ім'ям\n3. Створити користувача\n4. Створити адміністратора\n"
                "0. Вийти з акаунта\n> ";
        if (!(cin >> choice) || choice == "0") {
            return;
        }
        if (choice == "1") {
            show_all();
        } else if (choice == "2") {
            find_by_name();
        } else if (choice == "3") {
            register_student("user");
        } else if (choice == "4") {
            register_student("admin");
        }
    }
}

void login() {
    string login, password;
    cout << "Логін: ";
    cin >> login;
    cout << "Пароль: ";
    cin >> password;
    int i = find_student(login);
    if (i == -1 || students[i].password != hash_password(login, password)) {
        cout << "Невірний логін або пароль\n";
        return;
    }
    cout << "Вітаємо, " << students[i].name << "!\n";
    if (students[i].role == "admin") {
        admin_menu();
    } else {
        cout << "Ваші дані: ";
        print_student(students[i]);
    }
}

int main() {
    if (load_file()) {
        cout << "Завантажено анкет: " << students.size() << "\n";
    } else {
        cout << "Файл " << FILE_NAME << " не знайдено, створено новий\n";
        save_file();
    }
    while (students.empty()) {
        cout << "Створіть обліковий запис адміністратора\n";
        register_student("admin");
    }
    string choice;
    while (true) {
        cout << "\n1. Увійти\n2. Зареєструватися\n0. Завершити\n> ";
        if (!(cin >> choice) || choice == "0") {
            return 0;
        }
        if (choice == "1") {
            login();
        } else if (choice == "2") {
            register_student("user");
        }
    }
}
