import tkinter as tk
from tkinter import ttk

from game import HIDDEN, MAX_MISSES, Game

CELL = {"width": 2, "font": ("Menlo", 30, "bold"), "relief": "solid", "bd": 1, "fg": "#222222"}
COLORS = {"hidden": "#ffffff", "selected": "#ffe08a", "open": "#c6efc6", "missed": "#f6c4c4"}


class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Вгадай слово")
        self.resizable(False, False)
        self.configure(padx=28, pady=22)
        self.status, self.counter = tk.StringVar(), tk.StringVar()
        ttk.Label(self, text="Вгадай слово", font=("Helvetica", 22, "bold")).pack()
        ttk.Label(self, text="Оберіть клітинку мишею та натисніть літеру на клавіатурі").pack(pady=(4, 16))
        self.board = tk.Frame(self)
        self.board.pack()
        ttk.Label(self, textvariable=self.counter, font=("Helvetica", 14)).pack(pady=(16, 2))
        ttk.Label(self, textvariable=self.status, font=("Helvetica", 14, "bold")).pack()
        ttk.Button(self, text="Нова гра", command=self.new_game, takefocus=False).pack(pady=(14, 0))
        self.bind("<Key>", self.on_key)
        self.new_game()

    def new_game(self) -> None:
        self.game, self.selected = Game(), 0
        for widget in self.board.winfo_children():
            widget.destroy()
        self.cells = []
        for position in range(len(self.game.word)):
            cell = tk.Label(self.board, **CELL)
            cell.grid(row=0, column=position, padx=4, ipadx=6, ipady=4)
            cell.bind("<Button-1>", lambda _, index=position: self.select(index))
            tk.Label(self.board, text=position, fg="#888888").grid(row=1, column=position)
            self.cells.append(cell)
        self.status.set("")
        self.render()

    def select(self, position: int) -> None:
        if not self.game.over:
            self.selected = position
            self.render()

    def on_key(self, event: tk.Event) -> None:
        letter = event.char
        if self.game.over or len(letter) != 1 or not letter.isalpha():
            return
        if self.game.guess(letter, self.selected):
            self.status.set("Вгадали!")
            hidden = [i for i, char in enumerate(self.game.mask) if char == HIDDEN]
            self.selected = next((i for i in hidden if i > self.selected), hidden[0] if hidden else self.selected)
        else:
            self.status.set(f"Літери «{letter}» немає на позиції {self.selected}")
        if self.game.won:
            self.status.set("Перемога! Слово вгадано")
        elif self.game.lost:
            self.status.set(f"Спроби вичерпано. Загадане слово: {''.join(self.game.word)}")
        self.render()

    def render(self) -> None:
        for position, cell in enumerate(self.cells):
            char = self.game.mask[position]
            if char != HIDDEN:
                cell.configure(text=char, bg=COLORS["open"])
            elif self.game.lost:
                cell.configure(text=self.game.word[position], bg=COLORS["missed"])
            else:
                cell.configure(text=HIDDEN, bg=COLORS["selected" if position == self.selected else "hidden"])
        self.counter.set(f"Невдалі спроби: {self.game.misses} з {MAX_MISSES}")


if __name__ == "__main__":
    App().mainloop()
