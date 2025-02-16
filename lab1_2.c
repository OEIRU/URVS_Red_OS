#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#define MAX_FILES 1024

// Структура для хранения информации о файле
typedef struct {
    char name[256];
    time_t mtime;
    int is_dir;
} FileInfo;

// Функция для сравнения файлов по времени модификации
int compare_files(const void *a, const void *b) {
    FileInfo *fileA = (FileInfo *)a;
    FileInfo *fileB = (FileInfo *)b;

    // Сначала сортируем по типу (каталоги в конце)
    if (fileA->is_dir && !fileB->is_dir) return 1;
    if (!fileA->is_dir && fileB->is_dir) return -1;

    // Затем сортируем по времени модификации
    if (fileA->mtime > fileB->mtime) return -1;
    if (fileA->mtime < fileB->mtime) return 1;

    return 0;
}

int main() {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    FileInfo files[MAX_FILES];
    int file_count = 0;

    // Открываем текущий каталог
    dir = opendir(".");
    if (dir == NULL) {
        perror("opendir");
        return 1;
    }

    // Читаем содержимое каталога
    while ((entry = readdir(dir)) != NULL && file_count < MAX_FILES) {
        // Пропускаем текущий и родительский каталоги
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Получаем информацию о файле
        if (stat(entry->d_name, &file_stat) == -1) {
            perror("stat");
            continue;
        }

        // Заполняем структуру FileInfo
        strncpy(files[file_count].name, entry->d_name, sizeof(files[file_count].name) - 1);
        files[file_count].name[sizeof(files[file_count].name) - 1] = '\0';
        files[file_count].mtime = file_stat.st_mtime;
        files[file_count].is_dir = S_ISDIR(file_stat.st_mode);

        file_count++;
    }

    closedir(dir);

    // Сортируем файлы
    qsort(files, file_count, sizeof(FileInfo), compare_files);

    // Выводим отсортированный список
    for (int i = 0; i < file_count; i++) {
        printf("%s\n", files[i].name);
    }

    return 0;
}
