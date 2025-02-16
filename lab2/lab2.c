#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h> 

double factor(int n) {
    double s = 1.0;
    for (int i = 1; i <= n; i++)
        s *= i;
    return s;
}

int main() {
    struct storage {
        int pid;    // идентификатор потомка
        double data; // значение, полученное от потомка
    } st;

    int n;
    double x, cos_val = 0.0, pi_val = 0.0, f = 0.0;  
    char *file_name = "file.tmp";
    int fd;

    // Create the file before opening it
    fd = open(file_name, O_RDWR | O_CREAT | O_TRUNC, 0666); // Open or create file with read/write permissions
    if (fd == -1) {
        perror("open");
        return 1;
    }

    printf("\nВведите x = ");
    if (scanf("%lf", &x) != 1) {
        fprintf(stderr, "Error reading x\n");
        close(fd);
        remove(file_name);
        return 1;
    }
    printf("\nВведите n = ");
    if (scanf("%d", &n) != 1) {
        fprintf(stderr, "Error reading n\n");
        close(fd);
        remove(file_name);
        return 1;
    }
    printf("\n");

    pid_t pid_pi = fork();
    if (pid_pi == 0) { // первый потомок процесса (для Pi)
        st.pid = getpid();
        st.data = 0.0;
        int a = 1;  
        for (int i = 0; i < n; i++) {
            st.data += 4.0 * a / (2.0 * i + 1.0);
            a = a * (-1);
        }
        write(fd, &st, sizeof(st));
        close(fd);
        exit(0);
    } else if (pid_pi < 0) {
        perror("fork (pi)");
        close(fd);
        remove(file_name);
        return 1;
    }

    pid_t pid_cos = fork();
    if (pid_cos == 0) { // второй потомок процесса (для cos)
        st.pid = getpid();
        st.data = 1.0;
        int a = 1;  
        for (int i = 1; i < n; i++) {
            a = a * (-1);
            st.data += a * pow(x, 2 * i) / factor(2 * i);
        }
        write(fd, &st, sizeof(st));
        close(fd);
        exit(0);
    } else if (pid_cos < 0) {
        perror("fork (cos)");
        close(fd);
        remove(file_name);
        return 1;
    }

    // Parent process: wait for children to finish writing
    waitpid(pid_pi, NULL, 0);
    waitpid(pid_cos, NULL, 0);

    lseek(fd, 0, SEEK_SET); // переходим в начало файла

    // Read data from file
    for (int i = 0; i < 2; i++) {
        read(fd, &st, sizeof(st));
        if (st.pid == pid_pi) {
            pi_val = st.data;
        } else if (st.pid == pid_cos) {
            cos_val = st.data;
        }
    }

    f = pi_val * cos_val; // вычисляем функцию


    printf("Pi = %lf\n", pi_val);
    printf("cos(%lf) = %lf\n", x, cos_val);
    printf("f(%lf) = %lf\n", x, f);
    
    close(fd); // закрываем
    remove(file_name); // удаляем
    return 0;
}
    