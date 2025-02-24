#!/bin/bash

# Начало скрипта

# Функция для получения списка файлов и их сортировки по размеру
get_sorted_files() {
    # Массив для хранения информации о файлах
    declare -a files=()

    # Перебираем все элементы в текущей директории
    for file in *; do
        # Проверяем, является ли элемент файлом
        if [ -f "$file" ]; then
            # Получаем размер файла с помощью stat
            size=$(stat --format="%s" "$file")
            # Добавляем размер и имя файла в массив
            files+=("$size $file")
        fi
    done

    # Сортируем массив по размеру (числовая сортировка)
    printf "%s\n" "${files[@]}" | sort -nr
}

# Функция для получения списка директорий
get_sorted_dirs() {
    # Массив для хранения имен директорий
    declare -a dirs=()

    # Перебираем все элементы в текущей директории
    for dir in *; do
        # Проверяем, является ли элемент директорией
        if [ -d "$dir" ]; then
            # Добавляем имя директории в массив
            dirs+=("$dir")
        fi
    done

    # Выводим список директорий
    printf "%s\n" "${dirs[@]}"
}

# Получаем отсортированный список файлов
echo "Sorted files by size:"
sorted_files=$(get_sorted_files)
echo "$sorted_files"

# Получаем список директорий
echo "Directories:"
sorted_dirs=$(get_sorted_dirs)
echo "$sorted_dirs"