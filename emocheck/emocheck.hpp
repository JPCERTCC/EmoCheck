#ifndef EMOCHECK_EMOCHECK_HPP_
#define EMOCHECK_EMOCHECK_HPP_

// standard modules
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <tuple>
#include <unordered_set>
#include <vector>

// windows basic modules
#include <Windows.h>

#include "utils/service.hpp"
#include "utils/utils.hpp"
#include "winternl.h"

// windows additional modules
#include <Psapi.h>
#include <TlHelp32.h>

// Macro for debug message.
#ifdef DEBUG
#define DBG(X) std::cerr << "[Dbg] " << X << std::endl
#else
#define DBG(X)
#endif

namespace emocheck {

static const char* EMOCHECK_VERSION = "2.0";
static const char* EMOCHECK_RELEASE_DATE = "2021/1/27";
static const char* EMOCHECK_URL = "https://github.com/JPCERTCC/EmoCheck";
static const char* LINE_DELIMITER =
    "____________________________________________________\n";

const unsigned short int LANG_ID_JP = 0x0411;
const unsigned short int LANG_ID_FR = 0x040c;

struct EmotetLoader {
    unsigned short int version;  // version of scanner
    std::string filename;
    std::string filepath;
    std::string run_key;
    std::string cmd_line;     // registerd command line in the run key
    boolean file_validation;  // used by v3 scanner
    boolean is_admin;
    WinService srv;
    Rundll32Cmd rundll32;  // store rundll32 commandline.
};

struct EmotetProcess {
    std::string process_name;
    int pid;
    std::string image_path;
    std::string run_key;
    std::string cmd_line;
    WinService srv;
};

std::tuple<int, std::vector<EmotetProcess>> ScanEmotet(bool);

}  //namespace emocheck

#endif  //EMOCHECK_EMOCHECK_HPP_