#include "scan.hpp"

namespace emocheck {

// https://en.cppreference.com/w/cpp/language/escape
static std::regex DIR_PAT("^[A-Za-z]{4,16}$");
static std::regex FILE_PAT("^(?!.*\\.(exe|dll)$)[A-Za-z]{3,15}\\.[A-Za-z]{3}$");

typedef std::unordered_set<std::string> strset;

void GenAllowList(std::string root, strset &allowlist, int depth = 0) {
    HANDLE hFind;
    WIN32_FIND_DATAA find_data;

    std::string curdir = root;
    curdir.erase(0, curdir.find_last_of("\\") + 1);
    std::string searchpath = root + "\\*";

    hFind = FindFirstFileA(searchpath.c_str(), &find_data);

    if (hFind == INVALID_HANDLE_VALUE)
        return;

    do {
        if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            std::string dirname = find_data.cFileName;
            if (dirname != "." && dirname != "..") {
                allowlist.insert(dirname);
                GenAllowList(root + "\\" + dirname, allowlist, depth + 1);
            }
        }
    } while (FindNextFileA(hFind, &find_data));

    FindClose(hFind);
    return;
}

bool FileExistInAllowList(strset &allowlist, std::string path) {
    std::string dirname = path;
    dirname.erase(dirname.find_last_of("\\"));
    dirname.erase(0, dirname.find_last_of("\\") + 1);

    if (allowlist.find(dirname) != allowlist.end())
        return true;
    else
        return false;
}

void PrintAllowList(strset allowlist) {
    for (std::string dirname : allowlist) {
        std::cout << dirname << std::endl;
    }
}

void TraverseFolder(std::string path, int depth, std::vector<std::string> &susp_files) {
    HANDLE hFind;
    WIN32_FIND_DATAA find_data;

    if (depth > 1)
        return;

    std::string search_path = path + "\\*";

    hFind = FindFirstFileA(search_path.c_str(), &find_data);

    if (hFind == INVALID_HANDLE_VALUE)
        return;

    do {
        if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && depth == 0) {
            std::string dirname = find_data.cFileName;
            if (dirname != "." && dirname != ".." &&
                std::regex_match(dirname, DIR_PAT)) {
                TraverseFolder(path + "\\" + dirname, depth + 1, susp_files);
            }
        } else if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && depth == 1) {
            std::string filename = find_data.cFileName;
            if (std::regex_match(filename, FILE_PAT)) {
                std::string susp_file = path + "\\" + filename;
                // add to suspicious file if the file path matches to emotet pattern and if the file is pe file.
                if (is_pefile(susp_file)) {
                    susp_files.push_back(susp_file);
                }
            }
        }
    } while (FindNextFileA(hFind, &find_data));

    FindClose(hFind);
    return;
}

// ユーザー権限でEmotetを実行した場合
//    step1: %LOCALAPPDATA%以下を探索
//    step2: レジストリキー登録を確認する
//    step3: rundll32.exeで呼び出すExport関数を確認する
// 管理者権限でEmotetを実行した場合
//    step1: C:\WINDOWS\system32(32bitOS) or C:\WINDOWS\SysWOW64(64bitOS) 以下を探索
//    step2: サービス登録を確認する
//    step3: rundll32.exeで呼び出すExport関数を確認する

std::vector<EmotetLoader> EmotetScannerV4() {
    DBG("====== Scan V4 =====");

    std::vector<EmotetLoader> suspicious;

    wchar_t path[MAX_PATH];
    std::vector<std::string> susp_files;

    std::string filename;

    std::vector<const char *> susp_patts;
    susp_patts.push_back("^.*rundll32\\.exe.*,.*RunDLL.*$");
    susp_patts.push_back("^.*rundll32\\.exe.*,.*ShowDialogA.*$");
    susp_patts.push_back("^.*rundll32\\.exe.*,.*[A-Za-z]{4,15}.*$");  // add random string pattern (https://app.any.run/tasks/7c9371c6-fd5b-43b0-b422-4a78b1a4eebf/ )

    static std::regex name_pat("^[A-Za-z]{7,16}$");

    // set spcial folder CSIDLs for allowlist.
    std::vector<unsigned short> allowbase = {CSIDL_APPDATA, CSIDL_PROGRAM_FILES, CSIDL_PROGRAM_FILESX86};

    // Get Special folder pathes
    SHGetSpecialFolderPathW(NULL, path, CSIDL_LOCAL_APPDATA, 0);
    std::string localappdata = WideCharToString(path);
    std::string sysdir_x86 = GetSysDirX86();

    // generate allowlist for scan_v4
    strset *allowlist = new strset();
    std::string root;
    for (auto csidl : allowbase) {
        SHGetSpecialFolderPathW(NULL, path, csidl, 0);
        root = WideCharToString(path);
        GenAllowList(root, *allowlist);
    }

    // PrintAllowList(*allowlist);

    // search for emotet executed with a normal user auth.
    std::string reg_path = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    std::vector<RegKeyValue *> runkeys = ListRegSZValues(HKEY_CURRENT_USER, reg_path);

    TraverseFolder(localappdata, 0, susp_files);

    for (std::string susp_path : susp_files) {
        // Ignore the suspicious file if the same file exists under a specified folder.
        if (FileExistInAllowList(*allowlist, susp_path)) {
            continue;
        }
        DBG("suspicious file:" << susp_path);
        filename = susp_path.substr(susp_path.find_last_of("\\") + 1);
        for (RegKeyValue *runkey : runkeys) {
            for (const char *susp_pat : susp_patts) {
                std::regex patt(susp_pat);
                if (runkey->name == filename || std::regex_search(runkey->name, name_pat)) {
                    if (std::regex_search(runkey->value, patt) && contain(runkey->value, filename.c_str())) {  // check registry value
                        DBG("suspicious reg name : " << runkey->name);
                        DBG("                reg value: " << runkey->value);
                        DBG("        unified reg value: " << PathUnification(runkey->value));
                        EmotetLoader *candi = new EmotetLoader();
                        candi->version = 4;
                        candi->filepath = susp_path;
                        candi->filename = filename;
                        candi->is_admin = false;
                        candi->run_key = "HKCU\\" + reg_path + "\\" + runkey->name;
                        candi->cmd_line = runkey->value;

                        Rundll32Cmd *rundll32 = new Rundll32Cmd();
                        rundll32 = ParseRundll32CmdLine(runkey->value);
                        if (rundll32) {
                            candi->rundll32 = *rundll32;
                        }

                        suspicious.push_back(*candi);
                        break;
                    }
                }
            }
        }
    }

    susp_files.clear();  // init vector of suspicious file list.

    // search for emotet executed with a privileged user.
    std::vector<WinService> services = ListWinServices();

    TraverseFolder(sysdir_x86, 0, susp_files);

    for (std::string susp_path : susp_files) {
        // Ignore the suspicious file if the same file exists under a specified folder.
        if (FileExistInAllowList(*allowlist, susp_path)) {
            continue;
        }
        DBG("suspicious file:" << susp_path);
        for (WinService srv : services) {
            filename = susp_path.substr(susp_path.find_last_of("\\") + 1);
            if (filename == srv.name) {
                DBG("suspicious service: " << srv.name);
                // get binary path of suspicious service.
                LPQUERY_SERVICE_CONFIG config = GetWinServiceConfig(srv.name);
                if (config) {
                    srv.binary_path = WideCharToString(config->lpBinaryPathName);
                    srv.start_type = config->dwStartType;
                }
                DBG("      binary path: " << srv.binary_path);
                // check the binary path of suspicous service.
                for (const char *susp_pat : susp_patts) {
                    std::regex patt(susp_pat);
                    if (std::regex_search(srv.binary_path, patt) && contain(srv.binary_path, filename.c_str())) {
                        EmotetLoader *candi = new EmotetLoader();
                        candi->version = 4;
                        candi->filepath = susp_path;
                        candi->filename = filename;
                        candi->is_admin = true;
                        candi->srv = srv;

                        Rundll32Cmd *rundll32 = new Rundll32Cmd();
                        rundll32 = ParseRundll32CmdLine(srv.binary_path);
                        if (rundll32) {
                            candi->rundll32 = *rundll32;
                        }
                        suspicious.push_back(*candi);
                    }
                }
            }
        }
    }
    return suspicious;
}

}  // namespace emocheck
