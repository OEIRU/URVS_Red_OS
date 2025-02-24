#!/bin/bash

# Начало скрипта

# Функция для получения списка файлов и их сортировки по размеру
get_sorted_files() {
    # Используем find для поиска всех файлов в текущей директории (не рекурсивно)
    # Используем stat для получения размера файла и его имени
    find . -maxdepth 1 -type f -exec stat --format="%s %n" {} + | sort -nr
}

# Функция для получения списка директорий
get_sorted_dirs() {
    # Используем find для поиска всех директорий в текущей директории (не рекурсивно)
    # Выводим только имена директорий
    find . -maxdepth 1 -type d -not -name "." -exec echo {} \;
}

# Получаем отсортированный список файлов
echo "Sorted files by size:"
sorted_files=$(get_sorted_files)
echo "$sorted_files"

# Получаем список директорий
echo "Directories:"
sorted_dirs=$(get_sorted_dirs)
echo "$sorted_dirs"