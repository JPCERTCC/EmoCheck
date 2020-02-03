# EmoCheck

Windows OS 向け Emotet 検知ツール

## 使用方法

1. Releases からツールをダウンロード
2. 感染が疑われるホストでツールを実行
3. 出力されるレポートテキストを確認

## ダウンロード

以下のページからダウンロードできます。

 [Release](https://github.com/JPCERTCC/EmoCheck/releases)

なお、ファイルのハッシュ値は以下の通りです。

> emocheck_x86.exe  
>   MD5   : 9508DACDF443B422D159160E02043045  
>   SHA256: 3F9BEFD9287923A844FA7F38DEADFE2238380398E1F4F4C902A18CD4CCF1BFA0  

> emocheck_x64.exe  
>   MD5   : 9E1B8BE8402A51B8FEE0B590B4965060  
>   SHA256: C8CC438BF271DFAA110C58C748C54175823269DA7202EA19FC75EEEA359FAEB5  

## Emotetの検知方法

EmoCheck ではホストのプロセス名から Emotet のプロセスを検知します。

## レポート例

Emotetが検知された場合、以下のようなレポートが作成されます。

```txt
[Emocheck v0.0.1]
プログラム実行時刻: 2020-02-03 13:45:51
____________________________________________________

[結果]
Emotetを検知しました

[詳細]
     プロセス名    : khmerbid.exe
     プロセスID    : 6132
     イメージパス  : C:\Users\tani\AppData\Local\khmerbid.exe
____________________________________________________

イメージパスの実行ファイルを隔離/削除してください。
```

レポートは以下のパスに生成されます。

- [emocheck.exe を実行したフォルダー]\yyyymmddhhmmss_emocheck.txt

## スクリーンショット

<div align="left"><img src="./img/report_jp.png"></div>

## その他

### 動作確認環境

- Windows 10 1809 64bit 日本語版
- Windows 8.1 64bit 日本語版
- Windows 7 SP1 32bit 日本語版
- Windows 7 SP1 64bit 日本語版

### ビルド環境

- Windows 10 1809 64bit 日本語版
- Microsoft Visual Studio Community 2017
