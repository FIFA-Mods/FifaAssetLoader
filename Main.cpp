#include "plugin.h"
#include <vector>
#include <filesystem>
#include "DebugPrint.h"

using namespace plugin;
using namespace std;
using namespace std::filesystem;

enum SearchPathFlags
{
    SEARCHPATH_ADD_TO_TAIL, ///< Put this search location after existing locations
    SEARCHPATH_ADD_TO_HEAD  ///< Put this search location before the existing locations
};

path GetIniSearchPath(wstring const &key) {
    wchar_t buf[MAX_PATH];
    GetPrivateProfileStringW(L"SearchPaths", key.c_str(), L"", buf, MAX_PATH, FIFA::GameDirPath(L"plugins\\AssetLoader.ini").c_str());
    return path(ToUTF8(buf));
}

uintptr_t gSetSearchPath = 0;

void METHOD FileSysManager_SetSearchPath(void *t, DUMMY_ARG, char const *searchPath) {
    vector<path> searchPaths;
    path s = searchPath;
    for (size_t i = 1; i <= 10; i++) {
        auto p = GetIniSearchPath(Format(L"Path%d", i));
        if (!p.is_absolute())
            p = s / p;
        if (find(searchPaths.begin(), searchPaths.end(), p) == searchPaths.end())
            searchPaths.push_back(p);
    }
    if (find(searchPaths.begin(), searchPaths.end(), s) == searchPaths.end())
        searchPaths.push_back(searchPath);
    string newSearchPath;
    bool first = true;
    for (auto const &p : searchPaths) {
        if (first)
            first = false;
        else
            newSearchPath += ';';
        newSearchPath += p.string();
    }
    CallMethodDynGlobal(gSetSearchPath, t, newSearchPath.c_str());
}

void METHOD OnAddSearchLocation(void *t, DUMMY_ARG, const char *NewLocation, unsigned int flags) {
    CallMethod<0xB71EF0>(t, NewLocation, flags);
    DebugPrint(Format("AddedSearchLocation: %s, %s\n", NewLocation, flags ? "head" : "tail"));
}

void METHOD OnSetSearchPath(void *t, DUMMY_ARG, const char *p) {
    CallMethod<0xB71EE0>(t, p);
    DebugPrint(Format("SetSearchPath: %s\n", p));
}

void METHOD OnUpdateSearchPath(void *t, DUMMY_ARG, const char *p) {
    CallMethod<0xB71DE0>(t, p);
    DebugPrint(Format("Search path updated: %s\n", p));
}

class FifaAssetLoader {
public:
    FifaAssetLoader() {
        if (!CheckPluginName(Magic<'A','s','s','e','t','L','o','a','d','e','r','.','a','s','i'>()))
            return;
        auto v = FIFA::GetAppVersion();
        switch (v.id()) {
        case ID_FIFA13_1700_RLD:
            patch::SetUChar(0x1299B3E + 1, SEARCHPATH_ADD_TO_TAIL);
            patch::SetUChar(0x1299B81 + 1, SEARCHPATH_ADD_TO_TAIL);
            gSetSearchPath = patch::RedirectCall(0x1299A0C, FileSysManager_SetSearchPath);
            break;
        case ID_FIFA13_1800:
            patch::SetUChar(0x129EADE + 1, SEARCHPATH_ADD_TO_TAIL);
            patch::SetUChar(0x129EB21 + 1, SEARCHPATH_ADD_TO_TAIL);
            gSetSearchPath = patch::RedirectCall(0x129E9AC, FileSysManager_SetSearchPath);
            break;
        case ID_FIFA12_1700:
            patch::SetUChar(0xB9BCB5 + 1, SEARCHPATH_ADD_TO_TAIL);
            patch::SetUChar(0xB9BCF8 + 1, SEARCHPATH_ADD_TO_TAIL);
            gSetSearchPath = patch::RedirectCall(0xB9BB62, FileSysManager_SetSearchPath);
            break;
        case ID_FIFA12_1500_SKD:
            patch::SetUChar(0xB99FB5 + 1, SEARCHPATH_ADD_TO_TAIL);
            patch::SetUChar(0xB99FF8 + 1, SEARCHPATH_ADD_TO_TAIL);
            gSetSearchPath = patch::RedirectCall(0xB99E62, FileSysManager_SetSearchPath);


            //patch::RedirectCall(0xB3FF3C, OnAddSearchLocation);
            //patch::RedirectCall(0xB3FFD6, OnAddSearchLocation);
            //patch::RedirectCall(0xB99FF3, OnAddSearchLocation);
            //patch::RedirectCall(0xB9A001, OnAddSearchLocation);
            //patch::RedirectCall(0xCB92BE, OnAddSearchLocation);
            //patch::RedirectCall(0xD0D132, OnAddSearchLocation);
            //patch::RedirectCall(0xD36E5F, OnAddSearchLocation);
            //patch::RedirectCall(0x10230EC, OnAddSearchLocation);
            //patch::RedirectCall(0xB99E62, OnSetSearchPath);
            //patch::RedirectCall(0xCB92A9, OnSetSearchPath);

            patch::RedirectJump(0xB71EE0, OnUpdateSearchPath);
            patch::RedirectCall(0xB7201D, OnUpdateSearchPath);
            patch::RedirectCall(0xB721B1, OnUpdateSearchPath);
            patch::RedirectCall(0xB726F1, OnUpdateSearchPath);
            break;
        case ID_FIFA12_1000_RLD:
            patch::SetUChar(0x516085 + 1, SEARCHPATH_ADD_TO_TAIL);
            patch::SetUChar(0x5160C8 + 1, SEARCHPATH_ADD_TO_TAIL);
            gSetSearchPath = patch::RedirectCall(0x515F32, FileSysManager_SetSearchPath);
            break;
        case ID_FIFA11_1010_RLD:
        case ID_FIFA11_1010:
            patch::SetUChar(0x8E8E0C + 1, SEARCHPATH_ADD_TO_TAIL);
            patch::SetUChar(0x8E8E4F + 1, SEARCHPATH_ADD_TO_TAIL);
            gSetSearchPath = patch::RedirectCall(0x8E8D11, FileSysManager_SetSearchPath);
            break;
        }
    }
} fifaAssetLoader;
