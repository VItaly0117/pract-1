#pragma once
#include <fstream>
#include <functional>
#include <random>
#include <sstream>
#include <string>
#include <vector>

const std::string DATA_FILE = "students.csv";

struct Student {
    std::string login, password, role, name, surname;
    int age = 0;
    bool is_student = false;
};

inline std::string hash_password(const std::string &password, const std::string &salt) {
    std::string hash = salt + password;
    for (int i = 0; i < 10000; i++) hash = std::to_string(std::hash<std::string>{}(hash + salt));
    return salt + "$" + hash;
}

inline int parse_age(const std::string &text) {
    try {
        size_t used;
        int age = std::stoi(text, &used);
        return used == text.size() && age >= 1 && age <= 120 ? age : -1;
    } catch (...) {
        return -1;
    }
}

inline std::string card(const Student &s) {
    return s.name + " " + s.surname + " | вік: " + std::to_string(s.age) + " | студент: " + (s.is_student ? "так" : "ні") +
           " | роль: " + s.role + " | логін: " + s.login;
}

struct Registry {
    std::vector<Student> students;

    bool load() {
        std::ifstream file(DATA_FILE);
        if (!file) {
            save();
            return true;
        }
        std::string line, age, status;
        std::getline(file, line);
        while (std::getline(file, line)) {
            std::stringstream row(line);
            Student s;
            for (std::string *field : {&s.login, &s.password, &s.role, &s.name, &s.surname, &age}) std::getline(row, *field, ',');
            std::getline(row, status);
            s.age = parse_age(age);
            s.is_student = status == "true";
            students.push_back(s);
        }
        return false;
    }

    void save() const {
        std::ofstream file(DATA_FILE);
        file << "login,password,role,name,surname,age,is_student\n";
        for (const Student &s : students) {
            file << s.login << ',' << s.password << ',' << s.role << ',' << s.name << ',' << s.surname << ',' << s.age << ','
                 << (s.is_student ? "true" : "false") << '\n';
        }
    }

    Student *find(const std::string &login) {
        for (Student &s : students) {
            if (s.login == login) return &s;
        }
        return nullptr;
    }

    bool has_admin() const {
        for (const Student &s : students) {
            if (s.role == "admin") return true;
        }
        return false;
    }

    std::string add(Student s, const std::string &password) {
        if (s.login.empty() || s.login.find_first_of(" ,") != std::string::npos) return "Логін не може бути порожнім або містити пробіли та коми";
        if (find(s.login)) return "Такий логін вже зайнятий";
        if (password.size() < 4) return "Пароль має містити щонайменше 4 символи";
        if (s.name.empty() || s.surname.empty()) return "Ім'я та прізвище обов'язкові";
        if ((s.name + s.surname).find(',') != std::string::npos) return "Ім'я та прізвище не можуть містити коми";
        if (s.age < 1) return "Вік має бути числом від 1 до 120";
        s.password = hash_password(password, std::to_string(std::random_device{}()));
        students.push_back(s);
        save();
        return "";
    }

    Student *login(const std::string &login, const std::string &password) {
        Student *s = find(login);
        if (!s) return nullptr;
        std::string salt = s->password.substr(0, s->password.find('$'));
        return hash_password(password, salt) == s->password ? s : nullptr;
    }

    std::vector<Student> filter(const std::string &name) const {
        std::vector<Student> found;
        for (const Student &s : students) {
            if (s.name.find(name) != std::string::npos) found.push_back(s);
        }
        return found;
    }
};
