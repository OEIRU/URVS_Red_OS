#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <errno.h> 

// Максимальное количество файлов/директорий, которые мы можем обработать.
#define MAX_FILES 1024

// Структура для хранения информации о файле или директории.
// Включает имя файла, время последнего изменения и тип файла.
typedef struct {
    char name[256];       // Имя файла/директории (массив символов фиксированного размера)
    time_t mtime;         // Время последнего изменения файла (тип time_t из <time.h>)
    mode_t type;          // Тип файла (например, S_IFREG для обычных файлов, S_IFDIR для директорий)
} FileInfo;

// Функция для сравнения двух файлов/директорий по времени их последнего изменения.
// Используется в функции qsort для сортировки массивов файлов.
int compare_files(const void *a, const void *b) {
    FileInfo *fileA = (FileInfo *)a; // Приведение типа первого аргумента к структуре FileInfo
    FileInfo *fileB = (FileInfo *)b; // Приведение типа второго аргумента к структуре FileInfo

    if (fileA->mtime > fileB->mtime) return -1; // Если время изменения fileA больше, возвращаем -1
    if (fileA->mtime < fileB->mtime) return 1;  // Если время изменения fileA меньше, возвращаем 1
    return 0; // Если времена равны, возвращаем 0
}

// Функция для получения строкового представления типа файла.
// Используется для вывода понятного описания типа файла.
const char *get_file_type(mode_t type) {
    if (S_ISREG(type)) return "Regular File";   // Обычный файл
    if (S_ISDIR(type)) return "Directory";     // Директория
    if (S_ISLNK(type)) return "Symbolic Link"; // Символическая ссылка
    return "Unknown"; // Неизвестный тип
}

int main() {
    DIR *dir; // Указатель на открытый каталог
    struct dirent *entry; // Структура для чтения элементов каталога
    struct stat file_stat; // Структура для получения информации о файле/директории
    FileInfo files[MAX_FILES];   // Массив для хранения информации о файлах
    FileInfo dirs[MAX_FILES];    // Массив для хранения информации о директориях
    FileInfo others[MAX_FILES];  // Массив для хранения информации о других типах файлов
    int file_count = 0; 
    int dir_count = 0;  
    int other_count = 0;

    // Получаем текущую рабочую директорию.
    // Функция getcwd записывает путь к текущей директории в буфер current_dir.
    // Если что-то пойдет не так, используем perror для вывода сообщения об ошибке.
    char current_dir[1024];
    if (!getcwd(current_dir, sizeof(current_dir))) {
        perror("Error getting current directory");
        return 1;
    }

    // Открываем текущую рабочую директорию.
    // Функция opendir возвращает указатель на структуру DIR, если всё прошло успешно.
    dir = opendir(current_dir);
    if (dir == NULL) {
        perror("Error opening current directory"); 
        return 1; 
    }

    // Читаем содержимое каталога поэлементно.
    // Функция readdir возвращает указатель на структуру dirent для каждого элемента каталога.
    while ((entry = readdir(dir)) != NULL) {
        // Пропускаем специальные записи "." и "..".
        // Эти записи представляют текущую и родительскую директории соответственно.
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Формируем полный путь к текущему элементу каталога.
        // Функция snprintf безопасно формирует строку пути, предотвращая переполнение буфера.
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", current_dir, entry->d_name);

        // Получаем информацию о файле/директории.
        // Функция lstat используется вместо stat, чтобы получить информацию о символических ссылках без их разрешения.
        if (lstat(full_path, &file_stat) == -1) { // Проверяем результат вызова lstat
            perror(full_path); 
            continue; 
        }

        // Проверяем права доступа к файлу/директории.
        // Функция access проверяет, есть ли у нас право на чтение файла/директории.
        if (access(full_path, R_OK) != 0) { 
            perror(full_path);
            continue; 
        }

        // Определяем тип текущего элемента каталога и добавляем его в соответствующий массив.
        if (S_ISDIR(file_stat.st_mode)) { // Если это директория
            if (dir_count < MAX_FILES) { 
                strncpy(dirs[dir_count].name, entry->d_name, sizeof(dirs[dir_count].name) - 1);
                dirs[dir_count].name[sizeof(dirs[dir_count].name) - 1] = '\0'; 
                dirs[dir_count].mtime = file_stat.st_mtime; // Сохраняем время последнего изменения
                dirs[dir_count].type = file_stat.st_mode; // Сохраняем тип файла
                dir_count++; // Увеличиваем счетчик 
            }
        } else if (S_ISREG(file_stat.st_mode)) { // Если это обычный файл
            if (file_count < MAX_FILES) { 
                strncpy(files[file_count].name, entry->d_name, sizeof(files[file_count].name) - 1);
                files[file_count].name[sizeof(files[file_count].name) - 1] = '\0';
                files[file_count].mtime = file_stat.st_mtime; 
                files[file_count].type = file_stat.st_mode; 
                file_count++;
            }
        } else { // Если это другой тип файла
            if (other_count < MAX_FILES) { 
                strncpy(others[other_count].name, entry->d_name, sizeof(others[other_count].name) - 1);
                others[other_count].name[sizeof(others[other_count].name) - 1] = '\0'; 
                others[other_count].mtime = file_stat.st_mtime; 
                others[other_count].type = file_stat.st_mode; 
                other_count++; 
            }
        }
    }

    // Функция closedir освобождает ресурсы, связанные с открытым каталогом.
    closedir(dir);

    // Сортируем массивы файлов, директорий и других типов файлов по времени их последнего изменения.
    // Функция qsort использует функцию compare_files для сравнения элементов массива.
    qsort(files, file_count, sizeof(FileInfo), compare_files);
    qsort(dirs, dir_count, sizeof(FileInfo), compare_files);
    qsort(others, other_count, sizeof(FileInfo), compare_files);

    int i;

    // Выводим отсортированный список файлов, если он не пуст.
    if (file_count > 0) {
        fprintf(stdout, "Files sorted by modification time:\n");
        for (i = 0; i < file_count; i++) {
            fprintf(stdout, "%s (%s)\n", files[i].name, get_file_type(files[i].type)); // Вывод имени файла и его тип
        }
    } else {
        perror(stdout, "No regular files found in the current directory.\n"); // Сообщаем, что файлов нет
    }

    if (dir_count > 0) {
        fprintf(stdout, "\nDirectories sorted by modification time:\n");
        for (i = 0; i < dir_count; i++) {
            fprintf(stdout, "%s (%s)\n", dirs[i].name, get_file_type(dirs[i].type)); 
        }
    } else {
        perror(stdout, "\nNo directories found in the current directory.\n");
    }

    // Выводим отсортированный список других типов файлов, если он не пуст.
    if (other_count > 0) {
        fprintf(stdout, "\nOther types sorted by modification time:\n");
        for (i = 0; i < other_count; i++) {
            fprintf(stdout, "%s (%s)\n", others[i].name, get_file_type(others[i].type)); 
        }
    } else {
        perror("\nNo other types of files found in the current directory.\n"); 
    }

    return 0; 
}