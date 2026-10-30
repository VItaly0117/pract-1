import tkinter as tk
from tkinter import messagebox, ttk

from core import ADMIN, USER, Registry, Student, parse_age

FIELDS = {"login": "Логін", "password": "Пароль", "name": "Ім'я", "surname": "Прізвище", "age": "Вік"}
COLUMNS = {
    "login": "Логін",
    "name": "Ім'я",
    "surname": "Прізвище",
    "age": "Вік",
    "is_student": "Студент",
    "role": "Роль",
}


def yes_no(value: bool) -> str:
    return "так" if value else "ні"


class ProfileDialog(tk.Toplevel):
    def __init__(self, app: "App", title: str, role: str | None):
        super().__init__(app)
        self.app, self.role, self.result = app, role, None
        self.title(title)
        self.resizable(False, False)
        self.transient(app)
        self.values = {key: tk.StringVar() for key in FIELDS}
        self.is_student = tk.BooleanVar(value=True)
        self.is_admin = tk.BooleanVar()
        form = ttk.Frame(self, padding=20)
        form.pack()
        for row, (key, label) in enumerate(FIELDS.items()):
            ttk.Label(form, text=label).grid(row=row, column=0, sticky="w", padx=(0, 12), pady=4)
            entry = ttk.Entry(form, textvariable=self.values[key], width=28, show="•" if key == "password" else "")
            entry.grid(row=row, column=1, pady=4)
            if row == 0:
                entry.focus_set()
        ttk.Checkbutton(form, text="Студент", variable=self.is_student).grid(row=5, column=1, sticky="w", pady=4)
        if role is None:
            ttk.Checkbutton(form, text="Адміністратор", variable=self.is_admin).grid(row=6, column=1, sticky="w")
        ttk.Button(form, text="Зберегти", command=self.submit).grid(row=7, column=1, sticky="e", pady=(14, 0))
        self.bind("<Return>", lambda _: self.submit())
        self.grab_set()

    def submit(self) -> None:
        data = {key: value.get() for key, value in self.values.items()}
        role = self.role or (ADMIN if self.is_admin.get() else USER)
        try:
            age = parse_age(data["age"])
            self.result = self.app.registry.add(
                data["login"], data["password"], role, data["name"], data["surname"], age, self.is_student.get()
            )
        except ValueError as error:
            messagebox.showerror("Помилка", str(error), parent=self)
            return
        self.destroy()


class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Анкетування")
        self.geometry("720x440")
        self.minsize(620, 380)
        self.registry = Registry()
        self.body = ttk.Frame(self, padding=20)
        self.body.pack(fill="both", expand=True)
        self.after(100, self.start)

    def start(self) -> None:
        if self.registry.created:
            messagebox.showinfo(
                "Новий файл", f"Файл {self.registry.path.name} не знайдено — створено новий", parent=self
            )
        if not self.registry.has_admin() and not self.ask_profile("Створення адміністратора", ADMIN):
            self.destroy()
            return
        self.show_login()

    def ask_profile(self, title: str, role: str | None) -> Student | None:
        dialog = ProfileDialog(self, title, role)
        self.wait_window(dialog)
        return dialog.result

    def clear(self) -> None:
        for widget in self.body.winfo_children():
            widget.destroy()

    def header(self, text: str) -> None:
        bar = ttk.Frame(self.body)
        bar.pack(fill="x", pady=(0, 16))
        ttk.Label(bar, text=text, font=("Helvetica", 18, "bold")).pack(side="left")
        ttk.Button(bar, text="Вийти", command=self.show_login).pack(side="right")

    def show_login(self) -> None:
        self.clear()
        form = ttk.Frame(self.body)
        form.place(relx=0.5, rely=0.45, anchor="center")
        ttk.Label(form, text="Вхід до системи", font=("Helvetica", 20, "bold")).grid(columnspan=2, pady=(0, 16))
        login, password = tk.StringVar(), tk.StringVar()
        for row, (label, var, show) in enumerate((("Логін", login, ""), ("Пароль", password, "•")), start=1):
            ttk.Label(form, text=label).grid(row=row, column=0, sticky="w", padx=(0, 12), pady=4)
            entry = ttk.Entry(form, textvariable=var, width=26, show=show)
            entry.grid(row=row, column=1, pady=4)
            if row == 1:
                entry.focus_set()
        buttons = ttk.Frame(form)
        buttons.grid(row=3, column=1, sticky="e", pady=(14, 0))
        ttk.Button(buttons, text="Реєстрація", command=self.register).pack(side="left", padx=(0, 8))
        ttk.Button(buttons, text="Увійти", command=lambda: self.login(login.get(), password.get())).pack(side="left")
        self.bind("<Return>", lambda _: self.login(login.get(), password.get()))

    def login(self, login: str, password: str) -> None:
        user = self.registry.authenticate(login, password)
        if not user:
            messagebox.showerror("Помилка", "Невірний логін або пароль", parent=self)
        elif user.role == ADMIN:
            self.show_admin(user)
        else:
            self.show_user(user)

    def register(self) -> None:
        if student := self.ask_profile("Реєстрація", USER):
            messagebox.showinfo("Анкету збережено", student.card(), parent=self)

    def show_user(self, user: Student) -> None:
        self.clear()
        self.unbind("<Return>")
        self.header(f"Вітаємо, {user.name}!")
        ttk.Label(self.body, text="Мої дані", font=("Helvetica", 15, "bold")).pack(anchor="w", pady=(0, 8))
        card = ttk.Frame(self.body)
        card.pack(anchor="w")
        rows = (
            ("Логін", user.login),
            ("Ім'я", user.name),
            ("Прізвище", user.surname),
            ("Вік", user.age),
            ("Студент", yes_no(user.is_student)),
            ("Роль", user.role),
        )
        for row, (label, value) in enumerate(rows):
            ttk.Label(card, text=f"{label}:").grid(row=row, column=0, sticky="w", padx=(0, 16), pady=3)
            ttk.Label(card, text=str(value), font=("Helvetica", 13, "bold")).grid(row=row, column=1, sticky="w")

    def show_admin(self, user: Student) -> None:
        self.clear()
        self.unbind("<Return>")
        self.header(f"Адміністратор: {user.name} {user.surname}")
        tools = ttk.Frame(self.body)
        tools.pack(fill="x", pady=(0, 10))
        query = tk.StringVar()
        ttk.Label(tools, text="Фільтр за ім'ям:").pack(side="left")
        ttk.Entry(tools, textvariable=query, width=22).pack(side="left", padx=8)
        ttk.Button(tools, text="Створити користувача", command=lambda: self.create_user(query)).pack(side="right")
        self.table = ttk.Treeview(self.body, columns=list(COLUMNS), show="headings")
        for key, title in COLUMNS.items():
            self.table.heading(key, text=title)
            self.table.column(key, width=70 if key in ("age", "is_student", "role") else 120, anchor="w")
        self.table.pack(fill="both", expand=True)
        query.trace_add("write", lambda *_: self.fill(query.get()))
        self.fill("")

    def fill(self, query: str) -> None:
        self.table.delete(*self.table.get_children())
        for student in self.registry.by_name(query):
            row = (student.login, student.name, student.surname, student.age, yes_no(student.is_student), student.role)
            self.table.insert("", "end", values=row)

    def create_user(self, query: tk.StringVar) -> None:
        if self.ask_profile("Новий користувач", None):
            self.fill(query.get())


if __name__ == "__main__":
    App().mainloop()
