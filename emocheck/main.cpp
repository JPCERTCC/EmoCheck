/**
 * @file   main.cpp
 */

// emocheck module
#include "emocheck.h"

// standard modules
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

// windows basic module
#include <windows.h>

namespace emocheck
{

void PrintBanner()
{
    char banner[] =
        "  ______                  _____ _               _   \n"
        "|  ____|                / ____| |             | |   \n"
        "| |__   _ __ ___   ___ | |    | |__   ___  ___| | __\n"
        "|  __| | '_ ` _ ` / _ `| |    | '_ ` / _ `/ __| |/ /\n"
        "| |____| | | | | | (_) | |____| | | |  __/ (__|   < \n"
        "|______|_| |_| |_|`___/ `_____|_| |_|`___|`___|_|`_`\n";

    std::cout << LINE_DELIMITER
              << banner
              << LINE_DELIMITER << "\n"
              << "Emotet detection tool by JPCERT/CC.\n\n"
              << "Version      : " << EMOCHECK_VERSION << "\n"
              << "Release Date : " << EMOCHECK_RELEASE_DATE << "\n"
              << "URL          : " << EMOCHECK_URL << "\n"
              << LINE_DELIMITER << std::endl;

    // unsigned short int usrDefLangId = GetUserDefaultLangID();
}

void PrintReport(std::vector<EmotetProcess> emotet_processes)
{
    if (GetUserDefaultLangID() == LANG_ID_JP)
    {
        // Japanese Report
        if (emotet_processes.size() > 0)
        {
            std::cout.imbue(std::locale(""));
            for (int i = 0; i < emotet_processes.size(); ++i)
            {
                std::cout << "[!!] Emotet 検知"
                          << "\n"
                          << "     プロセス名    : " << emotet_processes[i].process_name.c_str() << "\n"
                          << "     プロセスID    : " << emotet_processes[i].pid << "\n"
                          << "     イメージパス  : " << emotet_processes[i].image_path.c_str() << std::endl;
            }
            std::cout << LINE_DELIMITER << std::endl;
            std::cout << "Emotetのプロセスが見つかりました。\n"
                      << "イメージパスの実行ファイルを隔離/削除してください。\n"
                      << std::endl;
        }
        else
        {
            std::cout << "Emotetは検知されませんでした。\n"
                      << std::endl;
        }
    }
    else
    {
        // English Report
        if (emotet_processes.size() > 0)
        {
            for (int i = 0; i < emotet_processes.size(); ++i)
            {
                std::cout << "[!!] Detected"
                          << "\n"
                          << "     Process Name: " << emotet_processes[i].process_name.c_str() << "\n"
                          << "     PID         : " << emotet_processes[i].pid << "\n"
                          << "     Image Path  : " << emotet_processes[i].image_path.c_str() << std::endl;
            }
            std::cout << LINE_DELIMITER << std::endl;
            std::cout << "Emotet had be detected.\n"
                      << "Please remove or isolate the suspicious execution file.\n"
                      << std::endl;
        }
        else
        {
            std::cout << "No detection.\n"
                      << std::endl;
        }
    }
    return;
}

void WriteReport(std::vector<EmotetProcess> emotet_processes)
{
    char filename[28];
    char time_iso8601[20];

    time_t t = time(nullptr);
    struct tm local_time;

    localtime_s(&local_time, &t);
    std::strftime(time_iso8601, 20, "%Y-%m-%d %H:%M:%S", &local_time);
    std::strftime(filename, 28, "%Y%m%d%H%M%S_emocheck.txt", &local_time);

    std::ofstream outputfile(filename);

    if (GetUserDefaultLangID() == LANG_ID_JP)
    {
        // Japanese Report
        outputfile << "[Emocheck v" << EMOCHECK_VERSION << "]" << std::endl;
        outputfile << "プログラム実行時刻: " << time_iso8601 << std::endl;
        outputfile << LINE_DELIMITER << std::endl;
        if (emotet_processes.size() > 0)
        {
            outputfile << "[結果]\n"
                       << "Emotetを検知しました。\n"
                       << std::endl;
            for (int i = 0; i < emotet_processes.size(); ++i)
            {
                outputfile << "[詳細]\n"
                           << "     プロセス名    : " << emotet_processes[i].process_name.c_str() << "\n"
                           << "     プロセスID    : " << emotet_processes[i].pid << "\n"
                           << "     イメージパス  : " << emotet_processes[i].image_path.c_str() << std::endl;
            }
            outputfile << LINE_DELIMITER << std::endl;
            outputfile << "イメージパスの実行ファイルを隔離/削除してください。" << std::endl;
        }
        else
        {
            outputfile << "[結果]\n"
                       << "検知しませんでした。" << std::endl;
        }
        outputfile.close();
        std::cout.imbue(std::locale(""));
        std::cout << "以下のファイルに結果を出力しました。" << std::endl;
        std::cout << "\n\t" << filename << "\n"
                  << std::endl;
        std::cout << "ツールのご利用ありがとうございました。\n"
                  << std::endl;
    }
    else
    {
        // English Report
        outputfile << "[Emocheck v" << EMOCHECK_VERSION << "]" << std::endl;
        outputfile << "Scan time: " << time_iso8601 << std::endl;
        outputfile << LINE_DELIMITER << std::endl;
        if (emotet_processes.size() > 0)
        {
            outputfile << "[Result] \nDetected Emotet process.\n"
                       << std::endl;
            for (int i = 0; i < emotet_processes.size(); ++i)
            {
                outputfile << "[Emotet Process] \n"
                           << "     Process Name  : " << emotet_processes[i].process_name.c_str() << "\n"
                           << "     Process ID    : " << emotet_processes[i].pid << "\n"
                           << "     Image Path    : " << emotet_processes[i].image_path.c_str() << std::endl;
            }
            outputfile << LINE_DELIMITER << std::endl;
            outputfile << "Please remove or isolate the suspicious execution file." << std::endl;
        }
        else
        {
            outputfile << "[Result] \nNo detection." << std::endl;
        }
        outputfile.close();
        std::cout << "Report has exported to following file." << std::endl;
        std::cout << "\n\t" << filename << "\n"
                  << std::endl;
        std::cout << "Thank you for using our tool.\n"
                  << std::endl;
    }
    return;
}

} // namespace emocheck

int main()
{
    std::vector<emocheck::EmotetProcess> scan_result;

    emocheck::PrintBanner();
    scan_result = emocheck::ScanEmotet();
    emocheck::PrintReport(scan_result);
    emocheck::WriteReport(scan_result);

    system("pause");

    return 0;
}
