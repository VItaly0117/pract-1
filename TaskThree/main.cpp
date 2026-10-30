#include <iostream>

#include "anketa.h"

Registry registry;

std::string ask(const std::string &prompt) {
    std::string line;
    while (line.empty()) {
        std::cout << prompt;
        if (!std::getline(std::cin, line)) std::exit(0);
    }
    return line;
}

void create(const std::string &role) {
    Student s;
    s.role = role;
    s.login = ask("Логін: ");
    std::string password = ask("Пароль: ");
    s.name = ask("Ім'я: ");
    s.surname = ask("Прізвище: ");
    s.age = parse_age(ask("Вік: "));
    s.is_student = ask("Чи є студентом? (так/ні): ") == "так";
    std::string error = registry.add(s, password);
    std::cout << (error.empty() ? "Анкету збережено: " + card(registry.students.back()) : "Помилка: " + error) << "\n";
}

void show(const std::string &name) {
    std::vector<Student> found = registry.filter(name);
    for (const Student &s : found) std::cout << card(s) << "\n";
    if (found.empty()) std::cout << "Анкет не знайдено\n";
}

void session(const Student user) {
    bool admin = user.role == "admin";
    while (true) {
        std::cout << "\n— " << user.name << " " << user.surname << " (" << user.role << ") —\n1. Мої дані\n";
        if (admin) std::cout << "2. Створити користувача\n3. Усі анкети\n4. Пошук за ім'ям\n";
        std::string choice = ask("0. Вийти з акаунта\n> ");
        if (choice == "0") return;
        if (choice == "1") std::cout << card(user) << "\n";
        if (!admin) continue;
        if (choice == "2") create(ask("Надати права адміністратора? (так/ні): ") == "так" ? "admin" : "user");
        if (choice == "3") show("");
        if (choice == "4") show(ask("Ім'я: "));
    }
}

int main() {
    if (registry.load()) {
        std::cout << "Файл " << DATA_FILE << " не знайдено — створено новий\n";
    } else {
        std::cout << "Завантажено анкет: " << registry.students.size() << "\n";
    }
    while (!registry.has_admin()) {
        std::cout << "Створіть обліковий запис адміністратора\n";
        create("admin");
    }
    while (true) {
        std::string choice = ask("\n— Анкетування —\n1. Увійти\n2. Зареєструватися\n0. Завершити\n> ");
        if (choice == "0") return 0;
        if (choice == "2") create("user");
        if (choice == "1") {
            std::string login = ask("Логін: ");
            Student *user = registry.login(login, ask("Пароль: "));
            if (user) {
                session(*user);
            } else {
                std::cout << "Невірний логін або пароль\n";
            }
        }
    }
}
