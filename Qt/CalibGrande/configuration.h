#ifndef CONFIGURATION_H
#define CONFIGURATION_H


#include <stdio.h>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <fstream>

#include <QTextBrowser>

using namespace std;

class Config{
public:
    Config(QTextBrowser *TBrowser);
    ~Config();

    void SetHV(int ist, int idet, int vol);
    void SetThreshold(int ist, int ich, int thr);
    void SaveHV(int ist, int idet, int vol);
    void SetDetStatus(int ist, int idet, int status);

    void GetST();      // getting number of active stations for analisis
    void ReadCNF(const char *CNF, int NumOfSt);
    void WriteCNF(const char *CNF, int NumOfSt);
    void WriteOutCNF(const char *CNF, int NumOfSt);
    void WriteAllCNF();
    void WriteOutAllCNF();

    void ReadINT();

    void ThresholdSetting();  // reading AMP/NumberOfStation.txt

    int AnalysisHVcounts(int ist, int idet);

    int GetHV(int ist, int idet);
    int GetSaveHV(int ist, int idet);
    int GetThreshold(int ist, int ich);
    int GetDetStatus(int ist, int idet);
    int GetHVcount(int ist, int ich);
    int GetAmpHVcount(int ist, int ich);
    int GetNumOfSt();


private:

    int *StStatus;
    int **DetStatus;
    int **Threshold;
    int **HV;
    int **CV_HV;    // calibrated HV value
    int **HVcount;
    int **AmpHVcount;
    int NumOfSt;
    int **NumOfAttempts;

    char **NameCNF;
    char **TMPStr;

    int *NumOfStrCNF;
    int *FADC;
    QTextBrowser *TBrowser1;

};


#endif // CONFIGURATION_H
