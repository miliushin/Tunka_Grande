#include "runprog.h"
#include "global.h"

#include <QScrollBar>
#include <QMessageBox>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


using namespace std;

#define PBSTR "##################################################"
#define PBWIDTH 50

void runprog::PrintProgress(double percent){
    int val = (int)(percent  * 100);
    int lpad = (int) (percent * PBWIDTH);
    int rpad = PBWIDTH - lpad;
    char textout[200];
    memset(textout, '\0',200);
    snprintf(textout,200,"Выполнено: %3d%% [%.*s%*s]\n",val, lpad, PBSTR, rpad, "");

    TBrowser1->insertPlainText(textout);
    sb1->setValue(sb1->maximum());

}


runprog::runprog(QTextBrowser *TBrowser11, QTextBrowser *TBrowser22, QPushButton *PB)
{
    NumOfDetCompl = 0;
    Flag_HV = false;
    Flag_THR = false;
    Flag_RUN = false;
    PBRun = PB;
    TBrowser1 = TBrowser11;
    TBrowser2 = TBrowser22;
    sb1 = TBrowser1->verticalScrollBar();
    sb2 = TBrowser2->verticalScrollBar();

    CNF = new Config(TBrowser1);
    Calib = new Calibration(TBrowser1,TBrowser2);

    process = new QProcess(this);
    process->setProcessChannelMode (QProcess::MergedChannels);
    connect (process, SIGNAL(readyRead()), this, SLOT(print_run_process()));
    connect (process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(print_exit_process()));

    p_adata = new QProcess();
    p_adata->setProcessChannelMode (QProcess::MergedChannels);
    connect (p_adata, SIGNAL(readyRead()), this, SLOT(print_run_adata()));
    connect (p_adata, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(print_exit_adata()));
}

runprog::~runprog(){
    delete Calib;
    delete CNF;
    process->close();
    p_adata->close();
}


void runprog::RunCommands(){
    Calib->SetTimeRun();

    run_menu();
    TBrowser1->insertPlainText("======================================\n");
    TBrowser1->insertPlainText("            Установка порогов         \n");
    TBrowser1->insertPlainText("======================================\n");

    print_TBrowser1("Запуск набора данных");
    run_start();
}

void runprog::print_run_process(){
    QByteArray output = process->readAll();

    char strout[100];
    memset(strout,'\0',100);
    snprintf(strout,100,"%s",QString(output).toUtf8().data());

    TBrowser2->insertPlainText(QString(output));
    sb2->setValue(sb2->maximum());
}

void runprog::print_run_adata(){

    QByteArray output = p_adata->readAll();

    char strout[100];
    memset(strout,'\0',100);
    snprintf(strout,100,"%s",QString(output).toUtf8().data());

    printf("string: %s\n",strout);

    TBrowser2->insertPlainText(QString(output));
    QScrollBar *sb = TBrowser2->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void runprog::run_menu(){
    process->start("./tu-gra_6_78+mega.exe");
}

void runprog::run_start(){
    if( !process->waitForStarted() ) {
        return;
    }

    process->write("6 51 27 4"); // 6 61 27 4
    process->closeWriteChannel();

}

/*
void runprog::run_exit(){
    process->write("27");
    process->closeWriteChannel();

    if( !process->waitForFinished() ) {
        return;
    }

    process->close();

}
*/

void runprog::print_exit_process(){
    int iexit = process->exitCode();
    printf("Exit Status: %d\n",iexit);
    //process->close();

    if (iexit == 1) {
        run_adata();
    }
    else if (iexit == 0){
        QMessageBox msgBox;
        msgBox.critical(nullptr, "Error", "ERROR:\n tu-gra_.exe завершилась с ошибкой !\n"
                        "Сделайте restart Tunka-Grande и нажмите ОК.");
        run_menu();
        run_start();
    }
    else{
        QMessageBox msgBox;
        msgBox.critical(nullptr, "Error", "ERROR:\n tu-gra_.exe завершилась с ошибкой !\n"
                        "Программа калибровки будет завершена.");
        exit(1);

    }
}

void runprog::run_adata(){
    print_TBrowser1("Анализ данных...");

    // run data analysis 
    p_adata->start("/home/ima/TakeMaxAmpl/build/TakeMaxAmpl",
                   QStringList() << Calib->GetMainDir());

}

void runprog::print_exit_adata(){
    int iexit = p_adata->exitCode();
    printf("Exit Status ADATA: %d\n",iexit);

    if (iexit == 0) {
        print_TBrowser1("Анализ данных завершен успешно");
    }
    else {
        print_TBrowser1("Анализ данных завершен с ошибкой! ВНИМАНИЕ!");
    }
    if (Flag_THR == false){
        CNF->ThresholdSetting();
        Flag_THR = true;
    }
    else {
        if (Flag_HV == true){
            // ------------ The second and more HV setting ----------------------
            sleep(2);
            CNF->ReadINT();
            for (int i = 0; i < NUMBER_OF_STATIONS; i++){
                for (int j = 0; j < NUMBER_OF_DET; j++){
                    printf("read det status\n");

                    int det_status = CNF->GetDetStatus(i,j);
                    if (det_status == 1){
                        printf("AnalysisHVcounts\n");

                        int id = CNF->AnalysisHVcounts(i,j);
                        if (id == 1){
                            printf("WriteOutAllCNF\n");

                            CNF->WriteOutAllCNF();
                            printf("END WriteOutAllCNF\n");

                            NumOfDetCompl++;
                        }
                    }
                }
            }
            printf("printf progress\n");

            double iProgress = (double)NumOfDetCompl/((double)(CNF->GetNumOfSt()*20));
            PrintProgress(iProgress);
            if (iProgress == 1.){
                print_TBrowser1("Калибровка завершена.");
                PBRun->setStyleSheet("");
                CNF->WriteOutAllCNF();
            }

            Calib->SetTimeRun();
            print_TBrowser1("Запуск набора данных");
            run_menu();
            run_start();
        }
    }

    if (Flag_HV == false){
        // ----------  The first HV setting ----------------------------------
        Flag_HV = true;
        TBrowser1->insertPlainText("======================================\n");
        TBrowser1->insertPlainText("            Установка HV              \n");
        TBrowser1->insertPlainText("======================================\n");
        sb1->setValue(sb1->maximum());
        for (int i = 0; i < NUMBER_OF_STATIONS; i++){
            CNF->SetHV(i,0,3200);
            CNF->SetHV(i,6,3200);

            CNF->SetDetStatus(i,0,1);
            CNF->SetDetStatus(i,6,1);

            //CNF->SetHV(i,12,3200);
            //CNF->SetHV(i,14,3200);
            //CNF->SetHV(i,16,3200);
            //CNF->SetHV(i,18,3200);

            CNF->WriteAllCNF();

        }
        Calib->SetTimeRun();
        print_TBrowser1("Запуск набора данных");
        run_menu();
        run_start();
    }
}


void runprog::make_pb_red(){
    PBRun->setStyleSheet("QPushButton{"
                          "background-color: red;"
                          "border-style: outset;"
                          "border-width: 2px;"
                          "border-radius: 5px;"
                          "border-color: red}");
}

void runprog::print_TBrowser1(const char* text){

    char st[3];
    int sys_ch, sys_min, sys_sec;

    // Get system time
    time_t rawtime;
    struct tm * timeinfo;
    char buffer [80];
    memset(buffer,'\0', 80);
    time ( &rawtime );                               // current time in sec
    timeinfo = localtime ( &rawtime );               // current time in structure
    strftime (buffer,80,"%H:%M:%S",timeinfo);        // time formated

    for(int i = 0; i < 80; i++){
        if (buffer[i] == ':'){
            memset(st, '\0', 3);
            st[0] = buffer[i-2];
            st[1] = buffer[i-1];
            sys_ch = atoi(st);

            st[0] = buffer[i+1];
            st[1] = buffer[i+2];
            sys_min = atoi(st);

            st[0] = buffer[i+4];
            st[1] = buffer[i+5];
            sys_sec = atoi(st);
            break;
        }
    }

    char out[100];
    memset(out, '\0',100);
    snprintf(out,100,"[%02d:%02d:%02d] %s\n",sys_ch,sys_min,sys_sec,text);
    TBrowser1->insertPlainText(out);
    sb1->setValue(sb1->maximum());
}