import random

WORDS = (
    ("t", "h", "i", "s"),
    ("i", "s"),
    ("a",),
    ("w", "o", "r", "d"),
    ("a", "r", "r", "a", "y"),
    ("m", "a", "t", "r", "i", "x"),
    ("p", "y", "t", "h", "o", "n"),
)
MAX_MISSES = 5
HIDDEN = "*"


class Game:
    def __init__(self, word: tuple[str, ...] | None = None):
        self.word = word or random.choice(WORDS)
        self.mask = [HIDDEN] * len(self.word)
        self.misses = 0

    def guess(self, letter: str, position: int) -> bool:
        hit = self.word[position] == letter.lower()
        if hit:
            self.mask[position] = self.word[position]
        else:
            self.misses += 1
        return hit

    @property
    def left(self) -> int:
        return MAX_MISSES - self.misses

    @property
    def won(self) -> bool:
        return HIDDEN not in self.mask

    @property
    def lost(self) -> bool:
        return self.misses >= MAX_MISSES

    @property
    def over(self) -> bool:
        return self.won or self.lost

    def __str__(self) -> str:
        return "{ " + ", ".join(f"'{char}'" for char in self.mask) + " }"
