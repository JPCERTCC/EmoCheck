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

__The following released files have code signing with Microsoft Authenticode.__

> emocheck_x86_signed.exe  
>   MD5   : 7b48be91855af1c1cee55c2b4aa6005d  
>   SHA256: 4c39ef11ade2e99eaefe37b6549e96108fd19f4152a55059a9e04a7dd13ad989  

> emocheck_x64_signed.exe  
>   MD5   : 4739c25603fab312ab89508920039806  
>   SHA256: fe07c8f02ff713d3e6cfa7f515b22da2b06cde8c6598639e1cb25c3a49ad9e86  

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

## Notes

### Tested environments

- Windows 10 1809 64bit Japanese Edition
- Windows 8.1 64bit Japanese Edition
- Windows 7 SP1 32bit Japanese Edition
- Windows 7 SP1 64bit Japanese Edition

### Build

- Windows 10 1809 64bit Japanese Edition
- Microsoft Visual Studio Community 2017
