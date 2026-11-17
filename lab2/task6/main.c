#include <stdio.h>
#include "status.h"
#include "func.h"

static void print_status(status st, int scanned) {
    switch (st) {
        case SUCCESS:
            printf("Успешно, считано элементов: %d\n", scanned);
            break;
        case ERROR_NULL_POINTER:
            printf("Ошибка: нулевой указатель.\n");
            break;
        case ERROR_IO:
            printf("Ошибка ввода-вывода.\n");
            break;
        case ERROR_FORMAT:
            printf("Ошибка: неверный спецификатор формата.\n");
            break;
        case ERROR_CONVERSION:
            printf("Ошибка преобразования после %d элементов.\n", scanned);
            break;
        default:
            printf("Неизвестный статус.\n");
            break;
    }
}

int main(void) {
    int scanned = 0;
    int roman = 0;
    unsigned int zeck = 0;
    int low = 0;
    int up = 0;

    status st = oversscanf("XIV 1011 1f 1F", "%Ro %Zr %Cv %CV", &scanned, &roman, &zeck, &low, 16, &up, 16);
    printf("--- oversscanf ---\n");
    print_status(st, scanned);
    if (st == SUCCESS) {
        printf("Ro: %d, Zr: %u, Cv(16): %d, CV(16): %d\n", roman, zeck, low, up);
    }

    FILE *file = tmpfile();
    if (!file) {
        printf("Не удалось создать временный файл.\n");
        return 1;
    }
    fputs("MCMXCIV 00011 7f -1010 42 3.5 abc", file);
    rewind(file);

    int r2 = 0;
    unsigned int z2 = 0;
    int hex = 0;
    int bin = 0;
    int dec = 0;
    double real = 0.0;
    char word[16] = "";

    st = overfscanf(file, "%Ro %Zr %Cv %Cv %d %lf %15s", &scanned,
                    &r2, &z2, &hex, 16, &bin, 2, &dec, &real, word);
    printf("--- overfscanf ---\n");
    print_status(st, scanned);
    if (st == SUCCESS) {
        printf("Ro: %d, Zr: %u, Cv(16): %d, Cv(2): %d, d: %d, lf: %.1f, s: %s\n",
               r2, z2, hex, bin, dec, real, word);
    }

    fclose(file);
    return 0;
}