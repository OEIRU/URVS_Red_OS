#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

// Максимальное количество файлов, которые мы можем обработать
#define MAX_FILES 1024

// Структура для хранения информации о файле
typedef struct {
    char name[256];  // Имя файла
    time_t mtime;    // Время последнего изменения
    int is_dir;      // 1, если это директория, 0 — если файл
} FileInfo;

// Функция для сравнения двух файлов
int compare_files(FileInfo *fileA, FileInfo *fileB) {
    // Если fileA — директория, а fileB — нет, то fileA должен быть в конце списка
    if (fileA->is_dir && !fileB->is_dir) return 1;
    // Если fileB — директория, а fileA — нет, то fileB должен быть в конце списка
    if (!fileA->is_dir && fileB->is_dir) return -1;

    // Сортируем по времени модификации (сначала новые файлы)
    if (fileA->mtime > fileB->mtime) return -1;
    if (fileA->mtime < fileB->mtime) return 1;

    // Если время одинаковое, порядок не важен
    return 0;
}

// Функция для сортировки массива FileInfo (сортировка пузырьком)
void sort_files(FileInfo files[], int file_count) {
    for (int i = 0; i < file_count - 1; i++) {
        for (int j = 0; j < file_count - i - 1; j++) {
            if (compare_files(&files[j], &files[j + 1]) > 0) {
                // Меняем местами files[j] и files[j + 1]
                FileInfo temp = files[j];
                files[j] = files[j + 1];
                files[j + 1] = temp;
            }
        }
    }
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
        printf("Ошибка: не удалось открыть каталог.\n");
        return 1;
    }

    // Читаем содержимое каталога
    while ((entry = readdir(dir)) != NULL && file_count < MAX_FILES) {
        // Пропускаем текущий (".") и родительский ("..") каталоги
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Получаем информацию о файле
        if (stat(entry->d_name, &file_stat) == -1) {
            printf("Ошибка: не удалось получить информацию о файле %s.\n", entry->d_name);
            continue;
        }

        // Заполняем структуру FileInfo
        strncpy(files[file_count].name, entry->d_name, sizeof(files[file_count].name) - 1);
        files[file_count].name[sizeof(files[file_count].name) - 1] = '\0';  // Убедимся, что строка завершается нулем
        files[file_count].mtime = file_stat.st_mtime;
        files[file_count].is_dir = S_ISDIR(file_stat.st_mode);  // Проверяем, является ли файл директорией

        file_count++;
    }

    // Закрываем каталог
    closedir(dir);

    // Сортируем файлы вручную (сортировка пузырьком)
    sort_files(files, file_count);

    // Выводим отсортированный список файлов
    for (int i = 0; i < file_count; i++) {
        printf("%s\n", files[i].name);
    }

    return 0;
}