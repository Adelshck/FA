#include <stdio.h>
#include "status.h"
#include "func.h"

void print_err(const status err) {
    switch (err) {
        case SUCCESS:
            fprintf(stderr, "Успешное выполнение.\n");
            break;
        case ERROR_INVALID_PARAMETER:
            fprintf(stderr, "Ошибка: Переданы некорректные параметры.\n");
            break;
        case ERROR_FILE_OPEN:
            fprintf(stderr, "Ошибка: Не удалось открыть файл.\n");
            break;
        case ERROR_FILE_READ:
            fprintf(stderr, "Ошибка: Ошибка чтения из файла.\n");
            break;
        case ERROR_FILE_WRITE:
            fprintf(stderr, "Ошибка: Ошибка записи в файл.\n");
            break;
        case ERROR_TOKEN_TOO_LONG:
            fprintf(stderr, "Ошибка: Токен превышает максимально допустимый размер буфера.\n");
            break;
        case ERROR_INVALID_NUMBER_FORMAT:
            fprintf(stderr, "Ошибка: Невалидный формат числа или невозможно определить основание.\n");
            break;
        case ERROR_OVERFLOW:
            fprintf(stderr, "Ошибка: Переполнение при переводе в десятичную систему счисления.\n");
            break;
        case ERROR_SAME_FILE:
            fprintf(stderr, "Ошибка: Одинаковые файлы на ввод и вывод.\n");
            break;
        default:
            fprintf(stderr, "Ошибка: Неизвестная ошибка (код %d).\n", err);
            break;
    }
}


int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Использование: %s <входной_файл> <выходной_файл>\n", argv[0]);
        return ERROR_INVALID_PARAMETER;
    }

    status res = process_files(argv[1], argv[2]);
    if (res != SUCCESS) {
        print_err(res);
        return res;
    }

    printf("Обработка успешно завершена.\n");
    return SUCCESS;
}