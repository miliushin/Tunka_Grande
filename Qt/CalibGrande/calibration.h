#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <stdio.h>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <fstream>

#include <TFile.h>
#include <TGraph.h>
#include <TH1F.h>

#include <QTextBrowser>
#include <QProcess>
#include <QObject>

class Calibration
{

public:
    Calibration(QTextBrowser *TBrowser11, QTextBrowser *TBrowser22);
    ~Calibration();

    void SetTimeRun();
    const char* GetMainDir();

private:
    FILE *RunPath;
    FILE *FPATHDATA;

    char RUN_PATH[200];
    char STOPRUN[200];
    char PATH_DATA[200];      // data directory
    char find_main_dir[200];  // bash sript for find last data dir
    char dirname[200];        // name of the last directory

    QTextBrowser *TBrowser1;
    QTextBrowser *TBrowser2;

};

#endif // CALIBRATION_H
