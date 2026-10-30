from getpass import getpass

from core import ADMIN, USER, Registry, Student, parse_age


def ask(prompt: str) -> str:
    while not (value := input(prompt).strip()):
        print("Поле не може бути порожнім")
    return value


def ask_yes(prompt: str) -> bool:
    while (answer := input(f"{prompt} (так/ні): ").strip().casefold()) not in ("так", "ні"):
        print("Введіть «так» або «ні»")
    return answer == "так"


def ask_age() -> int:
    while True:
        try:
            return parse_age(input("Вік: "))
        except ValueError as error:
            print(error)


def ask_credentials(registry: Registry) -> tuple[str, str]:
    while True:
        login, password = ask("Логін: "), getpass("Пароль: ")
        try:
            registry.check_credentials(login, password)
            return login, password
        except ValueError as error:
            print(error)


def ask_profile(registry: Registry, role: str) -> Student:
    login, password = ask_credentials(registry)
    name = ask("Ім'я: ")
    surname = ask("Прізвище: ")
    return registry.add(login, password, role, name, surname, ask_age(), ask_yes("Чи є студентом?"))


def show(students: list[Student]) -> None:
    if not students:
        print("Анкет не знайдено")
    for student in students:
        print(student.card())


def menu(title: str, actions: dict, back: str = "Вийти з акаунта") -> None:
    while True:
        print(f"\n— {title} —")
        for key, (label, _) in actions.items():
            print(f"{key}. {label}")
        print(f"0. {back}")
        choice = input("> ").strip()
        if choice == "0":
            return
        if choice in actions:
            actions[choice][1]()


def create_user(registry: Registry) -> None:
    role = ADMIN if ask_yes("Надати права адміністратора?") else USER
    print("Створено:", ask_profile(registry, role).card())


def session(registry: Registry, user: Student) -> None:
    actions = {"1": ("Мої дані", lambda: print(user.card()))}
    if user.role == ADMIN:
        actions |= {
            "2": ("Створити користувача", lambda: create_user(registry)),
            "3": ("Усі анкети", lambda: show(registry.students)),
            "4": ("Пошук за ім'ям", lambda: show(registry.by_name(ask("Ім'я: ")))),
        }
    menu(f"{user.name} {user.surname} ({user.role})", actions)


def login(registry: Registry) -> None:
    user = registry.authenticate(ask("Логін: "), getpass("Пароль: "))
    if user:
        session(registry, user)
    else:
        print("Невірний логін або пароль")


def register(registry: Registry) -> None:
    print("Анкету збережено:", ask_profile(registry, USER).card())


def main() -> None:
    registry = Registry()
    if registry.created:
        print(f"Файл {registry.path.name} не знайдено — створено новий")
    else:
        print(f"Завантажено анкет: {len(registry.students)}")
    if not registry.has_admin():
        print("Створіть обліковий запис адміністратора")
        ask_profile(registry, ADMIN)
    actions = {"1": ("Увійти", lambda: login(registry)), "2": ("Зареєструватися", lambda: register(registry))}
    menu("Анкетування", actions, "Завершити")


if __name__ == "__main__":
    main()
