#include <cstring>
#include <iostream>
#include <random>

using namespace std;

// Скільки разів можна помилитися
const int MAX_MISSES = 5;

// Матриця слів: кожен рядок це одне слово, решта рядка заповнена нулями
char WORDS[7][7] = {
    {'t', 'h', 'i', 's'},
    {'i', 's'},
    {'a'},
    {'w', 'o', 'r', 'd'},
    {'a', 'r', 'r', 'a', 'y'},
    {'m', 'a', 't', 'r', 'i', 'x'},
    {'l', 'e', 't', 't', 'e', 'r'},
};

// Виводимо слово у вигляді { 'i', '*' }
void print_mask(char mask[], int length) {
    cout << "{ ";
    for (int i = 0; i < length; i++) {
        cout << "'" << mask[i] << "'";
        if (i < length - 1) {
            cout << ", ";
        }
    }
    cout << " }\n";
}

int main() {
    // Беремо випадкове слово з матриці
    random_device random;
    char *word = WORDS[random() % 7];
    int length = strlen(word);

    // Спочатку всі літери закриті зірочками
    char mask[7];
    for (int i = 0; i < length; i++) {
        mask[i] = '*';
    }

    // misses це помилки, opened це скільки літер уже відкрито
    int misses = 0;
    int opened = 0;
    cout << "Загадано слово з " << length << " літер\n";

    // Граємо, поки не відкрили все слово або не закінчились спроби
    while (opened < length && misses < MAX_MISSES) {
        print_mask(mask, length);
        char letter;
        int position;
        cout << "Літера: ";
        cin >> letter;
        cout << "Позиція: ";
        cin >> position;
        // Якщо на цій позиції саме ця літера, відкриваємо її
        if (position >= 0 && position < length && word[position] == letter && mask[position] == '*') {
            mask[position] = letter;
            opened++;
            cout << "Вгадали!\n";
        } else {
            misses++;
            cout << "Не вгадали. Залишилось спроб: " << MAX_MISSES - misses << "\n";
        }
    }

    print_mask(mask, length);
    if (opened == length) {
        cout << "Перемога!\n";
    } else {
        cout << "Поразка. Слово: " << word << "\n";
    }
}
