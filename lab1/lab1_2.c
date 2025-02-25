#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

// Максимальное количество файлов/директорий, которые мы можем обработать
#define MAX_FILES 1024

// Структура для хранения информации о файле или директории
typedef struct {
    char name[256];  // Имя файла/директории
    time_t mtime;    // Время последнего изменения
} FileInfo;

// Функция для сравнения двух файлов/директорий
int compare_files(const void *a, const void *b) {
    FileInfo *fileA = (FileInfo *)a;
    FileInfo *fileB = (FileInfo *)b;
    if (fileA->mtime > fileB->mtime) return -1;
    if (fileA->mtime < fileB->mtime) return 1;
    return 0;
}

int main() {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    FileInfo files[MAX_FILES];   // Для файлов
    FileInfo dirs[MAX_FILES];    // Для директорий
    int file_count = 0;
    int dir_count = 0;

    // Открываем текущий каталог
    dir = opendir(".");
    if (dir == NULL) {
        printf("Ошибка: не удалось открыть каталог.\n");
        return 1;
    }

    // Читаем содержимое каталога
    while ((entry = readdir(dir)) != NULL) {
        // Пропускаем текущий (".") и родительский ("..") каталоги
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Получаем информацию о файле/директории
        if (stat(entry->d_name, &file_stat) == -1) {
            printf("Ошибка: не удалось получить информацию о %s.\n", entry->d_name);
            continue;
        }

        // Определяем, является ли элемент файлом или директорией
        if (S_ISDIR(file_stat.st_mode)) {
            // Это директория
            if (dir_count < MAX_FILES) {
                strncpy(dirs[dir_count].name, entry->d_name, sizeof(dirs[dir_count].name) - 1);
                dirs[dir_count].name[sizeof(dirs[dir_count].name) - 1] = '\0';
                dirs[dir_count].mtime = file_stat.st_mtime;
                dir_count++;
            }
        } else {
            // Это файл
            if (file_count < MAX_FILES) {
                strncpy(files[file_count].name, entry->d_name, sizeof(files[file_count].name) - 1);
                files[file_count].name[sizeof(files[file_count].name) - 1] = '\0';
                files[file_count].mtime = file_stat.st_mtime;
                file_count++;
            }
        }
    }

    // Закрываем каталог
    closedir(dir);

    // Сортируем файлы по времени изменения
    qsort(files, file_count, sizeof(FileInfo), compare_files);

    // Сортируем директории по времени изменения
    qsort(dirs, dir_count, sizeof(FileInfo), compare_files);

    // Выводим отсортированный список файлов
    printf("Files sorted by modification time:\n");
    for (int i = 0; i < file_count; i++) {
        printf("%s\n", files[i].name);
    }

    // Выводим отсортированный список директорий
    printf("\nDirectories sorted by modification time:\n");
    for (int i = 0; i < dir_count; i++) {
        printf("%s\n", dirs[i].name);
    }

    return 0;
}