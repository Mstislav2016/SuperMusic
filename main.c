#ifndef UNICODE
#define UNICODE
#endif
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <wchar.h>

// Идентификаторы кнопок
#define ID_LISTBOX   101
#define ID_BTN_PLAY  102
#define ID_BTN_STOP  103
#define ID_BTN_NEXT  104
#define ID_BTN_PREV  105

HWND hListBox;
wchar_t folderPath[MAX_PATH];

// Функция воспроизведения
void PlayMusic(const wchar_t* filename) {
    wchar_t command[2048];
    wchar_t fullPath[MAX_PATH];
    wchar_t shortPath[MAX_PATH];
    
    mciSendStringW(L"close mp3", NULL, 0, NULL);
    swprintf(fullPath, MAX_PATH, L"%ls\\%ls", folderPath, filename);
    
    // Превращаем путь в короткий (DOS-формат), чтобы избежать проблем с пробелами и кириллицей в MCI
    if (GetShortPathNameW(fullPath, shortPath, MAX_PATH) == 0) {
        swprintf(command, 2048, L"open \"%ls\" type mpegvideo alias mp3", fullPath);
    } else {
        swprintf(command, 2048, L"open %ls type mpegvideo alias mp3", shortPath);
    }
    
    if (mciSendStringW(command, NULL, 0, NULL) != 0) {
        return;
    }
    mciSendStringW(L"play mp3", NULL, 0, NULL);
}

// Загрузка песен из папки Super_music
void LoadPlaylist(HWND hList) {
    WIN32_FIND_DATAW findData;
    wchar_t searchPath[MAX_PATH];
    swprintf(searchPath, MAX_PATH, L"%ls\\*.mp3", folderPath);

    HANDLE hFind = FindFirstFileW(searchPath, &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            SendMessageW(hList, LB_ADDSTRING, 0, (LPARAM)findData.cFileName);
        } while (FindNextFileW(hFind, &findData));
        FindClose(hFind);
        SendMessageW(hList, LB_SETCURSEL, 0, 0); // Выбираем первую песню по умолчанию
    } else {
        CreateDirectoryW(folderPath, NULL);
    }
}

// Обработка логики окна
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            // Создаем кнопки
            CreateWindowW(L"BUTTON", L"⭠ Назад", WS_VISIBLE | WS_CHILD, 10, 310, 70, 30, hwnd, (HMENU)ID_BTN_PREV, NULL, NULL);
            CreateWindowW(L"BUTTON", L"▶ Играть", WS_VISIBLE | WS_CHILD, 85, 310, 70, 30, hwnd, (HMENU)ID_BTN_PLAY, NULL, NULL);
            CreateWindowW(L"BUTTON", L"⏸ Стоп", WS_VISIBLE | WS_CHILD, 160, 310, 70, 30, hwnd, (HMENU)ID_BTN_STOP, NULL, NULL);
            CreateWindowW(L"BUTTON", L"⭢ След.", WS_VISIBLE | WS_CHILD, 235, 310, 70, 30, hwnd, (HMENU)ID_BTN_NEXT, NULL, NULL);
            
            // Список файлов
            hListBox = CreateWindowW(L"LISTBOX", NULL, WS_VISIBLE | WS_CHILD | LBS_NOTIFY | WS_VSCROLL | WS_BORDER, 10, 10, 295, 280, hwnd, (HMENU)ID_LISTBOX, NULL, NULL);
            
            // Определяем путь к папке Super_music рядом с EXE
            GetModuleFileNameW(NULL, folderPath, MAX_PATH);
            wchar_t* lastSlash = wcsrchr(folderPath, L'\\');
            if (lastSlash) *(lastSlash + 1) = L'\0';
            wcscat(folderPath, L"Super_music");

            LoadPlaylist(hListBox);
            break;
        }
        case WM_COMMAND: {
            int sel = (int)SendMessageW(hListBox, LB_GETCURSEL, 0, 0);
            int count = (int)SendMessageW(hListBox, LB_GETCOUNT, 0, 0);

            if (LOWORD(wParam) == ID_BTN_PLAY) {
                if (sel != LB_ERR) {
                    wchar_t fileName[256];
                    SendMessageW(hListBox, LB_GETTEXT, sel, (LPARAM)fileName);
                    PlayMusic(fileName);
                }
            }
            if (LOWORD(wParam) == ID_BTN_STOP) {
                mciSendStringW(L"stop mp3", NULL, 0, NULL);
            }
            if (LOWORD(wParam) == ID_BTN_NEXT) {
                if (count > 0) {
                    // Если это последняя песня (count-1), переходим на 0, иначе на sel+1
                    int nextSel = (sel >= count - 1) ? 0 : sel + 1;
                    SendMessageW(hListBox, LB_SETCURSEL, nextSel, 0);
                    SendMessageW(hwnd, WM_COMMAND, ID_BTN_PLAY, 0);
                }
            }
            if (LOWORD(wParam) == ID_BTN_PREV) {
                if (count > 0) {
                    // Если это первая песня (0), переходим на последнюю (count-1), иначе на sel-1
                    int prevSel = (sel <= 0) ? (count - 1) : sel - 1;
                    SendMessageW(hListBox, LB_SETCURSEL, prevSel, 0);
                    SendMessageW(hwnd, WM_COMMAND, ID_BTN_PLAY, 0);
                }
            }
            break;
        }
        case WM_DESTROY:
            mciSendStringW(L"close mp3", NULL, 0, NULL);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int main() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    const wchar_t CLASS_NAME[] = L"MusicPlayerClass";
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassW(&wc);
    HWND hwnd = CreateWindowExW(0, CLASS_NAME, L"C Player (Super Music)", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE, 
                                CW_USEDEFAULT, CW_USEDEFAULT, 330, 400, NULL, NULL, hInstance, NULL);

    MSG msg = {0};
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}