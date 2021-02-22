#ifndef EMOCHECK_UTILS_UTILS_HPP_
#define EMOCHECK_UTILS_UTILS_HPP_

// standard modules
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

// windows basic modules
#include <Windows.h>

// windows additional modules
#include <Psapi.h>
#include <Shlobj.h>

namespace emocheck {

struct Rundll32Cmd {
    std::string bin_path;
    std::string dll;
    std::string export_func;
};

std::string WideCharToString(wchar_t *wide_char);
const wchar_t *StringToWideChar(std::string str);
std::vector<unsigned char> IntToBytes(unsigned int integer);
bool contain(std::string s, const char *v);
std::vector<std::string> split(std::string str, std::string sep);
std::string ReplaceString(std::string str, const char *before, const char *after, int cnt = 1);

bool Is64bit();
bool IsWindows7();
void load_libs();
void free_libs();
std::string GetSysDirX86();
boolean IsWinOSx64();
std::string PathUnification(std::string path);
Rundll32Cmd *ParseRundll32CmdLine(std::string cmd_line);

}  // namespace emocheck

#endif  //EMOCHECK_UTILS_UTILS_HPP_
