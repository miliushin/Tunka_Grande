#include "calibration.h"
#include "runprog.h"
#include "global.h"


#include <QMessageBox>
#include <QTextStream>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sysexits.h>
#include <unistd.h>
#include <dirent.h>

using namespace std;


Calibration::Calibration(QTextBrowser *TBrowser11, QTextBrowser *TBrowser22)
{
    TBrowser1 = TBrowser11;
    TBrowser2 = TBrowser22;

    RunPath = fopen("path.sh","r");
    if (RunPath == NULL) {
        QMessageBox msgBox;
        msgBox.critical(nullptr, "Error", "ERROR:\n path.sh is not opened !");
        exit(1);
    }

    memset(RUN_PATH, '\0', 200);
    memset(STOPRUN, '\0', 200);
    fscanf(RunPath, "%s", RUN_PATH);
    fscanf(RunPath, "%s", STOPRUN);
    fclose(RunPath);

    FPATHDATA = fopen("PATHFILES.CNF","r");
    if (FPATHDATA == NULL) {
        QMessageBox msgBox;
        msgBox.critical(nullptr, "Error", "ERROR:\n PATHFILES.CNF is not opened !");
        exit(1);
    }

    memset(PATH_DATA, '\0', 200);
    memset(dirname,'\0', 200);

    fscanf(FPATHDATA, "%s", PATH_DATA);
    fclose(FPATHDATA);

}

Calibration::~Calibration(){

}


void Calibration::SetTimeRun(){
    char st1[200], st2[200], st3[200];
    int ch,min,sec;
    char st[3];

    FILE *FTime = fopen(STOPRUN,"r");

    memset(st1, '\0', 200);
    memset(st2, '\0', 200);
    memset(st3, '\0', 200);

    fscanf(FTime, "%[^\n]",st1); fgetc(FTime);
    fscanf(FTime, "%[^\n]",st2); fgetc(FTime);
    fscanf(FTime, "%[^\n]",st3); fgetc(FTime);

    fclose(FTime);

    // Get system time
    time_t rawtime;
    struct tm * timeinfo;
    char buffer [80];                                // string for current time
    time ( &rawtime );                               // current time in sec
    timeinfo = localtime ( &rawtime );               // current time in structure
    strftime (buffer,80,"%H:%M:%S",timeinfo);        // time formated

    for(int i = 0; i < 100; i++){
        if (buffer[i] == ':'){
            memset(st, '\0', 3);
            st[0] = buffer[i-2];
            st[1] = buffer[i-1];
            ch = atoi(st);

            st[0] = buffer[i+1];
            st[1] = buffer[i+2];
            min = atoi(st);

            st[0] = buffer[i+4];
            st[1] = buffer[i+5];
            sec = atoi(st);

            break;
        }
    }

    if ((TIME_CAL + min) >= 60.){
        ch += 1;
        min = TIME_CAL + min - 60;
        if (ch >= 24) {
            ch = 0;
        }
    }
    else{
        min += TIME_CAL;
    }
    printf("OPEN:: %s\n",STOPRUN);

    FILE *FTimeNew = fopen(STOPRUN,"w");
    fprintf(FTimeNew, "%02d:%02d:%02d //__stop_RUN_time \n", ch,min,sec);
    fprintf(FTimeNew,"%s\n",st2);
    fprintf(FTimeNew,"%s\n",st3);

    printf("%02d:%02d:%02d //__stop_RUN_time \n", ch,min,sec);
    printf("%s\n",st2);
    printf("%s\n",st3);

    fclose(FTimeNew);

}

const char* Calibration::GetMainDir(){
    memset(find_main_dir, '\0', 218);
    memset(dirname, '\0', 218);

    snprintf(find_main_dir, 218, "ls -lt %s >> tmp.txt", PATH_DATA);

    system(find_main_dir);  // finding the last file
    FILE *ftmp;
    if((ftmp=fopen("tmp.txt", "r"))==NULL) {
        QMessageBox msgBox;
        msgBox.critical(nullptr, "Error", "ERROR:\n tmp.txt is not opened!");
        exit(1);
    }
    char tt[200], tt1[200], tname[200];
    memset(tt, '\0',200);
    fscanf(ftmp, "%[^\n] %[^\n]",tt,tt1);

    memset(tname, '\0',200);

    for (int i = 0; i < 8; i++){
        fscanf(ftmp,"%s",tt);
    }

    fscanf(ftmp, "%s\n", dirname);
    fclose(ftmp);
    system("rm tmp.txt");

    return dirname;
}
