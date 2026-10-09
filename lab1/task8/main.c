#include <stdio.h>
#include "status.h"
#include "func.h"

void print_err(const status err) {
    const char *messages[] = {
        [SUCCESS]                     = "Успешное выполнение.",
        [ERROR_INVALID_PARAMETER]     = "Ошибка: Переданы некорректные параметры.",
        [ERROR_FILE_OPEN]             = "Ошибка: Не удалось открыть файл.",
        [ERROR_FILE_READ]             = "Ошибка: Ошибка чтения из файла.",
        [ERROR_FILE_WRITE]            = "Ошибка: Ошибка записи в файл.",
        [ERROR_TOKEN_TOO_LONG]        = "Ошибка: Токен превышает максимально допустимый размер буфера.",
        [ERROR_INVALID_NUMBER_FORMAT] = "Ошибка: Невалидный формат числа или невоможно определить основание.",
        [ERROR_OVERFLOW]              = "Ошибка: Переполнение при переводе в десятичную систему счисления."
    };

    if (err >= SUCCESS && err <= ERROR_OVERFLOW) {
        fprintf(stderr, "%s\n", messages[err]);
    } else {
        fprintf(stderr, "Ошибка: Неизвестная ошибка (код %d).\n", err);
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