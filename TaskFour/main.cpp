#include <cstring>
#include <iostream>
#include <random>

using namespace std;

const int MAX_MISSES = 5;

char WORDS[7][7] = {
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
    char *word = WORDS[random() % 7];
    int length = strlen(word);

    char mask[7];
    for (int i = 0; i < length; i++) {
        mask[i] = '*';
    }

    int misses = 0;
    int opened = 0;
    cout << "Загадано слово з " << length << " літер\n";

    while (opened < length && misses < MAX_MISSES) {
        print_mask(mask, length);
        char letter;
        int position;
        cout << "Літера: ";
        cin >> letter;
        cout << "Позиція: ";
        cin >> position;
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
