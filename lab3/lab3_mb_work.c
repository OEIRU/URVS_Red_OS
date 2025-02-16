#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>

// Вспомогательные переменные для ожидания события
volatile sig_atomic_t got1 = 0, got3 = 0;

// Функции, вызываемые при получении соответствующего сигнала
void sigfunc1(int signum)
{
    got1 = 1;
}

void sigfunc3(int signum)
{
    got3 = 1;
}

int main()
{
    int pid1, pid2, pid3, pipe1[2], pipe2[2], pipe0[2];
    int count = 0; // Переменная-счетчик
    char data[50]; // Строка для передачи сообщений и данных

    // Создание программных каналов
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1 || pipe(pipe0) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    if ((pid1 = fork()) == 0)
    { // Процесс P1
        if ((pid3 = fork()) == 0)
        { // Процесс P3
            pid1 = getppid();
            pid3 = getpid();
            printf("P1: create P3\n");

            // Подготовка данных для P0
            strncpy(data, "## From P3 data 1/2 ##", sizeof(data));
            printf("P3: -- data to K1 --\n");
            write(pipe1[1], data, sizeof(data));

            printf("P3: send signal to P1\n");
            sleep(1);
            kill(pid1, SIGUSR1); // Передача сигнала P1

            signal(SIGUSR2, sigfunc3);
            printf("P3: .. waiting for signal ..\n");
            while (!got3)
                pause(); // Ожидание сигнала

            printf("P3: !! signal !!\n");
            printf("P3: -- data from K2 to K1 --\n");

            // Считывание из K2 данных и запись их в K1
            while (count < 2)
            {
                if (read(pipe2[0], data, sizeof(data)) > 0)
                {
                    count++;
                    printf("P3: ++ data from K2: '%s' ++\n", data);
                    write(pipe1[1], data, sizeof(data));
                }
            }

            printf("P3: -- data to K1 --\n");
            strncpy(data, "## From P3 data 2/2 ##", sizeof(data));
            write(pipe1[1], data, sizeof(data));

            printf("P3: XX end P3 XX\n");
            exit(EXIT_SUCCESS);
        }

        // Процесс P1
        printf("P0: create K1,K2,K0\n");
        printf("P0: create P1\n");
        pid1 = getpid();
        signal(SIGUSR1, sigfunc1);

        printf("P1: .. waiting for signal ..\n");
        while (!got1)
            pause(); // Ожидание сигнала

        printf("P1: !! signal !!\n");
        printf("P1: -- data to K2 --\n");
        strncpy(data, "## From P1 data 1/1 ##", sizeof(data));
        write(pipe2[1], data, sizeof(data));

        printf("P1: identifikator P3 to K0\n");
        write(pipe0[1], &pid3, sizeof(pid3)); // Передача через K0 идентификатора P3

        printf("P1: .. waiting for end P3 ..\n");
        wait(NULL); // Ожидание завершения P3

        printf("P1: !! end P3 !!\n");
        printf("P1: XX end P1 XX\n");
        exit(EXIT_SUCCESS);
    }
    else if ((pid2 = fork()) == 0)
    { // Процесс P2
        pid2 = getpid();
        printf("P0: create P2\n");

        printf("P2: .. look at K0 ..\n");
        while (read(pipe0[0], &pid3, sizeof(int)) <= 0)
            ; // Опрос K0

        printf("P2: !! identificator P3 !!\n");
        printf("P2: -- data to K2 --\n");
        strncpy(data, "## From P2 data 1/1 ##", sizeof(data));
        write(pipe2[1], data, sizeof(data));

        printf("P2: send signal to P3\n");
        sleep(1);
        kill(pid3, SIGUSR2); // Передача сигнала P3

        printf("P2: XX end P2 XX\n");
        exit(EXIT_SUCCESS);
    }
    else
    { // Процесс P0
        printf("P0: .. look at K1 ..\n");

        // Обработка данных из канала K1
        while (count < 4)
        {
            if (read(pipe1[0], data, sizeof(data)) > 0)
            {
                printf("P0: ++ new data: '%s' ++\n", data);
                count++;
            }
        }

        printf("P0: .. waiting for end P1,P2,P3 ..\n");
        wait(NULL); // Ожидание завершения P1
        wait(NULL); // Ожидание завершения P2
        printf("P0: !! end P1,P2,P3 !!\n");
        printf("P0: The end.\n");
        exit(EXIT_SUCCESS);
    }
}
