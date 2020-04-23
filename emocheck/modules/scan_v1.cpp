/*
 * LICENSE
 * Please refer to the LICENSE.txt at https://github.com/JPCERTCC/EmoCheck/
 */

#include "scan_v1.hpp"

#include "../utils/utils.hpp"

// standard modules
#include <string>

namespace emocheck {

std::string SelectWordFromKeywords(std::string keywords, int ptr, int keylen) {
    std::string keyword;

    for (int i = ptr; i > 0; i--) {
        if (keywords[i] != ',') {
            continue;
        } else {
            ptr = i;
            break;
        }
    }
    if (keywords[ptr] == ',') {
        ptr++;
    }
    for (int i = ptr; i < keylen; i++) {
        if (keywords[i] != ',') {
            keyword += keywords[i];
            ptr++;
        } else {
            break;
        }
    }
    return keyword;
}

std::string GenerateEmotetProcessName() {
    uint32_t q;
    uint32_t seed;
    int keylen;
    int mod;
    std::string keywords;
    std::string keyword;

    seed = GetVolumeSerialNumber();

    keywords =
        "duck,mfidl,targets,ptr,khmer,purge,metrics,acc,inet,msra,symbol,driver,"
        "sidebar,restore,msg,volume,cards,shext,query,roam,etw,mexico,basic,url,"
        "createa,blb,pal,cors,send,devices,radio,bid,format,thrd,taskmgr,timeout,"
        "vmd,ctl,bta,shlp,avi,exce,dbt,pfx,rtp,edge,mult,clr,wmistr,ellipse,vol,"
        "cyan,ses,guid,wce,wmp,dvb,elem,channel,space,digital,pdeft,violet,thunk";

    keylen = int(keywords.length());

    // first round
    q = seed / keylen;
    mod = seed % keylen;
    keyword += SelectWordFromKeywords(keywords, mod, keylen);

    // second round
    seed = 0xFFFFFFFF - q;
    mod = seed % keylen;
    keyword += SelectWordFromKeywords(keywords, mod, keylen);

    return keyword;
}

}  // namespace emocheck