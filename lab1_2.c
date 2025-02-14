#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// Структура для хранения информации о файле
typedef struct {
    char name[256];
    time_t mtime;
    int is_dir;
} FileInfo;

// Функция для сравнения двух FileInfo по времени изменения
int compare_files(const void *a, const void *b) {
    FileInfo *fileA = (FileInfo *)a;
    FileInfo *fileB = (FileInfo *)b;

    if (fileA->is_dir != fileB->is_dir) {
        return fileA->is_dir - fileB->is_dir;
    }

    return (int)(fileB->mtime - fileA->mtime);
}

int main() {
    DIR *dir;
    struct dirent *ent;
    struct stat file_stat;
    FileInfo *files = NULL;
    int count = 0;

    // Открываем каталог
    if ((dir = opendir(".")) == NULL) {
        perror("opendir");
        return 1;
    }

    // Читаем записи каталога и собираем информацию о файлах
    while ((ent = readdir(dir)) != NULL) {
        if (stat(ent->d_name, &file_stat) == -1) {
            perror("stat");
            continue;
        }

        files = realloc(files, (count + 1) * sizeof(FileInfo));
        if (!files) {
            perror("realloc");
            return 1;
        }

        strncpy(files[count].name, ent->d_name, sizeof(files[count].name) - 1);
        files[count].name[sizeof(files[count].name) - 1] = '\0';
        files[count].mtime = file_stat.st_mtime;
        files[count].is_dir = S_ISDIR(file_stat.st_mode);

        count++;
    }

    // Сортируем файлы по времени изменения и типу (файл/каталог)
    qsort(files, count, sizeof(FileInfo), compare_files);

    // Выводим отсортированные файлы
    for (int i = 0; i < count; i++) {
        printf("%s\n", files[i].name);
    }

    // Освобождаем память
    free(files);

    // Закрываем каталог
    if (closedir(dir) == -1) {
        perror("closedir");
        return 1;
    }

    return 0;
}