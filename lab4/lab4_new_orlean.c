#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

void sigchld_handler(int sig) {
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        printf("Процесс %d завершился\n", pid);
    }
}

int main() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("Ошибка sigaction");
        exit(EXIT_FAILURE);
    }

    // Запуск ls -a > a.txt в фоне
    pid_t ls_pid = fork();
    if (ls_pid == 0) {
        printf("Создан процесс для ls -a (PID: %d)\n", getpid());

        // Открываем файл a.txt для записи
        int fd = open("a.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            perror("Ошибка открытия a.txt");
            exit(EXIT_FAILURE);
        }

        // Закрываем стандартный вывод (дескриптор 1)
        close(STDOUT_FILENO);

        // Создаем копию дескриптора fd и присваиваем его дескриптору 1
        fcntl(fd, F_DUPFD, STDOUT_FILENO);

        // Закрываем оригинальный дескриптор fd
        close(fd);

        // Выполняем команду ls -a
        execlp("ls", "ls", "-a", NULL);
        perror("Ошибка execlp ls");
        exit(EXIT_FAILURE);
    } else if (ls_pid < 0) {
        perror("Ошибка fork для ls");
        exit(EXIT_FAILURE);
    } else {
        printf("Процесс ls -a запущен в фоне (PID: %d)\n", ls_pid);
    }

    // Запуск who > b.txt
    pid_t who_pid = fork();
    if (who_pid == 0) {
        printf("Создан процесс для who (PID: %d)\n", getpid());

        // Открываем файл b.txt для записи
        int fd = open("b.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            perror("Ошибка открытия b.txt");
            exit(EXIT_FAILURE);
        }

        // Закрываем стандартный вывод (дескриптор 1)
        close(STDOUT_FILENO);

        // Создаем копию дескриптора fd и присваиваем его дескриптору 1
        fcntl(fd, F_DUPFD, STDOUT_FILENO);

        // Закрываем оригинальный дескриптор fd
        close(fd);

        // Выполняем команду who
        execlp("who", "who", NULL);
        perror("Ошибка execlp who");
        exit(EXIT_FAILURE);
    } else if (who_pid < 0) {
        perror("Ошибка fork для who");
        exit(EXIT_FAILURE);
    } else {
        waitpid(who_pid, NULL, 0);
        printf("Процесс who завершен (PID: %d)\n", who_pid);
    }

    // Запуск cat a.txt b.txt | sort
    int pipefd[2];
    if (pipe(pipefd)) {
        perror("Ошибка создания канала");
        exit(EXIT_FAILURE);
    }

    pid_t cat_pid = fork();
    if (cat_pid == 0) {
        printf("Создан процесс для cat (PID: %d)\n", getpid());

        // Закрываем конец канала для чтения
        close(pipefd[0]);

        // Закрываем стандартный вывод (дескриптор 1)
        close(STDOUT_FILENO);

        // Создаем копию дескриптора pipefd[1] и присваиваем его дескриптору 1
        fcntl(pipefd[1], F_DUPFD, STDOUT_FILENO);

        // Закрываем оригинальный дескриптор pipefd[1]
        close(pipefd[1]);

        // Выполняем команду cat
        execlp("cat", "cat", "a.txt", "b.txt", NULL);
        perror("Ошибка execlp cat");
        exit(EXIT_FAILURE);
    } else if (cat_pid < 0) {
        perror("Ошибка fork для cat");
        exit(EXIT_FAILURE);
    }

    pid_t sort_pid = fork();
        if (sort_pid == 0) {
        printf("Создан процесс для sort (PID: %d)\n", getpid());

        // Закрываем конец канала для записи
        close(pipefd[1]);

        // Закрываем стандартный ввод (дескриптор 0)
        close(STDIN_FILENO);

        // Создаем копию дескриптора pipefd[0] и присваиваем его дескриптору 0
        fcntl(pipefd[0], F_DUPFD, STDIN_FILENO);

        // Закрываем оригинальный дескриптор pipefd[0]
        close(pipefd[0]);

        // Выполняем команду sort
        execlp("sort", "sort", NULL);
        perror("Ошибка execlp sort");
        exit(EXIT_FAILURE);
    } else if (sort_pid < 0) {
        perror("Ошибка fork для sort");
        exit(EXIT_FAILURE);
    }

    // Закрываем оба конца канала в родительском процессе
    close(pipefd[0]);
    close(pipefd[1]);

    // Ожидаем завершения процессов
    waitpid(cat_pid, NULL, 0);
    printf("Процесс cat завершен (PID: %d)\n", cat_pid);
    waitpid(sort_pid, NULL, 0);
    printf("Процесс sort завершен (PID: %d)\n", sort_pid);

    return 0;
}