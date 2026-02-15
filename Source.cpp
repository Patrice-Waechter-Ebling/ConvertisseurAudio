#include <iostream>
#include <filesystem>
#include <string>
#include <cstdlib>
#include <windows.h>
#include <shlobj.h> // Ajout requis pour SHGetKnownFolderPath et FOLDERID_Music
#pragma comment(lib, "shell32.lib")

namespace fs = std::filesystem;
bool is_audio_file(const fs::path& p)
{
    std::string ext = p.extension().string();
    for (auto& c : ext) c = tolower(c);
    return ext == ".wav" || ext == ".flac" || ext == ".aac" || ext == ".ogg" || ext == ".m4a" || ext == ".wma" || ext == ".mp4" || ext == ".mp3" || ext == ".ac3";
}

std::wstring BrowseForFolder(const wchar_t* title)
{
    BROWSEINFOW bi = { 0 };
    bi.lpszTitle = title;
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    PIDLIST_ABSOLUTE pidl = SHBrowseForFolderW(&bi);
    if (!pidl) return L"";
    wchar_t path[MAX_PATH];
    if (!SHGetPathFromIDListW(pidl, path))return L"";
    CoTaskMemFree(pidl);
    return path;
}

int main()
{
    std::wcout << L"ConvertisseurAudio Rapide via FFMpeg \tv:1.00\t(C)Patrice Waechter-Ebling \n\nChoisissez le dossier source contenant les fichiers audio...\n";
    std::wstring sourceFolder = BrowseForFolder(L"Sélectionnez le dossier source");
    if (sourceFolder.empty()) {std::cout << "Aucun dossier sélectionné.\n";return 1;}
    fs::path sourceRoot = sourceFolder;
    std::wcout << L"Dossier source : " << sourceRoot.wstring() << L"\n\n";
    PWSTR musicPath = nullptr;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_Music, 0, NULL, &musicPath);
    if (FAILED(hr)) {std::cerr << "Erreur : impossible de trouver le dossier 'Ma musique'.\n";return 1;}
    fs::path outputRoot = fs::path(musicPath);
    CoTaskMemFree(musicPath);
    std::wcout << L"Dossier destination : " << outputRoot.wstring() << L"\n\n";
    for (auto& entry : fs::recursive_directory_iterator(sourceRoot))// --- Parcours récursif ---
    {
        if (!entry.is_regular_file()) continue;
        fs::path input = entry.path();
        if (!is_audio_file(input)) continue;
        fs::path relative = fs::relative(input, sourceRoot);// Chemin relatif pour recréer l’arborescence
        fs::path output = outputRoot / relative;
        output.replace_extension(".mp3");
        fs::create_directories(output.parent_path());// Création des dossiers
        std::string cmd = "ffmpeg -y -i \"" + input.string() + "\" -vn -ar 44100 -ac 2 -b:a 128k \"" + output.string() + "\" >nul 2>&1";
        std::wcout << L"Conversion : " << input.wstring() << L"\n";
        int result = system(cmd.c_str());

      /*  
      if (result == 0 && fs::exists(output)) {
        std::wcout << L"  -> OK, suppression de l'original\n";
        fs::remove(input);
        }    else {
        std::wcout << L"  -> ERREUR conversion\n";
        }
    */
    }
    std::cout << "\nConversion terminée.\n";
    system("pause");
    return 0;
}
