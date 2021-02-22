#include "emocheck.hpp"

#include "modules/scan.hpp"
#include "utils/file.hpp"
#include "utils/native.hpp"
#include "utils/process.hpp"
namespace emocheck {
bool is_debug = false;

void PrintAuthError(int pid) {
    std::cout << "[*] Fail to acquire imagepath from pid:" << pid << std::endl;
    std::cout << "    To acquire the file path of the suspicious process," << std::endl;
    std::cout << "    please run Emocheck with the administrative user." << std::endl;
}

void DebugProcessInfo(Proc proc) {
    DBG("Find suspicious process: " << proc.name);
    DBG("                    pid: " << proc.PID);
    DBG("           command line: " << proc.cmd_line);
    DBG("   unified command line: " << PathUnification(proc.cmd_line));
    return;
}

std::vector<EmotetProcess> ScanEmotetProcess(std::vector<EmotetLoader> candidates) {
    std::vector<EmotetProcess> emotet_procs;
    EmotetProcess *emo_proc;
    std::vector<Proc> process_list = ListProcess();
    std::set<int> pids;
    DBG("====== Scan Processes =====");

    for (Proc proc : process_list) {
        for (EmotetLoader susp : candidates) {
            bool is_emotet = false;

            if (proc.name == susp.filename) {
                DebugProcessInfo(proc);
                switch (susp.version) {
                    case 1:
                    case 2:
                        is_emotet = true;
                        break;
                    case 3:
                        // false positive
                        if (!ComparePath(susp.filepath, proc.image_path))
                            break;
                        // false positive
                        if (!susp.file_validation)
                            break;
                        // auth error
                        if (proc.image_path == "") {
                            PrintAuthError(proc.PID);
                            break;
                        }
                        is_emotet = true;
                        break;
                    default:
                        break;
                }
            } else if (susp.version == 4 && proc.name == "rundll32.exe") {
                // Scan_v4 only supports rundll32.exe processes.
                // Emotet: user-auth, Emocheck:user-auth (x86, x64) [2020/12-] -> normal
                // Emotet: admin-auth, Emocheck: admin-auth (x86) [2020/12-]    -> normal
                // Emotet: admin-auth, Emocheck: admin-auth (x64) [2020/12-]   -> wow64 path redirection
                // Emotet: admin-auth, Emocheck: user-auth (x86, x64)          -> auth error
                // We should consider the SysWow64 file-system redirector when checking the emotet image path contained in rundll32.exe command line.
                // https://docs.microsoft.com/en-us/windows/win32/winprog64/file-system-redirector

                DebugProcessInfo(proc);
                if (contain(PathUnification(proc.cmd_line), PathUnification(susp.filepath).c_str())) {
                    is_emotet = true;
                } else if (susp.is_admin) {
                    if (proc.cmd_line == "") {
                        PrintAuthError(proc.PID);
                    } else {
                        // syswow64 path redirection
                        std::string redirect_path = PathUnification(susp.filepath);
                        redirect_path = ReplaceString(redirect_path, "syswow64", "system32");
                        if (contain(PathUnification(proc.cmd_line), redirect_path.c_str())) {
                            is_emotet = true;
                        }
                    }
                }
            } else if (susp.version == 5) {
                // Scan_v5 supports all versions of Emotet.
                // Emotet: user-auth, Emocheck:user-auth (x86, x64) [all versions] -> normal
                // Emotet: admin-auth, Emocheck: admin-auth (x86) [all versions]   -> normal
                // Emotet: admin-auth, Emocheck: admin-auth (x64) [-2020/12]       -> normal
                // Emotet: admin-auth, Emocheck: admin-auth (x64) [2020/12-]       -> wow64 path redirection
                // Emotet: admin-auth, Emocheck: user-auth (x86, x64)              -> auth error

                if (contain(PathUnification(proc.cmd_line), PathUnification(susp.filepath).c_str())) {
                    is_emotet = true;
                } else if (susp.is_admin && proc.name == "rundll32.exe") {
                    if (proc.cmd_line == "") {
                        PrintAuthError(proc.PID);
                    } else {
                        // syswow64 path redirection
                        std::string redirect_path = PathUnification(susp.filepath);
                        redirect_path = ReplaceString(redirect_path, "syswow64", "system32");
                        if (contain(PathUnification(proc.cmd_line), redirect_path.c_str())) {
                            is_emotet = true;
                        }
                    }
                }
            }

            // push back process as Emotet Process.
            if (is_emotet && pids.find(proc.PID) == pids.end()) {
                pids.insert(proc.PID);
                emo_proc = new EmotetProcess();
                emo_proc->pid = proc.PID;
                emo_proc->process_name = proc.name;
                emo_proc->image_path = susp.filepath;
                emotet_procs.push_back(*emo_proc);
            }
        }
    }
    return emotet_procs;
}

std::tuple<int, std::vector<EmotetProcess>> ScanEmotet(bool debug) {
    std::vector<EmotetLoader> candidates;
    std::unordered_set<std::string> uniq_names;
    EmotetLoader susp_file;
    std::vector<EmotetLoader> susp_files;
    std::vector<EmotetProcess> emotet_procs;
    static const int NOT_INFECTED = 0;
    static const int INFECTED = 1;
    int is_infected;

    is_debug = debug;

    load_libs();

    // search suspicious files (- 2020/02/05)
    susp_file = GenerateEmotetV1ProcessName();
    susp_files.push_back(susp_file);

    // search suspicious files (2020/02/06 -)
    candidates = GetEmotetV2FileNameFromRegistry(GetVolumeSerialNumber());
    for (unsigned int i = 0; i < candidates.size(); i++) {
        if (candidates[i].filename.length() != 0 && uniq_names.find(candidates[i].filename) == uniq_names.end()) {
            susp_files.push_back(candidates[i]);
            uniq_names.insert(candidates[i].filename);
        }
    }

    // search suspicious files (2020/08/11 -)
    candidates = EmotetScannerV3();
    for (unsigned int i = 0; i < candidates.size(); i++) {
        if (candidates[i].filename.length() != 0) {
            susp_files.push_back(candidates[i]);
        }
    }

    // search suspicious files (2020/12/21 -)
    candidates = EmotetScannerV4();
    for (unsigned int i = 0; i < candidates.size(); i++) {
        if (candidates[i].filename.length() != 0) {
            susp_files.push_back(candidates[i]);
        }
    }

    // search suspicious files with generic rule (2020/12/21 -)
    candidates = EmotetScannerV5();
    for (unsigned int i = 0; i < candidates.size(); i++) {
        if (candidates[i].filename.length() != 0) {
            susp_files.push_back(candidates[i]);
        }
    }

    emotet_procs = ScanEmotetProcess(susp_files);

    if (emotet_procs.empty()) {
        is_infected = NOT_INFECTED;
    } else {
        is_infected = INFECTED;
    }

    free_libs();

    return std::forward_as_tuple(is_infected, emotet_procs);
}

}  // namespace emocheck
