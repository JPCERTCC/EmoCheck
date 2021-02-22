#include "scan.hpp"

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

EmotetLoader GenerateEmotetV1ProcessName() {
    uint32_t q;
    uint32_t seed;
    int keylen;
    int mod;
    std::string keywords;
    std::string keyword;
    EmotetLoader suspicious_file;

    seed = GetVolumeSerialNumber();

    keywords = "duck,mfidl,targets,ptr,khmer,purge,metrics,acc,inet,msra,symbol,driver,";
    keywords += "sidebar,restore,msg,volume,cards,shext,query,roam,etw,mexico,basic,url,";
    keywords += "createa,blb,pal,cors,send,devices,radio,bid,format,thrd,taskmgr,timeout,";
    keywords += "vmd,ctl,bta,shlp,avi,exce,dbt,pfx,rtp,edge,mult,clr,wmistr,ellipse,vol,";
    keywords += "cyan,ses,guid,wce,wmp,dvb,elem,channel,space,digital,pdeft,violet,thunk";

    keylen = int(keywords.length());

    // first round
    q = seed / keylen;
    mod = seed % keylen;
    keyword += SelectWordFromKeywords(keywords, mod, keylen);

    // second round
    seed = 0xFFFFFFFF - q;
    mod = seed % keylen;
    keyword += SelectWordFromKeywords(keywords, mod, keylen);
    suspicious_file.filename = keyword + ".exe";
    suspicious_file.version = 1;

    return suspicious_file;
}

}  // namespace emocheck