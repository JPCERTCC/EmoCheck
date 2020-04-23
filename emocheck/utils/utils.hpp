#ifndef EMOCHECK_UTILS_UTILS_HPP_
#define EMOCHECK_UTILS_UTILS_HPP_

// standard module
#include <string>
#include <vector>

// windows modules
#include "windows.h"

namespace emocheck {

std::string WideCharToString(wchar_t *wide_char);
std::vector<unsigned char> IntToBytes(unsigned int integer);
std::string GetImageFileName(DWORD pid);
std::string ConvertDivecePath(wchar_t *imagepath);
boolean CheckPeExtension(std::string filename);
unsigned int GetVolumeSerialNumber();

}  // namespace emocheck

#endif  //EMOCHECK_UTILS_UTILS_HPP_
