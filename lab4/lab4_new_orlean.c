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
        int fd = open("a.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            perror("Ошибка открытия a.txt");
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
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
        int fd = open("b.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            perror("Ошибка открытия b.txt");
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
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
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
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
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        execlp("sort", "sort", NULL);
        perror("Ошибка execlp sort");
        exit(EXIT_FAILURE);
    } else if (sort_pid < 0) {
        perror("Ошибка fork для sort");
        exit(EXIT_FAILURE);
    }

    close(pipefd[0]);
    close(pipefd[1]);

    waitpid(cat_pid, NULL, 0);
    printf("Процесс cat завершен (PID: %d)\n", cat_pid);
    waitpid(sort_pid, NULL, 0);
    printf("Процесс sort завершен (PID: %d)\n", sort_pid);

    return 0;
}