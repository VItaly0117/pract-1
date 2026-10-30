#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "anketa.h"

static Registry registry;

static void read_line(const char *prompt, char *buffer, int size) {
    printf("%s", prompt);
    fflush(stdout);
    if (!fgets(buffer, size, stdin)) {
        exit(EXIT_SUCCESS);
    }
    buffer[strcspn(buffer, "\n")] = '\0';
    char *start = buffer + strspn(buffer, " \t");
    memmove(buffer, start, strlen(start) + 1);
    for (size_t length = strlen(buffer); length && buffer[length - 1] == ' '; length--) {
        buffer[length - 1] = '\0';
    }
}

static void ask(const char *prompt, char *buffer, int size) {
    read_line(prompt, buffer, size);
    while (buffer[0] == '\0') {
        puts("Поле не може бути порожнім");
        read_line(prompt, buffer, size);
    }
}

static void ask_password(char *buffer, int size) {
    struct termios saved, hidden;
    bool tty = isatty(STDIN_FILENO) && tcgetattr(STDIN_FILENO, &saved) == 0;
    if (tty) {
        hidden = saved;
        hidden.c_lflag &= ~(tcflag_t)ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &hidden);
    }
    read_line("Пароль: ", buffer, size);
    if (tty) {
        tcsetattr(STDIN_FILENO, TCSANOW, &saved);
        putchar('\n');
    }
}

static bool ask_yes(const char *question) {
    char prompt[128], answer[16];
    snprintf(prompt, sizeof prompt, "%s (так/ні): ", question);
    for (;;) {
        read_line(prompt, answer, sizeof answer);
        if (strcmp(answer, "так") == 0) return true;
        if (strcmp(answer, "ні") == 0) return false;
        puts("Введіть «так» або «ні»");
    }
}

static int ask_age(void) {
    char text[16];
    for (;;) {
        read_line("Вік: ", text, sizeof text);
        int age = parse_age(text);
        if (age > 0) return age;
        puts("Вік має бути числом від 1 до 120");
    }
}

static Student *ask_profile(Role role) {
    Student profile = {.role = role};
    char password[FIELD_SIZE];
    for (;;) {
        ask("Логін: ", profile.login, FIELD_SIZE);
        ask_password(password, FIELD_SIZE);
        const char *error = registry_check_credentials(&registry, profile.login, password);
        if (!error) break;
        puts(error);
    }
    ask("Ім'я: ", profile.name, FIELD_SIZE);
    ask("Прізвище: ", profile.surname, FIELD_SIZE);
    profile.age = ask_age();
    profile.is_student = ask_yes("Чи є студентом?");
    const char *error = registry_add(&registry, &profile, password);
    if (error) {
        printf("Помилка: %s\n", error);
        return NULL;
    }
    return &registry.items[registry.count - 1];
}

static void print_card(const char *title, const Student *student) {
    char card[320];
    student_card(student, card, sizeof card);
    printf("%s%s\n", title, card);
}

static void show(const char *query) {
    int found = 0;
    for (int i = 0; i < registry.count; i++) {
        if (student_name_matches(&registry.items[i], query)) {
            print_card("", &registry.items[i]);
            found++;
        }
    }
    if (found == 0) puts("Анкет не знайдено");
}

static int menu(const char *title, const char *items[], int count, const char *back) {
    char choice[16];
    for (;;) {
        printf("\n— %s —\n", title);
        for (int i = 0; i < count; i++) {
            printf("%d. %s\n", i + 1, items[i]);
        }
        printf("0. %s\n", back);
        read_line("> ", choice, sizeof choice);
        char *end;
        long number = strtol(choice, &end, 10);
        if (end != choice && *end == '\0' && number >= 0 && number <= count) return (int)number;
    }
}

static void session(Student *user) {
    static const char *items[] = {"Мої дані", "Створити користувача", "Усі анкети", "Пошук за ім'ям"};
    char title[160], query[FIELD_SIZE];
    snprintf(title, sizeof title, "%s %s (%s)", user->name, user->surname, role_name(user->role));
    int count = user->role == ROLE_ADMIN ? 4 : 1, choice;
    while ((choice = menu(title, items, count, "Вийти з акаунта")) != 0) {
        Student *created;
        switch (choice) {
        case 1:
            print_card("", user);
            break;
        case 2:
            created = ask_profile(ask_yes("Надати права адміністратора?") ? ROLE_ADMIN : ROLE_USER);
            if (created) print_card("Створено: ", created);
            break;
        case 3:
            show("");
            break;
        case 4:
            ask("Ім'я: ", query, FIELD_SIZE);
            show(query);
            break;
        }
    }
}

static void login(void) {
    char name[FIELD_SIZE], password[FIELD_SIZE];
    ask("Логін: ", name, FIELD_SIZE);
    ask_password(password, FIELD_SIZE);
    Student *user = registry_authenticate(&registry, name, password);
    if (user) {
        session(user);
    } else {
        puts("Невірний логін або пароль");
    }
}

int main(void) {
    if (registry_load(&registry)) {
        printf("Файл %s не знайдено — створено новий\n", DATA_FILE);
    } else {
        printf("Завантажено анкет: %d\n", registry.count);
    }
    while (!registry_has_admin(&registry)) {
        puts("Створіть обліковий запис адміністратора");
        ask_profile(ROLE_ADMIN);
    }
    static const char *items[] = {"Увійти", "Зареєструватися"};
    int choice;
    while ((choice = menu("Анкетування", items, 2, "Завершити")) != 0) {
        if (choice == 1) {
            login();
        } else {
            Student *student = ask_profile(ROLE_USER);
            if (student) print_card("Анкету збережено: ", student);
        }
    }
    return 0;
}
