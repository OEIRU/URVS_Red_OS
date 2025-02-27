#!/bin/bash

# Общая функция для получения отсортированного списка элементов по времени изменения.
# Функция принимает два параметра:
# $1 - тип элемента ("file", "dir" или "link").
# $2 - описание типа элемента для вывода сообщений (например, "regular files" для файлов).
get_sorted_elements_by_time_desc() {
    local type="$1" # Тип элемента: "file", "dir" или "link"
    local description="$2" # Описание типа элемента для вывода сообщений

    # Проверяем, существует ли текущая директория.
    # Если директория не существует, выводим ошибку и завершаем работу функции.
    if [[ ! -d . ]]; then
        echo "Error: Current directory does not exist." >&2
        return 1
    fi

    # Массив для хранения информации о найденных элементах.
    declare -a elements=()
    local count=0 # Счетчик найденных элементов

    # Перебираем все элементы в текущей директории, включая скрытые.
    # Используем шаблон ".* *" для включения скрытых файлов/директорий.
    for element in .* *; do
        # Пропускаем специальные записи "." и "..".
        if [[ "$element" == "." || "$element" == ".." ]]; then
            continue
        fi

        # Работаем с типами элементов.
        case "$type" in
            file)
                # Проверяем, является ли элемент обычным файлом и доступен ли он для чтения.
                if [[ -f "$element" && -r "$element" ]]; then
                    # Получаем время изменения файла с помощью команды stat.
                    # Если команда stat завершается с ошибкой, выводим предупреждение и пропускаем этот элемент.
                    mtime=$(stat --format="%Y" "$element" 2>/dev/null) || {
                        echo "Warning: Could not get modification time for file '$element'." >&2
                        continue
                    }
                    # Добавляем время изменения и имя файла в массив.
                    elements+=("$mtime $element")
                    ((count++)) # Увеличиваем счетчик найденных файлов
                elif [[ ! -r "$element" ]]; then
                    # Если у нас нет прав на чтение элемента, выводим предупреждение.
                    echo "Warning: No read permission for '$element'." >&2
                fi
                ;;
            dir)
                # Проверяем, является ли элемент директорией и доступен ли он для чтения.
                if [[ -d "$element" && -r "$element" ]]; then
                    # Получаем время изменения директории с помощью команды stat.
                    # Если команда stat завершается с ошибкой, выводим предупреждение и пропускаем этот элемент.
                    mtime=$(stat --format="%Y" "$element" 2>/dev/null) || {
                        echo "Warning: Could not get modification time for directory '$element'." >&2
                        continue
                    }
                    # Добавляем время изменения и имя директории в массив.
                    elements+=("$mtime $element")
                    ((count++)) # Увеличиваем счетчик найденных директорий
                elif [[ ! -r "$element" ]]; then
                    # Если у нас нет прав на чтение элемента, выводим предупреждение.
                    echo "Warning: No read permission for '$element'." >&2
                fi
                ;;
            link)
                # Проверяем, является ли элемент символической ссылкой.
                if [[ -h "$element" || -L "$element" ]]; then
                    # Получаем время изменения ссылки с помощью команды stat.
                    # Если команда stat завершается с ошибкой, выводим предупреждение и пропускаем этот элемент.
                    mtime=$(stat --format="%Y" "$element" 2>/dev/null) || {
                        echo "Warning: Could not get modification time for symbolic link '$element'." >&2
                        continue
                    }
                    # Добавляем время изменения и имя ссылки в массив.
                    elements+=("$mtime $element")
                    ((count++)) # Увеличиваем счетчик найденных ссылок
                fi
                ;;
            *)
                # Если передан некорректный тип элемента, выводим ошибку и завершаем работу функции.
                echo "Error: Unknown element type '$type'." >&2
                return 1
                ;;
        esac
    done

    # Если элементов заданного типа не найдено, выводим соответствующее сообщение.
    if [[ $count -eq 0 ]]; then
        echo "No $description found in the current directory."
        return 0
    fi

    # Сортируем массив элементов по времени изменения (числовая сортировка по убыванию).
    # Каждый элемент массива имеет формат "время имя_элемента".
    printf "%s\n" "${elements[@]}" | sort -nr | while read -r line; do
        # Извлекаем только имя элемента из строки "время имя_элемента".
        echo "${line#* }"
    done
}

# Основной скрипт

# Выводим отсортированные списки файлов, используя общую функцию.
echo "Files sorted by modification time (newest first):"
# Вызываем функцию для обработки файлов.
# Первый параметр указывает тип элемента ("file"), второй — описание типа ("regular files").
get_sorted_elements_by_time_desc "file" "regular files"

# Выводим отсортированные списки директорий, используя общую функцию.
echo "Directories sorted by modification time (newest first):"
# Вызываем функцию для обработки директорий.
# Первый параметр указывает тип элемента ("dir"), второй — описание типа ("directories").
get_sorted_elements_by_time_desc "dir" "directories"

# Выводим отсортированные списки символьных ссылок, используя общую функцию.
echo "Symbolic links sorted by modification time (newest first):"
# Вызываем функцию для обработки символьных ссылок.
# Первый параметр указывает тип элемента ("link"), второй — описание типа ("symbolic links").
get_sorted_elements_by_time_desc "link" "symbolic links"