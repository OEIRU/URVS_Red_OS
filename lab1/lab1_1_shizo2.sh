#!/bin/bash

# Функция для получения списка файлов, отсортированных по времени изменения (по убыванию)
get_sorted_files_by_time_desc() {
    # Массив для хранения информации о файлах
    declare -a files=()

    # Перебираем все элементы в текущей директории, включая скрытые
    for file in .* *; do
        # Пропускаем специальные записи "." и ".."
        if [[ "$file" == "." || "$file" == ".." ]]; then
            continue
        fi

        # Проверяем, является ли элемент файлом
        if [ -f "$file" ]; then
            # Получаем время изменения файла (mtime) в формате Unix timestamp
            mtime=$(stat --format="%Y" "$file")
            # Добавляем время изменения и имя файла в массив
            files+=("$mtime $file")
        fi
    done

    # Сортируем массив по времени изменения (числовая сортировка по убыванию)
    printf "%s\n" "${files[@]}" | sort -nr | while read -r line; do
        # Извлекаем только имя файла из строки "время имя_файла"
        echo "${line#* }"
    done
}

# Функция для получения списка директорий, отсортированных по времени изменения (по убыванию)
get_sorted_dirs_by_time_desc() {
    # Массив для хранения информации о директориях
    declare -a dirs=()

    # Перебираем все элементы в текущей директории, включая скрытые
    for dir in .* *; do
        # Пропускаем специальные записи "." и ".."
        if [[ "$dir" == "." || "$dir" == ".." ]]; then
            continue
        fi

        # Проверяем, является ли элемент директорией
        if [ -d "$dir" ]; then
            # Получаем время изменения директории (mtime) в формате Unix timestamp
            mtime=$(stat --format="%Y" "$dir")
            # Добавляем время изменения и имя директории в массив
            dirs+=("$mtime $dir")
        fi
    done

    # Сортируем массив по времени изменения (числовая сортировка по убыванию)
    printf "%s\n" "${dirs[@]}" | sort -nr | while read -r line; do
        # Извлекаем только имя директории из строки "время имя_директории"
        echo "${line#* }"
    done
}

# Получаем отсортированный список файлов по времени изменения (по убыванию)
echo "Files sorted by modification time (newest first):"
sorted_files=$(get_sorted_files_by_time_desc)
echo "$sorted_files"

# Получаем отсортированный список директорий по времени изменения (по убыванию)
echo "Directories sorted by modification time (newest first):"
sorted_dirs=$(get_sorted_dirs_by_time_desc)
echo "$sorted_dirs"