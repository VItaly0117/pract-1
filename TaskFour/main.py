from game import MAX_MISSES, Game


def ask_letter() -> str:
    while len(letter := input("Літера: ").strip()) != 1 or not letter.isalpha():
        print("Введіть одну літеру")
    return letter


def ask_position(length: int) -> int:
    while not (text := input(f"Позиція (0-{length - 1}): ").strip()).isdigit() or int(text) >= length:
        print(f"Введіть число від 0 до {length - 1}")
    return int(text)


def main() -> None:
    game = Game()
    print(f"Загадано слово з {len(game.word)} літер. Дозволено невдалих спроб: {MAX_MISSES}")
    print(game)
    while not game.over:
        if game.guess(ask_letter(), ask_position(len(game.word))):
            print("Вгадали!")
        else:
            print(f"Не вгадали. Залишилось спроб: {game.left}")
        print(game)
    if game.won:
        print(f"Перемога! Невдалих спроб: {game.misses}")
    else:
        print(f"Спроби вичерпано. Загадане слово: {''.join(game.word)}")


if __name__ == "__main__":
    main()
