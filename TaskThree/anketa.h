#ifndef ANKETA_H
#define ANKETA_H

#include <stdbool.h>

#define DATA_FILE "students.csv"
#define MAX_STUDENTS 256
#define FIELD_SIZE 64
#define HASH_SIZE 98

typedef enum { ROLE_USER, ROLE_ADMIN } Role;

typedef struct {
    char login[FIELD_SIZE];
    char password[HASH_SIZE];
    Role role;
    char name[FIELD_SIZE];
    char surname[FIELD_SIZE];
    int age;
    bool is_student;
} Student;

typedef struct {
    Student items[MAX_STUDENTS];
    int count;
} Registry;

bool registry_load(Registry *registry);
bool registry_save(const Registry *registry);
Student *registry_find(Registry *registry, const char *login);
bool registry_has_admin(const Registry *registry);
const char *registry_check_credentials(Registry *registry, const char *login, const char *password);
const char *registry_add(Registry *registry, const Student *profile, const char *password);
Student *registry_authenticate(Registry *registry, const char *login, const char *password);
bool student_name_matches(const Student *student, const char *query);
const char *role_name(Role role);
void student_card(const Student *student, char *buffer, int size);
int parse_age(const char *text);

#endif
