#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>      // Для wait()
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <locale.h>

void Onsignal0() {
    printf("I'm process0. I get signal.\n");
}

void Onsignal1() {
    printf("I'm process1. I get signal.\n");
}

void Onsignal2() {
    printf("I'm process2. I get signal.\n");
}

void OnSignal_died() {
    printf("I'm in OnSignal_died\n");
}

int main() {
    setlocale(LC_TIME, "ru_RU");
    int i, pd[2], pid1, pid2, ppid, pid, p; // переменные для цикла, параметр и для каналов    
    char message[500];
    int status;    
    long int gp;    

    printf("PROCESS0: Starting executing\n");
    printf("PROCESS0 create pipe\n");

    // Создаем канал
    p = pipe(pd);
    if (p == -1) {
        perror("ERROR!!!"); // Ошибка при создании канала
        return 1;
    }

    // Устанавливаем обработчики сигнала (родительский процесс)
    signal(SIGUSR1, Onsignal0);
    signal(SIGCHLD, OnSignal_died);

    pid1 = fork(); // создаем процесс 1

    if (pid1 == -1) {
        perror("error! process1 not created");
        return 1;
    }

    if (pid1 == 0) { // Дочерний процесс 1
        printf("process1 created\n");
        printf("process1 fork process2\n");
        pid2 = fork(); // процесс 2

        if (pid2 == -1) {
            perror("error! process2 not created");
            exit(1);
        }

        if (pid2 == 0) { // Дочерний процесс 2
            printf("process2 created\n");

            gp = getpid(); // получаем идентификатор процесса
            strcpy(message, "          I'm process2. I'm busy\n");                    
            printf("process2 sent message in channel\n");

            // Записываем в канал информацию от второго процесса
            write(pd[1], &gp, sizeof(gp));
            write(pd[1], message, sizeof(message));

            printf("process2 finished\n");
            exit(0);
        } else { // Процесс 1
            // Ожидаем, пока process2 завершится и запишет данные в канал
            waitpid(pid2, &status, 0);

            gp = getpid(); // получаем идентификатор
            printf("process1 sent message in channel\n");    
            strcpy(message, "          I'm process1. I'm busy\n");    

            // Записываем в канал информацию от первого процесса
            write(pd[1], &gp, sizeof(gp));
            write(pd[1], message, sizeof(message));

            printf("process1 finished\n");
            exit(0);
        }
    } else { // Родительский процесс
        // Ожидаем, пока process1 завершится и запишет данные в канал
        waitpid(pid1, &status, 0);
        printf("process0 read messages\n");

        // Читаем данные из канала (теперь все данные должны быть там)
        for (i = 0; i < 2; i++) { // Изменено на 2, так как только process1 и process2 пишут
            long int read_pid;
            char read_message[500];

            // Читаем идентификатор процесса
            if (read(pd[0], &read_pid, sizeof(read_pid)) != sizeof(read_pid)) {
                perror("Error reading pid from pipe");
                exit(1);
            }

            // Читаем сообщение
            if (read(pd[0], read_message, sizeof(read_message)) != sizeof(read_message)) {
                perror("Error reading message from pipe");
                exit(1);
            }

            printf("process0 get: %ld\t%s", read_pid, read_message);
        }

        printf("PROCESS0: Exit\n");
    }

    return 0;
}
