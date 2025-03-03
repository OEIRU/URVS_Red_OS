// UPDATES:
// 1. После каждого вызова waitpid() добавлена проверка статуса завершения (WIFEXITED и WIFSIGNALED).
// 2. Добавлены сообщения о старте и завершении дочерних процессов.
// 3. Добавлены проверки результатов системных вызовов (write, read).
// 4. Добавлены комментарии
// 5. Нужна ли нам функция polling? Если да - реализовать перед waitpid().

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

// Функция для вычисления факториала числа n
double factorial(int n) {
    double result = 1.0;
    for (int i = 1; i <= n; i++) {
        result *= i;
    }
    return result;
}

int main() {
    // Структура для хранения данных от дочерних процессов
    struct storage {
        int pid;    // Идентификатор процесса-потомка
        double data; // Значение, полученное от потомка
    } st;

    int n; // Количество членов ряда для вычислений
    double x, cos_val = 0.0, pi_val = 0.0, f = 0.0; // Переменные для хранения результатов
    char *file_name = "file.tmp"; // Временный файл для обмена данными
    int fd;

    // Создание или открытие временного файла с правами на чтение и запись
    fd = open(file_name, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        perror("Error: don't open file\n");
        return 1;
    }

    // Ввод значений x и n
    printf("Введите x = ");
    if (scanf("%lf", &x) != 1) {
        perror("Error: Input x incorrect\n");
        close(fd);
        remove(file_name);
        return 1;
    }
    printf("Введите n = ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        perror("Error: Input n incorrect\n");
        close(fd);
        remove(file_name);
        return 1;
    }

    // Создание первого дочернего процесса для вычисления Pi
    pid_t pid_pi = fork();
    if (pid_pi == 0) { // Первый потомок (вычисление Pi)
        printf("Child process %d started. Parent PID: %d\n", getpid(), getppid());
        st.pid = getpid(); // Сохраняем PID текущего процесса
        st.data = 0.0;     // Инициализируем значение Pi
        int a = 1;         // Коэффициент для ряда Лейбница

    // Вычисление Pi через ряд Лейбница
    for (int i = 0; i < n; i++) {
        st.data += 4.0 * a / (2.0 * i + 1.0);
        a = a * (-1); // Изменяем знак для следующего члена ряда

        // Проверка на NaN
        if (isnan(st.data)) {
            fprintf(stderr, "Error in child process %d: Computation of Pi resulted in NaN at iteration %d.\n", getpid(), i);
            close(fd); // Закрываем файловый дескриптор
            exit(EXIT_FAILURE); // Завершаем процесс с кодом ошибки
        }
    }

        
        // Записываем данные в файл
        if (write(fd, &st, sizeof(st)) != sizeof(st)) {
            perror("Error: don't write\n");
            exit(EXIT_FAILURE);
        }
        close(fd); // Закрываем файл
        printf("Child process %d finished.\n", getpid());
        exit(0);   // Завершаем работу процесса
    } else if (pid_pi < 0) {
        perror("Error: don't make fork (pi)\n");
        close(fd);
        remove(file_name);
        return 1;
    }

    // Создание второго дочернего процесса для вычисления cos(x)
    pid_t pid_cos = fork();
    if (pid_cos == 0) { // Второй потомок (вычисление cos(x))
        printf("Child process %d started. Parent PID: %d\n", getpid(), getppid());
        st.pid = getpid(); // Сохраняем PID текущего процесса
    
        // Применяем периодичность для уменьшения значения x до диапазона [0, 2?]
        double reduced_x = fmod(x, 2 * M_PI); // Учитываем только основной период
        if (reduced_x < 0) {
            reduced_x += 2 * M_PI; // Если x отрицательно, приводим его к положительному значению
        }
    
        st.data = 1.0; // Инициализируем значение cos(x)
        int a = 1;     // Коэффициент для ряда
    
        // Вычисление cos(x) через степенной ряд
        for (int i = 1; i < n; i++) {
            a = a * (-1); // Изменяем знак для следующего члена ряда

            // Вычисляем текущий член ряда
            double current_term = a * pow(reduced_x, 2 * i) / factorial(2 * i);

            // Проверяем текущий член на NaN
            if (isnan(current_term)) {
                fprintf(stderr, "Error in child process %d: Computation of cos(x) resulted in NaN at iteration %d.\n", getpid(), i);
                close(fd); // Закрываем файловый дескриптор
                exit(EXIT_FAILURE); // Завершаем процесс с кодом ошибки
            }

            st.data += current_term; // Добавляем текущий член к результату
        }

        // Финальная проверка на NaN после завершения цикла
        if (isnan(st.data)) {
            fprintf(stderr, "Error in child process %d: Final result of cos(x) computation resulted in NaN.\n", getpid());
            close(fd); // Закрываем файловый дескриптор
            exit(EXIT_FAILURE); // Завершаем процесс с кодом ошибки
        }

        // Записываем данные в файл
        if (write(fd, &st, sizeof(st)) != sizeof(st)) {
            perror("Error: don't write cos\n");
            exit(EXIT_FAILURE);
        }
        close(fd); // Закрываем файл
        printf("Child process %d finished.\n", getpid());
        exit(0);   // Завершаем работу процесса
    } else if (pid_cos < 0) {
        perror("Error: make fork (cos)\n");
        close(fd);
        remove(file_name);
        return 1;
    }
    
    // Ожидание завершения первого дочернего процесса (вычисление Pi)
    int status_pi;
    if (waitpid(pid_pi, &status_pi, 0) == -1) {
        perror("Error: waitpid (pi)\n");
        close(fd);
        remove(file_name);
        return 1;
    }

    if (!WIFEXITED(status_pi) || WEXITSTATUS(status_pi) != 0) {
        fprintf(stderr, "Error: Child process %d (Pi computation) exited with an error.\n", pid_pi);
        close(fd);
        remove(file_name);
        return 1;
    }

    // Ожидание завершения второго дочернего процесса (вычисление cos(x))
    int status_cos;
    if (waitpid(pid_cos, &status_cos, 0) == -1) {
        perror("Error: waitpid (cos)\n");
        close(fd);
        remove(file_name);
        return 1;
    }

    if (!WIFEXITED(status_cos) || WEXITSTATUS(status_cos) != 0) {
        fprintf(stderr,"Error: Child process %d (cos(x) computation) exited with an error.\n", pid_cos);
        close(fd);
        remove(file_name);
        return 1;
    }

    // Если оба процесса завершились успешно, читаем результаты из файла
    lseek(fd, 0, SEEK_SET); // Переходим в начало файла
    for (int i = 0; i < 2; i++) {
        if (read(fd, &st, sizeof(st)) != sizeof(st)) {
            perror("Error: don't read pid\n");
            break;
        }
        if (st.pid == pid_pi) {
            pi_val = st.data; // Получаем значение Pi
            if (isnan(pi_val)) {
                perror("Error: Computed Pi value is NaN.\n");
                close(fd);
                remove(file_name);
                return 1;
            }
        } else if (st.pid == pid_cos) {
            cos_val = st.data; // Получаем значение cos(x)
            if (isnan(cos_val)) {
                perror("Error: Computed cos(x) value is NaN.\n");
                close(fd);
                remove(file_name);
                return 1;
            }
        }
    }

    // Вычисление функции f(x) = Pi * cos(x)
    f = pi_val * cos_val;

    // Вывод результатов
    printf("Pi = %.10lf\n", pi_val);
    printf("cos(%.10lf) = %.10lf\n", x, cos_val);
    printf("f(%.10lf) = %.10lf\n", x, f);

    // Очистка ресурсов
    close(fd);
    remove(file_name);
    return 0;
}