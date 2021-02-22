#ifndef EMOCHECK_UTILS_FILE_HPP_
#define EMOCHECK_UTILS_FILE_HPP_

// standard modules
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

// windows basic modules
#include <Windows.h>

#include "winternl.h"

// windows additional modules
#include <Psapi.h>

namespace emocheck {
// functions
std::string ConvertDivecePath(wchar_t *imagepath);
boolean CheckPeExtension(std::string filename);
unsigned int GetVolumeSerialNumber();
long long int AcquireFileSize(const char *fileName);
std::string EscapeBackSlash(std::string s);
bool ComparePath(std::string path_a, std::string path_b);
boolean is_pefile(std::string path);
long long int TakeDiffFileTime(FILETIME *ft1, FILETIME *ft2);

}  // namespace emocheck
#endif  // EMOCHECK_UTILS_FILE_HPP_