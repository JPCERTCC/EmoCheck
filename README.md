# EmoCheck

[![GitHub release](https://img.shields.io/github/release/jpcertcc/emocheck.svg)](https://github.com/jpcertcc/emocheck/releases)
[![Github All Releases](https://img.shields.io/github/downloads/jpcertcc/emocheck/total.svg)](http://www.somsubhra.com/github-release-stats/?username=jpcertcc&repository=emocheck)

Emotet detection tool for Windows OS.

## How to use

1. Download EmoCheck from the Releases page.
2. Run EmoCheck on the host.
3. Check the exported report.

## Download

Please download from the [Releases](https://github.com/JPCERTCC/EmoCheck/releases) page.

Latest hash:  

> emocheck_v1.0_x86.exe
>   MD5   : 00f7612ff7e571115638206dd5b32f2c
>   SHA256: 544b6eb3bc04e9015ae7b4078e20a33a28c35370fb9e2c498d65e072c4c81bf3

> emocheck_v1.0_x64.exe
>   MD5   : 59ecb58b11c6df9324043fe4d2caf210
>   SHA256: cb80892758754d12af2148bcffc32eae0daa02c4815415b394aff2d79e0b761b

## Command options

(since v0.0.2)  

- Specify output directory for the report (default: current directory)
  - `/output [your output directory]` or `-output [your output directory]`
- No console output
  - `/quiet` or `-quiet`
- Export the report in JSON style
  - `/json` or `-json`
- Debug mode (no report)
  - `/debug` or `-debug`
- Show help
  - `/help` or `-help`

## How EmoCheck detects Emotet

(v0.0.1)  
Emotet generates their process name from a specific word dictionary and C drive serial number.
EmoCheck scans the running process on the host, and find Emotet process from their process name.

(added in v0.0.2)  
Emotet keeps their encoded process name in a specific registry key.
EmoCheck looks up and decode the registry value, and find it from the process list.
Code Signing with Microsoft Authenticode.

(added in v1.0)  
Support the April 2020 updated of Emotet.  
Obfuscated code.  

## Sample Report

Text stlye:  

```txt
[Emocheck v0.0.2]
Scan time: 2020-02-10 13:06:20
____________________________________________________

[Result]
Detected Emotet process.

[Emotet Process]
     Process Name  : mstask.exe
     Process ID    : 716
     Image Path    : C:\Users\[username]\AppData\Local\mstask.exe
____________________________________________________

Please remove or isolate the suspicious execution file.
```

JSON style (added in v0.0.2):  

```json
{
  "scan_time":"2020-02-10 13:06:20",
  "hostname":"[your hostname]",
  "emocheck_version":"0.0.2",
  "is_infected":"yes",
  "emotet_processes":[
    {
       "process_name":"mstask.exe",
       "process_id":"716",
       "image_path":"C:\\Users\\[username]\\AppData\\Local\\mstask.exe"
    }
  ]
}
```

The report will be exported to the following path.

(v0.0.1)  
`[current directory]\yyyymmddhhmmss_emocheck.txt`

(since v0.0.2)  
`[output path]\[computer name]_yyyymmddhhmmss_emocheck.txt`  
`[output path]\[computer name]_yyyymmddhhmmss_emocheck.json`

## Screenshot

(v0.0.1)  
<div align="left"><img src="./img/report_en.png"></div>

## Releases

- (Feb. 3, 2020) v0.0.1
  - Initial release
- (Feb. 10, 2020) v0.0.2
  - update detecting method
  - add options
- (Aug. 11, 2020) v1.0.0
  - update detecting method

## Notes

### Tested environments

- Windows 10 1809 64bit Japanese Edition
- Windows 8.1 64bit Japanese Edition
- ~~Windows 7 SP1 32bit Japanese Edition~~
- ~~Windows 7 SP1 64bit Japanese Edition~~

Windows 7 does not support UTF-8 output in the Command Prompt.


### Build

- Windows 10 1809 64bit Japanese Edition
- Microsoft Visual Studio Community 2017

### Source code

Not published from v1.0.
