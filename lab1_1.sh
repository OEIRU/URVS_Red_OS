#!/bin/bash

# Сортируем файлы по размеру
sorted_files=$(ls -alt | grep -v '^d')
sorted_dirs=$(ls -alt | grep '^d')

# Выводим отсортированный список файлов
echo "$sorted_files"

echo "$sorted_dirs"
