#ifndef EMOCHECK_UTILS_REGISTRY_HPP_
#define EMOCHECK_UTILS_REGISTRY_HPP_

// standard modules
#include <algorithm>
#include <iostream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

// windows basic modules
#include <Windows.h>

// windows additional modules
#include <Psapi.h>

#ifdef DEBUG
#define DBG(X) std::cerr << "[Dbg] " << X << std::endl
#else
#define DBG(X)
#endif

namespace emocheck {

struct RegKeyValue {
    std::string name;
    std::string value;
};

RegKeyValue *LookupRegByName(HKEY root, std::string path, std::string name);
std::vector<RegKeyValue *> ListRegSZValues(HKEY root, std::string reg_path);
int test();
}  // namespace emocheck
#endif  // EMOCHECK_UTILS_REGISTRY_HPP_