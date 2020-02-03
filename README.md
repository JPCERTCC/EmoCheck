# EmoCheck

Emotet detection tool for Windows OS.

## How to use

1. Download EmoCheck from the Releases page.
2. Run EmoCheck on the host.
3. Check the exported report.

## Download

Please download from [Releases](https://github.com/JPCERTCC/EmoCheck/releases) page.

> emocheck_x86.exe  
>   MD5   : 9508DACDF443B422D159160E02043045  
>   SHA256: 3F9BEFD9287923A844FA7F38DEADFE2238380398E1F4F4C902A18CD4CCF1BFA0  

> emocheck_x64.exe  
>   MD5   : 9E1B8BE8402A51B8FEE0B590B4965060  
>   SHA256: C8CC438BF271DFAA110C58C748C54175823269DA7202EA19FC75EEEA359FAEB5  

## How EmoCheck detects Emotet

Emotet generates their process name from a specific word dictionary and C drive serial.
EmoCheck scans the running process on the host, and find Emotet process from their process name.

## Sample Report

```txt
[Emocheck v0.0.1]
Scan time: 2020-02-03 13:06:20
____________________________________________________

[Result] 
Detected Emotet process.

[Emotet Process] 
     Process Name  : khmerbid.exe
     Process ID    : 10508
     Image Path    : C:\Users\tani\AppData\Local\khmerbid.exe
____________________________________________________

Please remove or isolate the suspicious execution file.
```

The report will be exported to the following path.

- [path of emocheck.exe]\yyyymmddhhmmss_emocheck.txt

## Screenshot

<div align="left"><img src="./img/report_en.png"></div>

## Notes

### Tested environments

- Windows 10 1809 64bit Japanese Edition
- Windows 8.1 64bit Japanese Edition
- Windows 7 SP1 32bit Japanese Edition
- Windows 7 SP1 64bit Japanese Edition

### Build

- Windows 10 1809 64bit Japanese Edition
- Microsoft Visual Studio Community 2017
