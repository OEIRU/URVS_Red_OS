// *******************************************************************************************************
// Программа отображает информацию о количестве мониторов и поддержке инструкции RDTSC, используя данные из DLL.
// 
// Используется:
// - Многопоточность для асинхронной загрузки данных
// - Динамическая загрузка библиотеки winlib.dll
// - Inline-ассемблер (требуется 32-битная сборка)
// *******************************************************************************************************

#include <windows.h>     // Базовые функции Windows API
#include <string>        // Работа со строками
#include <sstream>       // Строковые потоки для форматирования вывода
using namespace std;

// *******************************************************************************************************
// Глобальные переменные и константы
// *******************************************************************************************************

// Основное окно и статическая метка
HWND hwnd = NULL;     // Хэндл главного окна
HWND label = NULL;    // Хэндл статической метки для отображения данных

// Cообщение для обновления текста метки из потока
#define WM_UPDATE_LABEL (WM_USER + 1)  // Уникальный идентификатор сообщения

// Цветовая палитра для дизайна
const COLORREF BG_COLOR = RGB(25, 25, 35);     // Темно-серый фон окна
const COLORREF TEXT_COLOR = RGB(255, 255, 255); // Белый текст

// Кастомный шрифт для метки
HFONT hFont = NULL;  // Указатель на шрифт Segoe UI

// *******************************************************************************************************
// Функция: thread
// Назначение: Выполняет загрузку данных из DLL в отдельном потоке и отправляет результат в основной поток
// Алгоритм:
// 1. Загружает winlib.dll
// 2. Получает адреса функций numb_monitors_back и search_rdtsc
// 3. Вызывает функции и формирует результат
// 4. Отправляет результат в основной поток через WM_UPDATE_LABEL
// Предположения:
// - winlib.dll находится в той же директории, что и исполняемый файл
// - Функции в DLL имеют корректные сигнатуры
// *******************************************************************************************************
DWORD WINAPI thread(LPVOID lpParam) {
    HINSTANCE hinstLib = LoadLibrary(TEXT("winlib.dll"));  // Загрузка библиотеки
    wstringstream lbl;  // Строковый поток для формирования вывода

    if (hinstLib != NULL) {
        // Получение функции numb_monitors_back
        typedef int (*numb_monitors_back_)();
        numb_monitors_back_ numb_monitors_back = (numb_monitors_back_)GetProcAddress(hinstLib, "numb_monitors_back");
        if (numb_monitors_back != NULL) {
            // Формирование строки с информацией о мониторах
            lbl << L"Vladimir Kostovsky PM-23" << endl
                << L"\t\t3th Level" << endl
                << L"Monitors: " << numb_monitors_back() << endl << endl;
        }

        // Получение функции search_rdtsc
        typedef int (*search_rdtsc_)();
        search_rdtsc_ search_rdtsc = (search_rdtsc_)GetProcAddress(hinstLib, "search_rdtsc");
        if (search_rdtsc != NULL) {
            int tsc = search_rdtsc();  // Вызов функции из DLL
            // Формирование строки с информацией о поддержке RDTSC
            lbl << L"\t\t4th Level" << endl
                << L"RDTSC Support: ";
            if (tsc > 0) lbl << L"YES";
            else lbl << L"NO";
        }

        FreeLibrary(hinstLib);  // Освобождение библиотеки
    }
    else {
        // Обработка ошибки загрузки DLL
        MessageBoxW(NULL, L"winlib.dll not found", L"Error", MB_OK | MB_ICONERROR);
    }

    // Динамическое выделение памяти для строки
    std::wstring* text = new std::wstring(lbl.str());
    PostMessage(hwnd, WM_UPDATE_LABEL, 0, (LPARAM)text);  // Отправка результата в основной поток
    return 0;
}

// *******************************************************************************************************
// Обработчик окна: WndProc
// Назначение: Обработка оконных сообщений, управление цветом и обновлением интерфейса
// Сообщения:
// - WM_CTLCOLORSTATIC: Настройка цвета текста и фона для метки
// - WM_UPDATE_LABEL: Обновление текста метки
// - WM_DESTROY: Очистка ресурсов и завершение работы
// *******************************************************************************************************
LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    static HBRUSH hBgBrush = CreateSolidBrush(BG_COLOR);        // Кисть для фона окна
    static HBRUSH hLabelBgBrush = CreateSolidBrush(RGB(40, 40, 50));  // Кисть для фона метки

    switch (Msg) {
        // Настройка цвета статического текста
    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        HWND ctrl = (HWND)lParam;

        if (GetDlgCtrlID(ctrl) == 1001) {
            SetTextColor(hdc, TEXT_COLOR);         // Белый текст
            SetBkColor(hdc, RGB(40, 40, 50));     // Темно-серый фон
            return (LRESULT)hLabelBgBrush;         // Возврат кисти для фона
        }
    } break;

                          // Обновление текста метки из потока
    case WM_UPDATE_LABEL: {
        std::wstring* text = (std::wstring*)lParam;
        SetWindowTextW(label, text->c_str());  // Установка текста метки
        delete text;                           // Освобождение памяти
    } break;

                        // Очистка ресурсов при закрытии окна
    case WM_DESTROY:
        DeleteObject(hBgBrush);
        DeleteObject(hLabelBgBrush);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, Msg, wParam, lParam);
    }
    return 0;
}

// *******************************************************************************************************
// Точка входа: WinMain
// Назначение: Инициализация окна, создание элементов интерфейса, запуск потока
// Этапы:
// 1. Регистрация класса окна
// 2. Создание окна и статической метки
// 3. Выбор шрифта
// 4. Запуск потока для загрузки данных
// *******************************************************************************************************
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Регистрация класса окна
    WNDCLASS wc = {};
    wc.style = CS_HREDRAW | CS_VREDRAW;         // Перерисовка при изменении размеров
    wc.lpfnWndProc = WndProc;                   // Указатель на обработчик сообщений
    wc.hInstance = hInstance;                   // Дескриптор экземпляра
    wc.hbrBackground = CreateSolidBrush(BG_COLOR);  // Темно-серый фон
    wc.lpszClassName = TEXT("ModernWindowClass");   // Имя класса
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);   // Стандартный курсор
    RegisterClass(&wc);

    // Размеры окна
    const int window_width = 500;
    const int window_height = 300;

    // Создание окна
    hwnd = CreateWindow(
        TEXT("ModernWindowClass"), TEXT("System Info"),  // Имя класса и заголовок
        WS_OVERLAPPEDWINDOW,                            // Стандартное окно
        CW_USEDEFAULT, CW_USEDEFAULT,                   // Автоматическое позиционирование
        window_width, window_height,                    // Размеры
        NULL, NULL, hInstance, NULL                      // Параметры по умолчанию
    );

    if (!hwnd) {
        MessageBox(NULL, TEXT("Window creation failed!"), TEXT("Error"), MB_OK);
        return 0;
    }

    // Получение клиентской области для точного позиционирования метки
    RECT rt;
    GetClientRect(hwnd, &rt);

    // Создание статической метки
    label = CreateWindow(
        TEXT("static"), TEXT("Loading..."),              // Имя класса и начальный текст
        WS_CHILD | WS_VISIBLE | SS_CENTER | WS_BORDER,   // Стили: центрированный текст с рамкой
        10, 10, window_width - 40, window_height - 50,   // Позиция и размеры
        hwnd, (HMENU)1001, hInstance, NULL               // Идентификатор метки
    );

    if (!label) {
        MessageBox(hwnd, TEXT("Label creation failed!"), TEXT("Error"), MB_OK);
    }

    // Создание кастомного шрифта Segoe UI
    hFont = CreateFont(
        18, 0, 0, 0, FW_NORMAL,                          // Размер и стиль
        FALSE, FALSE, FALSE,                             // Курсив, подчеркивание, зачеркивание
        DEFAULT_CHARSET,                                 // Кодировка
        OUT_DEFAULT_PRECIS,                              // Точность вывода
        CLIP_DEFAULT_PRECIS,                             // Точность обрезки
        CLEARTYPE_QUALITY,                               // Качество ClearType
        FF_DONTCARE,                                     // Семейство шрифта
        TEXT("Segoe UI")                                 // Имя шрифта
    );

    if (hFont) {
        SendDlgItemMessage(hwnd, 1001, WM_SETFONT, (WPARAM)hFont, TRUE);  // Установка шрифта для метки
    }

    // Создание потока для загрузки данных
    HANDLE hThread = CreateThread(NULL, 0, thread, NULL, 0, NULL);
    if (!hThread) {
        MessageBox(hwnd, TEXT("Failed to create thread"), TEXT("Error"), MB_OK);
    }

    // Отображение окна
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Цикл сообщений
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}