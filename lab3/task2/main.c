#include <stdio.h>
#include <stdlib.h>
#include "status.h"
#include "func.h"
#include "io.h"
#include <unistd.h>

static const char *status_message(status st)
{
    switch (st) {
        case ERROR_SUCCESS:
            return "Успешно";
        case ERROR_NULL_POINTER:
            return "Передан нулевой указатель";
        case ERROR_INVALID_ARGUMENT:
            return "Некорректный аргумент";
        case ERROR_INVALID_FORMAT:
            return "Некорректный формат данных";
        case ERROR_INVALID_TIME:
            return "Некорректное время";
        case ERROR_INVALID_ID:
            return "Некорректный идентификатор";
        case ERROR_DUPLICATE_ID:
            return "Идентификаторы докладов не уникальны";
        case ERROR_MEMORY:
            return "Не удалось выделить память";
        case ERROR_INPUT:
            return "Ошибка чтения входных данных";
        default:
            return "Неизвестная ошибка";
    }
}

int main(void)
{
    talk_t *talks = NULL;
    size_t n = 0;
    size_t halls = 0;
    size_t result = 0;

    status st = read_talks(STDIN_FILENO, &talks, &n, &halls);
    if (st != ERROR_SUCCESS) {
        fprintf(stderr, "Ошибка: %s\n", status_message(st));
        return 1;
    }

    st = max_talks_count(talks, n, halls, &result);
    if (st != ERROR_SUCCESS) {
        fprintf(stderr, "Ошибка: %s\n", status_message(st));
        free(talks);
        return 1;
    }

    printf("%zu\n", result);

    free(talks);
    return 0;
}