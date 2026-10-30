#include "anketa.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sha256.h"

#define SALT_BYTES 16
#define ITERATIONS 10000

static void to_hex(const unsigned char *bytes, int count, char *out) {
    for (int i = 0; i < count; i++) {
        sprintf(out + i * 2, "%02x", bytes[i]);
    }
}

static void random_salt(char out[SALT_BYTES * 2 + 1]) {
    unsigned char bytes[SALT_BYTES];
    FILE *source = fopen("/dev/urandom", "rb");
    if (!source || fread(bytes, 1, sizeof bytes, source) != sizeof bytes) {
        srand((unsigned)time(NULL) * 2654435761u);
        for (int i = 0; i < SALT_BYTES; i++) bytes[i] = (unsigned char)rand();
    }
    if (source) fclose(source);
    to_hex(bytes, SALT_BYTES, out);
}

static void hash_password(const char *salt, const char *password, char out[HASH_SIZE]) {
    char input[SALT_BYTES * 2 + FIELD_SIZE];
    int length = snprintf(input, sizeof input, "%s%s", salt, password);
    uint8_t digest[SHA256_SIZE];
    sha256((const uint8_t *)input, (size_t)length, digest);
    for (int i = 1; i < ITERATIONS; i++) {
        sha256(digest, sizeof digest, digest);
    }
    snprintf(out, HASH_SIZE, "%s$", salt);
    to_hex(digest, SHA256_SIZE, out + SALT_BYTES * 2 + 1);
}

static bool check_password(const char *password, const char *stored) {
    char salt[SALT_BYTES * 2 + 1], expected[HASH_SIZE];
    snprintf(salt, sizeof salt, "%s", stored);
    hash_password(salt, password, expected);
    unsigned char diff = 0;
    for (int i = 0; i < HASH_SIZE - 1; i++) {
        diff |= (unsigned char)(expected[i] ^ stored[i]);
    }
    return diff == 0;
}

bool registry_load(Registry *registry) {
    registry->count = 0;
    FILE *file = fopen(DATA_FILE, "r");
    if (!file) {
        registry_save(registry);
        return true;
    }
    char line[512], role[8], status[8];
    while (fgets(line, sizeof line, file) && registry->count < MAX_STUDENTS) {
        Student *s = &registry->items[registry->count];
        int fields = sscanf(line, "%63[^,],%97[^,],%7[^,],%63[^,],%63[^,],%d,%7[a-z]", s->login, s->password, role,
                            s->name, s->surname, &s->age, status);
        if (fields == 7 && strcmp(s->login, "login") != 0) {
            s->role = strcmp(role, "admin") == 0 ? ROLE_ADMIN : ROLE_USER;
            s->is_student = strcmp(status, "true") == 0;
            registry->count++;
        }
    }
    fclose(file);
    return false;
}

bool registry_save(const Registry *registry) {
    FILE *file = fopen(DATA_FILE, "w");
    if (!file) return false;
    fprintf(file, "login,password,role,name,surname,age,is_student\n");
    for (int i = 0; i < registry->count; i++) {
        const Student *s = &registry->items[i];
        fprintf(file, "%s,%s,%s,%s,%s,%d,%s\n", s->login, s->password, role_name(s->role), s->name, s->surname, s->age,
                s->is_student ? "true" : "false");
    }
    return fclose(file) == 0;
}

Student *registry_find(Registry *registry, const char *login) {
    for (int i = 0; i < registry->count; i++) {
        if (strcmp(registry->items[i].login, login) == 0) return &registry->items[i];
    }
    return NULL;
}

bool registry_has_admin(const Registry *registry) {
    for (int i = 0; i < registry->count; i++) {
        if (registry->items[i].role == ROLE_ADMIN) return true;
    }
    return false;
}

const char *registry_check_credentials(Registry *registry, const char *login, const char *password) {
    if (login[0] == '\0' || strpbrk(login, " ,")) return "Логін не може бути порожнім або містити пробіли та коми";
    if (registry_find(registry, login)) return "Такий логін вже зайнятий";
    if (strlen(password) < 4) return "Пароль має містити щонайменше 4 символи";
    return NULL;
}

const char *registry_add(Registry *registry, const Student *profile, const char *password) {
    const char *error = registry_check_credentials(registry, profile->login, password);
    if (error) return error;
    if (profile->name[0] == '\0' || profile->surname[0] == '\0') return "Ім'я та прізвище обов'язкові";
    if (strchr(profile->name, ',') || strchr(profile->surname, ',')) return "Ім'я та прізвище не можуть містити коми";
    if (profile->age < 1 || profile->age > 120) return "Вік має бути числом від 1 до 120";
    if (registry->count == MAX_STUDENTS) return "Досягнуто максимальної кількості анкет";
    Student *student = &registry->items[registry->count++];
    *student = *profile;
    char salt[SALT_BYTES * 2 + 1];
    random_salt(salt);
    hash_password(salt, password, student->password);
    return registry_save(registry) ? NULL : "Не вдалося записати файл";
}

Student *registry_authenticate(Registry *registry, const char *login, const char *password) {
    Student *student = registry_find(registry, login);
    return student && check_password(password, student->password) ? student : NULL;
}

static int next_codepoint(const char **text) {
    const unsigned char *s = (const unsigned char *)*text;
    int codepoint = s[0], size = 1;
    if (s[0] >= 0xE0 && s[1] && s[2]) {
        codepoint = (s[0] & 0x0F) << 12 | (s[1] & 0x3F) << 6 | (s[2] & 0x3F);
        size = 3;
    } else if (s[0] >= 0xC0 && s[1]) {
        codepoint = (s[0] & 0x1F) << 6 | (s[1] & 0x3F);
        size = 2;
    }
    *text += size;
    return codepoint;
}

static int lower_codepoint(int c) {
    if (c >= 'A' && c <= 'Z') return c + 0x20;
    if (c >= 0x410 && c <= 0x42F) return c + 0x20;
    if (c >= 0x400 && c <= 0x40F) return c + 0x50;
    if (c == 0x490) return 0x491;
    return c;
}

bool student_name_matches(const Student *student, const char *query) {
    if (*query == '\0') return true;
    const char *name = student->name;
    while (*name && *query) {
        if (lower_codepoint(next_codepoint(&name)) != lower_codepoint(next_codepoint(&query))) return false;
    }
    return *name == '\0' && *query == '\0';
}

const char *role_name(Role role) {
    return role == ROLE_ADMIN ? "admin" : "user";
}

void student_card(const Student *student, char *buffer, int size) {
    snprintf(buffer, (size_t)size, "%s %s | вік: %d | студент: %s | роль: %s | логін: %s", student->name,
             student->surname, student->age, student->is_student ? "так" : "ні", role_name(student->role),
             student->login);
}

int parse_age(const char *text) {
    char *end;
    long age = strtol(text, &end, 10);
    return end != text && *end == '\0' && age >= 1 && age <= 120 ? (int)age : -1;
}
