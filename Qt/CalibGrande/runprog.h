#ifndef RUNPROG_H
#define RUNPROG_H

#include <QProcess>
#include <QObject>
#include <QTextBrowser>
#include <QPushButton>

#include "configuration.h"
#include "calibration.h"

class runprog: public QObject
{
    Q_OBJECT

public:
    runprog(QTextBrowser *TBrowser11, QTextBrowser *TBrowser22, QPushButton *PB_RUN);
    ~runprog();

    void RunCommands();

public slots:
    void print_run_process();
    void print_exit_process();
    void print_run_adata();
    void print_exit_adata();

private:
    int NumOfDetCompl;

    void run_menu();
    void run_start();
    void run_exit();
    void run_adata();
    void make_pb_red();
    void PrintProgress(double percent);
    void print_TBrowser1(const char* text);

    bool Flag_HV;
    bool Flag_THR;
    bool Flag_RUN;

    QPushButton  *PBRun;
    QTextBrowser *TBrowser1;
    QTextBrowser *TBrowser2;
    QProcess *process;
    QProcess *p_adata;
    QScrollBar *sb1;
    QScrollBar *sb2;

    Config *CNF;
    Calibration *Calib;

};

#endif // RUNPROG_H
