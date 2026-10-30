#include <cstring>
#include <iostream>
#include <random>

using namespace std;

const int MAX_MISSES = 5;
const int WORD_COUNT = 7;
const int MAX_LENGTH = 7;

char WORDS[WORD_COUNT][MAX_LENGTH] = {
    {'t', 'h', 'i', 's'},
    {'i', 's'},
    {'a'},
    {'w', 'o', 'r', 'd'},
    {'a', 'r', 'r', 'a', 'y'},
    {'m', 'a', 't', 'r', 'i', 'x'},
    {'l', 'e', 't', 't', 'e', 'r'},
};

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
    random_device random;
    char *word = WORDS[random() % WORD_COUNT];
    int length = strlen(word);

    char mask[MAX_LENGTH];
    for (int i = 0; i < length; i++) {
        mask[i] = '*';
    }

    int misses = 0;
    int opened = 0;
    cout << "Вгадай слово! Загадано слово з " << length << " літер, можна помилитися " << MAX_MISSES << " разів\n";

    while (opened < length && misses < MAX_MISSES) {
        print_mask(mask, length);
        char letter;
        int position;
        cout << "Літера: ";
        cin >> letter;
        cout << "Позиція (0-" << length - 1 << "): ";
        cin >> position;
        if (cin.eof()) {
            return 0;
        }
        if (cin.fail() || position < 0 || position >= length) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Такої позиції немає\n";
            continue;
        }
        if (word[position] == letter) {
            if (mask[position] == '*') {
                mask[position] = letter;
                opened++;
            }
            cout << "Вгадали!\n";
        } else {
            misses++;
            cout << "Не вгадали. Залишилось спроб: " << MAX_MISSES - misses << "\n";
        }
    }

    print_mask(mask, length);
    if (opened == length) {
        cout << "Перемога! Слово вгадано\n";
    } else {
        cout << "Спроби вичерпано. Загадане слово: " << word << "\n";
    }
    return 0;
}
