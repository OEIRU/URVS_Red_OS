#!/bin/bash

# Функция для получения списка файлов, отсортированных по времени изменения
get_sorted_files_by_time() {
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
    
    # Сортируем массив по времени изменения (числовая сортировка)
    printf "%s\n" "${files[@]}" | sort -n | while read -r line; do
        # Извлекаем только имя файла из строки "время имя_файла"
        echo "${line#* }"
    done
}

# Функция для получения списка директорий, отсортированных по времени изменения
get_sorted_dirs_by_time() {
    # Массив для хранения информации о директориях
    declare -a dirs=()
    
    # Перебираем все элементы в текущей директории, включая скрытые
    for dir in .* *; do
        # Пропускаем специальные записи "." и ".."
        if [[ "$file" == "." || "$file" == ".." ]]; then
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
    
    # Сортируем массив по времени изменения (числовая сортировка)
    printf "%s\n" "${dirs[@]}" | sort -n | while read -r line; do
        # Извлекаем только имя директории из строки "время имя_директории"
        echo "${line#* }"
    done
}

# Получаем отсортированный список файлов по времени изменения
echo "Files sorted by modification time:"
sorted_files=$(get_sorted_files_by_time)
echo "$sorted_files"

# Получаем отсортированный список директорий по времени изменения
echo "Directories sorted by modification time:"
sorted_dirs=$(get_sorted_dirs_by_time)
echo "$sorted_dirs"