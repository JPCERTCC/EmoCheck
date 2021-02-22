#include "scan.hpp"

namespace emocheck {

void TraverseFolder7Day(std::string path, int depth, std::vector<std::string> &susp_files, FILETIME *dir_lastwrite) {
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
            if (dirname != "." && dirname != "..") {
                TraverseFolder7Day(path + "\\" + dirname, depth + 1, susp_files, &(find_data.ftLastWriteTime));
            }
        } else if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && depth == 1) {
            std::string filename = find_data.cFileName;
            std::string susp_file = path + "\\" + filename;
            // the diff of timestamp exactly match within 7days +- 100ms (resolution of time: 100ms)
            long long int diff = TakeDiffFileTime(&(find_data.ftLastWriteTime), dir_lastwrite);
            if (6047000000000LL <= diff && diff <= 6049000000000LL) {
                DBG("[v5] susp file: " << filename << "\t filetime_diff: " << diff / 10000 << "(ms)");
            }
            diff /= 1000000LL;
            if (is_pefile(susp_file) && 6047997LL <= diff && diff <= 6048002LL) {
                DBG("suspicious file:" << susp_file << "\t");
                susp_files.push_back(susp_file);
            }
        }
    } while (FindNextFileA(hFind, &find_data));

    FindClose(hFind);
    return;
}

std::vector<EmotetLoader> EmotetScannerV5() {
    std::vector<EmotetLoader> suspicious;
    std::vector<std::string> susp_files;

    std::string filename;

    DBG("====== Scan V5 =====");

    wchar_t path[MAX_PATH];
    SHGetSpecialFolderPathW(NULL, path, CSIDL_LOCAL_APPDATA, 0);
    std::string searchpath = WideCharToString(path);

    TraverseFolder7Day(searchpath, 0, susp_files, nullptr);
    TraverseFolder7Day(GetSysDirX86(), 0, susp_files, nullptr);

    for (std::string susp_path : susp_files) {
        EmotetLoader *candi = new EmotetLoader();
        filename = susp_path.substr(susp_path.find_last_of("\\") + 1);
        candi->version = 5;
        candi->filepath = susp_path;
        candi->filename = filename;
        candi->is_admin = contain(susp_path, GetSysDirX86().c_str());
        suspicious.push_back(*candi);
    }
    return suspicious;
}

}  // namespace emocheck