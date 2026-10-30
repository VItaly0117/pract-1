import hashlib
import hmac
import json
import os
from dataclasses import asdict, dataclass
from pathlib import Path

DATA_FILE = Path(__file__).with_name("students.json")
ADMIN = "admin"
USER = "user"


@dataclass
class Student:
    login: str
    password: str
    role: str
    name: str
    surname: str
    age: int
    is_student: bool

    def card(self) -> str:
        status = "так" if self.is_student else "ні"
        return f"{self.name} {self.surname} | вік: {self.age} | студент: {status} | роль: {self.role} | логін: {self.login}"


def hash_password(password: str, salt: bytes | None = None) -> str:
    salt = salt or os.urandom(16)
    digest = hashlib.pbkdf2_hmac("sha256", password.encode(), salt, 200_000)
    return f"{salt.hex()}${digest.hex()}"


def check_password(password: str, stored: str) -> bool:
    salt = bytes.fromhex(stored.split("$")[0])
    return hmac.compare_digest(hash_password(password, salt), stored)


def parse_age(text: str) -> int:
    if not text.strip().isdigit() or not 1 <= int(text) <= 120:
        raise ValueError("Вік має бути числом від 1 до 120")
    return int(text)


class Registry:
    def __init__(self, path: Path = DATA_FILE):
        self.path = path
        self.created = not path.exists()
        if self.created:
            self.students: list[Student] = []
            self.save()
        else:
            self.students = [Student(**item) for item in json.loads(path.read_text(encoding="utf-8"))]

    def save(self) -> None:
        data = [asdict(student) for student in self.students]
        self.path.write_text(json.dumps(data, ensure_ascii=False, indent=2), encoding="utf-8")

    def find(self, login: str) -> Student | None:
        return next((student for student in self.students if student.login == login), None)

    def has_admin(self) -> bool:
        return any(student.role == ADMIN for student in self.students)

    def check_credentials(self, login: str, password: str) -> None:
        if not login or " " in login:
            raise ValueError("Логін не може бути порожнім або містити пробіли")
        if self.find(login):
            raise ValueError(f"Логін «{login}» вже зайнятий")
        if len(password) < 4:
            raise ValueError("Пароль має містити щонайменше 4 символи")

    def add(self, login: str, password: str, role: str, name: str, surname: str, age: int, is_student: bool) -> Student:
        login, name, surname = login.strip(), name.strip(), surname.strip()
        self.check_credentials(login, password)
        if not name or not surname:
            raise ValueError("Ім'я та прізвище обов'язкові")
        student = Student(login, hash_password(password), role, name, surname, age, is_student)
        self.students.append(student)
        self.save()
        return student

    def authenticate(self, login: str, password: str) -> Student | None:
        student = self.find(login.strip())
        return student if student and check_password(password, student.password) else None

    def by_name(self, name: str) -> list[Student]:
        query = name.strip().casefold()
        return [student for student in self.students if not query or student.name.casefold() == query]
