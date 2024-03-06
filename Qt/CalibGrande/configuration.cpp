#include "configuration.h"
#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <cstring>

#include <QMessageBox>



Config::Config(QTextBrowser *TBrowser){
    NumOfSt = 0;
    TBrowser1 = TBrowser;

    TMPStr = new char*[50];
    for (int i = 0; i < 50; i++){
        TMPStr[i] = new char[200];
        memset(TMPStr[i], '\0', 200);
    }
    FADC = new int[32];
    NumOfStrCNF = new int[NUMBER_OF_STATIONS];
    DetStatus = new int*[NUMBER_OF_STATIONS];
    Threshold = new int*[NUMBER_OF_STATIONS];
    StStatus = new int[NUMBER_OF_STATIONS];
    HV = new int*[NUMBER_OF_STATIONS];
    CV_HV = new int*[NUMBER_OF_STATIONS];
    HVcount = new int*[NUMBER_OF_STATIONS];
    AmpHVcount = new int*[NUMBER_OF_STATIONS];
    NumOfAttempts = new int*[NUMBER_OF_STATIONS];


    NameCNF = new char*[NUMBER_OF_STATIONS];
    for (int i = 0; i < NUMBER_OF_STATIONS; i++){
        NumOfStrCNF[i] = 0;
        StStatus[i] = 0;
        NameCNF[i] = new char[200];
        memset(NameCNF[i],'\0', 200);
        int iSt = i + 31;
        snprintf(NameCNF[i], 200, "MUON_%d.CNF",iSt);
        DetStatus[i] = new int[NUMBER_OF_DET];
        Threshold[i] = new int[NUMBER_OF_CHANNELS];
        HVcount[i] = new int[NUMBER_OF_CHANNELS];
        AmpHVcount[i] = new int[NUMBER_OF_CHANNELS];
        HV[i] = new int[NUMBER_OF_DET];
        CV_HV[i] = new int[NUMBER_OF_DET];
        NumOfAttempts[i] = new int[NUMBER_OF_DET];
        for (int j = 0; j < NUMBER_OF_DET; j++){
            DetStatus[i][j] = 0;
            HV[i][j] = 0.;
            CV_HV[i][j] = 0.;
            NumOfAttempts[i][j] = 0;
        }
        for (int j = 0; j < NUMBER_OF_CHANNELS; j++){
            Threshold[i][j] = 5;
            HVcount[i][j] = 0;
            AmpHVcount[i][j] = 0;
        }
    }

    GetST();

    for (int i = 0; i < NUMBER_OF_STATIONS; i++){
        if (StStatus[i] == 1){
            ReadCNF(NameCNF[i],i);
            for (int ich = 0; ich < 8; ich++){
                SetThreshold(i,ich,MIN_THCODE);    // initial thresholds
            }
            WriteCNF(NameCNF[i],i);
        }
    }
}

Config::~Config(){

    for (int i = 0; i < NUMBER_OF_STATIONS; i++){
        delete[] DetStatus[i];
        delete[] Threshold[i];
        delete[] HV[i];
        delete[] CV_HV[i];
        delete[] NumOfAttempts[i];
    }
    delete[] DetStatus;
    delete[] Threshold;
    delete[] HV;
    delete[] CV_HV;
    delete[] NumOfAttempts;
    delete[] StStatus;

    for (int i = 0; i < 50; i++){
        delete[] TMPStr[i];
    }
    delete[] TMPStr;
    delete[] FADC;
}

void Config::GetST(){
    FILE *FPATH;
    char ST_CNF[200];
    char RUN_PATH[200];
    char STOPRUN[200];
    char ctmp[200];

    memset(ST_CNF, '\0', 200);

    FPATH = fopen("path.sh","r");
    if (FPATH == NULL) {
        TBrowser1->insertPlainText("path.sh is not opened \n");
        QMessageBox msgBox;
        msgBox.critical(nullptr, "Error", "ERROR:\n path.sh is not opened !");
        exit(1);
    }

    memset(RUN_PATH, '\0', 200);
    memset(STOPRUN, '\0', 200);
    memset(ctmp, '\0', 200);


    fscanf(FPATH, "%s", RUN_PATH);
    fscanf(FPATH, "%s", STOPRUN);
    fscanf(FPATH, "%s", ctmp);
    snprintf(ST_CNF, 200, "%s/%s",RUN_PATH,ctmp);
    fclose(FPATH);

    string num_of_st;
    ifstream FST_CNF(ST_CNF);

    // ----------------  read
    if (!FST_CNF) {
        char textout[100];
        memset(textout,'\0',100);
        snprintf(textout, 100,"ERROR:\n ST_CNF: %s is not opened\n",ST_CNF);
        QMessageBox msgBox;
        msgBox.critical(0, "Error", textout);
        exit(1);
    }
    else {
        while (getline(FST_CNF, num_of_st)) {
            int Nst = stoi(num_of_st);
            if (Nst >= 31 || Nst < 50 ){
                StStatus[Nst-31] = 1;
                NumOfSt++;
            }
        }
    }
}

void Config::SetHV(int ist, int ich, int vol){
    if (ist > NUMBER_OF_STATIONS || ich > NUMBER_OF_DET){
        printf("ERROR: ist = %d, ich = %d\n",ist,ich);
        exit(1);
    }
    HV[ist][ich] = vol;
}

void Config::SaveHV(int ist, int ich, int vol){
    if (ist > NUMBER_OF_STATIONS || ich > NUMBER_OF_DET){
        printf("ERROR: ist = %d, ich = %d\n",ist,ich);
        exit(1);
    }
    CV_HV[ist][ich] = vol;
}

void Config::SetDetStatus(int ist, int idet, int vol){
    if (ist > NUMBER_OF_STATIONS || idet > NUMBER_OF_DET){
        printf("ERROR: ist = %d, idet = %d\n",ist,idet);
        exit(1);
    }
    DetStatus[ist][idet] = vol;
}

void Config::SetThreshold(int ist, int ich, int thr){
    if (ist > NUMBER_OF_STATIONS || ich > NUMBER_OF_CHANNELS){
        char str[100];
        memset(str, '\0', 100);
        snprintf(str, 100, "ERROR: ist = %d, ich = %d\n",ist,ich);
        QMessageBox msgBox;
        msgBox.critical(nullptr, "Error", str);
        exit(1);
    }
    Threshold[ist][ich] = thr;
}

int Config::GetHV(int ist, int idet){
    if (ist > NUMBER_OF_STATIONS || idet > NUMBER_OF_DET){
        char str[100];
        memset(str, '\0', 100);
        snprintf(str, 100, "ERROR: ist = %d, ich = %d\n",ist,idet);
        QMessageBox msgBox;
        msgBox.critical(nullptr, "Error", str);
        exit(1);
    }
    return HV[ist][idet];
}

int Config::GetHVcount(int ist, int ich){
    if (ist > NUMBER_OF_STATIONS || ich > NUMBER_OF_CHANNELS){
        char str[100];
        memset(str, '\0', 100);
        snprintf(str, 100, "ERROR: ist = %d, ich = %d\n",ist,ich);
        QMessageBox msgBox;
        msgBox.critical(nullptr, "Error", str);
        exit(1);
    }
    return HVcount[ist][ich];
}

int Config::GetAmpHVcount(int ist, int ich){
    if (ist > NUMBER_OF_STATIONS || ich > NUMBER_OF_CHANNELS){
        char str[100];
        memset(str, '\0', 100);
        snprintf(str, 100, "ERROR: ist = %d, ich = %d\n",ist,ich);
        QMessageBox msgBox;
        msgBox.critical(nullptr, "Error", str);
        exit(1);
    }
    return AmpHVcount[ist][ich];
}

int Config::GetSaveHV(int ist, int idet){
    if (ist > NUMBER_OF_STATIONS || idet > NUMBER_OF_DET){
        char str[100];
        memset(str, '\0', 100);
        snprintf(str, 100, "ERROR: ist = %d, idet = %d\n",ist,idet);
        QMessageBox msgBox;
        msgBox.critical(nullptr, "Error", str);
        exit(1);
    }
    return CV_HV[ist][idet];
}

int Config::GetDetStatus(int ist, int idet){
    if (ist > NUMBER_OF_STATIONS || idet > NUMBER_OF_DET){
        char str[100];
        memset(str, '\0', 100);
        snprintf(str, 100, "ERROR: ist = %d, idet = %d\n",ist,idet);
        QMessageBox msgBox;
        msgBox.critical(nullptr, "Error", str);
        exit(1);
    }
    return DetStatus[ist][idet];
}
int Config::GetThreshold(int ist, int ich){
    if (ist > NUMBER_OF_STATIONS || ich > NUMBER_OF_CHANNELS){
        char str[100];
        memset(str, '\0', 100);
        snprintf(str, 100, "ERROR: ist = %d, ich = %d\n",ist,ich);
        QMessageBox msgBox;
        msgBox.critical(nullptr, "Error", str);
        exit(1);
    }
    return Threshold[ist][ich];
}

void Config::ReadCNF(const char *CNF, int NumOfSt){
    NumOfStrCNF[NumOfSt] = 0;
    ifstream FileCNF(CNF);
    string StrCNF;

    if (!FileCNF) {
        char textout[100];
        memset(textout,'\0',100);
        snprintf(textout, 100,"ERROR:\n ST_CNF: %s is not opened\n",CNF);
        QMessageBox msgBox;
        msgBox.critical(0, "Error", textout);
        exit(1);
    }
    else {
        int istr = 0;
        while (getline(FileCNF, StrCNF)) {
            NumOfStrCNF[NumOfSt]++;
            for(int i = 0; i < 200; i++){
                if (StrCNF[i]!='\0'){
                    TMPStr[istr][i] = StrCNF[i];
                }
                else
                    break;
            }

            istr++;
            if (istr >= 50){
                QMessageBox msgBox;
                msgBox.critical(0, "Error", "ERROR:\n number of string more 50 in MUON_.CNF\n");
                exit(1);
            }

        }
    }

    FileCNF.close();
}

void Config::WriteCNF(const char *CNF, int NumOfSt){
    bool FlagCode = false;
    bool FlagHV = false;
    bool FlagStop = false;
    int nstr = 0;
    char cStr[10];
    int iFADS = 0;

    FILE *FileCNF = fopen(CNF,"w");

    if (FileCNF==NULL){
        char textout[100];
        memset(textout,'\0',100);
        snprintf(textout, 100,"ERROR:\n %s is not opened\n",CNF);
        QMessageBox msgBox;
        msgBox.critical(0, "Error", textout);
        exit(1);
    }

    for (int i = 0; i < NumOfStrCNF[NumOfSt]; i++){

        if (TMPStr[i][0]=='#' && TMPStr[i][2]=='o'){
            FlagCode = true;
            fprintf(FileCNF,"%s\n",TMPStr[i]);
            continue;
        }

        // write threshold
        if (FlagCode == true && nstr == 8){
            FlagCode = false;
            for (int ii = 0; ii < nstr; ii++){
                fprintf(FileCNF, "%4d %12d %6d %6d\n",
                        FADC[ii*4],GetThreshold(NumOfSt, ii),FADC[ii*4+2],FADC[ii*4+3]);
            }
        }

        // write HV
        if (TMPStr[i][0]=='#' && TMPStr[i][2]=='N'){
            FlagHV = true;
            fprintf(FileCNF,"%s\n",TMPStr[i]);
            continue;
        }
        if (FlagHV == true){
            FlagHV = false;
            FlagStop = true;
            for (int ii = 0; ii < 20; ii++){
                int iDet = ii + 1;
                fprintf(FileCNF, "%4d %7d\n", iDet, GetHV(NumOfSt, ii));
            }
        }

        if (FlagCode == true){
            int j = 0;
            bool FL = false;
            for (int k = 0; k < 800; k++ ){
                if (j == 0 && FL == true){
                    FL = false;
                }
                if (TMPStr[i][k]!=' ' && FL==false){
                    cStr[j] = TMPStr[i][k];
                    j++;
                    if (TMPStr[i][k+1]==' ' || TMPStr[i][k+1]=='\0'){
                        FADC[iFADS] = atoi(cStr);
                        memset(cStr, '\0',10);
                        FL = true;
                        j = 0;
                        iFADS++;
                        if (iFADS > 32){
                            QMessageBox msgBox;
                            msgBox.setText("ERROR:\n iFADC > 32\n");
                            msgBox.exec();
                            exit(1);
                        }
                    }
                    if (TMPStr[i][k+1]=='\0')
                        break;
                }
            }
            nstr++;
        }
        else if (FlagHV==false && FlagStop==false){
            fprintf(FileCNF,"%s\n",TMPStr[i]);
        }

    }

    fclose(FileCNF);
}

void Config::WriteOutCNF(const char *CNF, int NumOfSt){
    bool FlagCode = false;
    bool FlagHV = false;
    bool FlagStop = false;
    int nstr = 0;
    char cStr[10];
    int iFADS = 0;

    FILE *FileCNF = fopen(CNF,"w");

    if (FileCNF==NULL){
        char textout[100];
        memset(textout,'\0',100);
        snprintf(textout, 100,"ERROR:\n %s is not opened\n",CNF);
        QMessageBox msgBox;
        msgBox.critical(0, "Error", textout);
        exit(1);
    }

    for (int i = 0; i < NumOfStrCNF[NumOfSt]; i++){

        if (TMPStr[i][0]=='#' && TMPStr[i][2]=='o'){
            FlagCode = true;
            fprintf(FileCNF,"%s\n",TMPStr[i]);
            continue;
        }

        // write threshold
        if (FlagCode == true && nstr == 8){
            FlagCode = false;
            for (int ii = 0; ii < nstr; ii++){
                fprintf(FileCNF, "%4d %12d %6d %6d\n",
                        FADC[ii*4],GetThreshold(NumOfSt, ii),FADC[ii*4+2],FADC[ii*4+3]);
            }
        }

        // write HV
        if (TMPStr[i][0]=='#' && TMPStr[i][2]=='N'){
            FlagHV = true;
            fprintf(FileCNF,"%s\n",TMPStr[i]);
            continue;
        }
        if (FlagHV == true){
            FlagHV = false;
            FlagStop = true;
            for (int ii = 0; ii < 20; ii++){
                int iDet = ii + 1;
                fprintf(FileCNF, "%4d %7d\n", iDet, CV_HV[NumOfSt][ii]);
            }
        }

        if (FlagCode == true){
            int j = 0;
            bool FL = false;
            for (int k = 0; k < 800; k++ ){
                if (j == 0 && FL == true){
                    FL = false;
                }
                if (TMPStr[i][k]!=' ' && FL==false){
                    cStr[j] = TMPStr[i][k];
                    j++;
                    if (TMPStr[i][k+1]==' ' || TMPStr[i][k+1]=='\0'){
                        FADC[iFADS] = atoi(cStr);
                        memset(cStr, '\0',10);
                        FL = true;
                        j = 0;
                        iFADS++;
                        if (iFADS > 32){
                            //printf("ERROR: iFADC > 32\n");
                            //exit(1);
                            QMessageBox msgBox;
                            msgBox.setText("ERROR:\n iFADC > 32\n");
                            msgBox.exec();
                            exit(1);
                        }
                    }
                    if (TMPStr[i][k+1]=='\0')
                        break;
                }
            }
            nstr++;
        }
        else if (FlagHV==false && FlagStop==false){
            fprintf(FileCNF,"%s\n",TMPStr[i]);
        }

    }

    fclose(FileCNF);
}

void Config::ThresholdSetting(){
    // reading threshilds
    for (int i = 0; i < NUMBER_OF_STATIONS; i++) {
      int namp[11];
      int i1, i2;
      int nst = i + 31;
      char file_amp[20];
      memset(file_amp, '\0', 20);
      snprintf(file_amp, 20, "AMP/%d.txt", nst);
      FILE *F_AMP = fopen(file_amp,"r");
      while(fscanf(F_AMP, "%d %d", &i1, &i2) == 2){
          namp[i1] = i2;
      }
      SetThreshold(i,0,namp[1]);
      SetThreshold(i,1,namp[3]);
      SetThreshold(i,2,namp[4]);
      SetThreshold(i,3,namp[6]);
      SetThreshold(i,4,namp[8]);
      SetThreshold(i,5,namp[10]);
      SetThreshold(i,6,2000);
      SetThreshold(i,7,2000);

      fclose(F_AMP);

      WriteCNF(NameCNF[i],i);

    }
    TBrowser1->insertPlainText("Пороги установлены\n");
}

void Config::ReadINT(){
    // reading INT/
    for (int i = 0; i < NUMBER_OF_STATIONS; i++) {

      int i1;
      double d1, d2;
      int nst = i + 31;
      char file_int[20];
      memset(file_int, '\0', 20);
      snprintf(file_int, 20, "INT/%d.txt", nst);
      FILE *F_INT;
      if((F_INT = fopen(file_int,"r")) == NULL){
          printf("Cannot open file: %s\n",file_int);
          exit(1);
      }
     while(fscanf(F_INT, "%d %lf %lf", &i1, &d1, &d2)==3){
          HVcount[i][i1] = (int)d1;
          AmpHVcount[i][i1] = (int)d2;
      }
      fclose (F_INT);
    }
}

void Config::WriteAllCNF(){
    for (int i = 0; i < NUMBER_OF_STATIONS; i++) {
      WriteCNF(NameCNF[i],i);
    }
}
void Config::WriteOutAllCNF(){
    for (int i = 0; i < NUMBER_OF_STATIONS; i++) {
      char iname[20];
      memset(iname,'\0',20);
      snprintf(iname, 20, "OUT/%s",NameCNF[i]);
      WriteOutCNF(iname,i);
    }
}

int Config::AnalysisHVcounts(int ist, int idet){
    NumOfAttempts[ist][idet]++;
    int FlagDet = 0;
    int deltaHV;
    int det_status = GetDetStatus(ist,idet);
    if (det_status == 1){
        if (idet <= 5){
            deltaHV =  HVcount[ist][0] - HV_INSTALL;
        }
        if (idet > 5 && idet <= 11){
            deltaHV =  HVcount[ist][2] - HV_INSTALL;
        }
        if (idet == 12 || idet == 13){
            deltaHV =  HVcount[ist][4] - HV_INSTALL;
        }
        if (idet == 14 || idet == 15){
            deltaHV =  HVcount[ist][6] - HV_INSTALL;
        }
        if (idet == 16 || idet == 17){
            deltaHV =  HVcount[ist][8] - HV_INSTALL;
        }
        if (idet == 18 || idet == 19){
            deltaHV =  HVcount[ist][10] - HV_INSTALL;
        }

        if ((deltaHV <= 50 && deltaHV >= -50) || NumOfAttempts[ist][idet] == NUM_OF_ATTEM) {
            FlagDet = 1;
            if (NumOfAttempts[ist][idet] == NUM_OF_ATTEM){
                CV_HV[ist][idet] = 0;
            }
            else {
                CV_HV[ist][idet] = HV[ist][idet];
            }
            HV[ist][idet] = 0;
            DetStatus[ist][idet] = 0;
            if ( (idet < 5) || (idet > 5 && idet < 11) ){
               DetStatus[ist][idet+1] = 1;
               HV[ist][idet+1] = HV_NOMINAL;
            }
            if (idet == 5){
               DetStatus[ist][12] = 1;
               HV[ist][12] = HV_NOMINAL;

            }
            if (idet == 11){
                DetStatus[ist][13] = 1;
                HV[ist][13] = HV_NOMINAL;

            }

            if (idet == 12 || idet == 13 || idet == 14 || idet == 15 ||
                    idet == 16 || idet == 17){
                DetStatus[ist][idet+2] = 1;
                HV[ist][idet+2] = HV_NOMINAL;

            }
            if (idet == 19){
                // the end of detector calibration
            }
        }
        if (deltaHV > 50 && AmpHVcount[ist][idet] > 20) {
            if (HV[ist][idet] == 3800){
                // voltage limit

            }
            double constA = (double)HVcount[ist][idet] - COEF_HV*(double)HV[ist][idet];
            HV[ist][idet] = (int)((HV_INSTALL - constA) / COEF_HV);
            if (HV[ist][idet] > 3800){
                HV[ist][idet] = 3800;
            }
        }
        else if (deltaHV > 50 && AmpHVcount[ist][idet] <= 20){
            HV[ist][idet] = HV[ist][idet] + 300;
            if (HV[ist][idet] > 3800){
                HV[ist][idet] = 3800;
            }
        }
        else if (deltaHV < -50 ){
            HV[ist][idet] = HV[ist][idet] + 300;
            if (HV[ist][idet] > 3800){
                HV[ist][idet] = 3800;
            }
        }
        WriteAllCNF();
    }
    return FlagDet;
}

int Config::GetNumOfSt(){
    return NumOfSt;
}
