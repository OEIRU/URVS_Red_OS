#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

#define LINES    10   // Высота таблицы
#define COLUMNS  30   // Ширина строки

// Структура таблицы
struct table {
    char el[LINES][COLUMNS];
};

// Инициализация таблицы
void table_init(struct table *t) {
    memset(t->el, 0, sizeof(t->el));
}

// Вставка строки в таблицу
char* table_insert(struct table *t, int line_number, const char *content) {
    if (line_number < 0 || line_number >= LINES) {
        fprintf(stderr, "Некорректный индекс таблицы\n");
        exit(EXIT_FAILURE);
    }

    // Выделяем память для старой строки
    char *old = malloc(COLUMNS);
    if (!old) {
        perror("Ошибка выделения памяти");
        exit(EXIT_FAILURE);
    }
    strncpy(old, t->el[line_number], COLUMNS);
    old[COLUMNS-1] = '\0';

    // Копируем новое содержимое
    strncpy(t->el[line_number], content, COLUMNS);
    t->el[line_number][COLUMNS-1] = '\0';
    
    return old;
}

// Печать таблицы
void table_print(const struct table *t) {
    printf("Таблица:\n");
    for (int i = 0; i < LINES; i++) {
        printf(" %2d | %s\n", i, t->el[i]);
    }
    printf("\n");
}

// Структура сообщения
struct msg_buf {
    long mtype;
    char mtext[COLUMNS + sizeof(int)];
};

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <K>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int K = atoi(argv[1]);

    struct table tab;
    table_init(&tab);

    int msgqid;
    struct msg_buf msgp;

    // Создаем очередь сообщений
    if ((msgqid = msgget(IPC_PRIVATE, IPC_CREAT | 0600)) == -1) {
        perror("Ошибка создания очереди");
        exit(EXIT_FAILURE);
    }

    // Создаем дочерние процессы
    for (int i = 0; i < K; i++) {
        if (fork() == 0) {
            // Формируем сообщение
            msgp.mtype = 1;
            int line_num = i % (K/2);
            memcpy(msgp.mtext, &line_num, sizeof(int));
            const char *text = (i < K/2) ? "not memes" : "memes";
            strncpy(msgp.mtext + sizeof(int), text, COLUMNS);

            // Отправляем сообщение
            if (msgsnd(msgqid, &msgp, sizeof(msgp.mtext), IPC_NOWAIT) == -1) {
                perror("Ошибка отправки");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }
    }

    // Обработка сообщений
    for (int i = 0; i < K; i++) {
        printf("%d. Замена строки\n", i+1);
        
        if (msgrcv(msgqid, &msgp, sizeof(msgp.mtext), 1, 0) == -1) {
            perror("Ошибка приема");
            exit(EXIT_FAILURE);
        }

        int line_number;
        memcpy(&line_number, msgp.mtext, sizeof(int));
        char *old = table_insert(&tab, line_number, msgp.mtext + sizeof(int));
        
        printf("Старая строка #%d: %s\n", line_number, old);
        free(old);
        table_print(&tab);
    }

    // Удаление очереди
    if (msgctl(msgqid, IPC_RMID, NULL) == -1) {
        perror("Ошибка удаления очереди");
    }

    return EXIT_SUCCESS;
}