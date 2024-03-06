#include "mainwindow.h"
#include "./ui_mainwindow.h"


#include "runprog.h"
#include <QWidget>
#include <QGridLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QTextBrowser>
#include <QPalette>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setGeometry(500,100,1100,700);
    this->setWindowTitle("Tunka-Grande");

    QGridLayout *grid = new QGridLayout();

    TBrowser1 = new QTextBrowser(this);  // progress status output
    TBrowser2 = new QTextBrowser(this);  // external programs progress output

    QScrollBar *SB1 = new QScrollBar(this);
    QScrollBar *SB2 = new QScrollBar(this);

    TBrowser1->setVerticalScrollBar(SB1);
    TBrowser2->setVerticalScrollBar(SB2);

    TBrowser1->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    TBrowser2->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);


    PB_RUN = new QPushButton("RUN");

    grid->addWidget(TBrowser1, 0, 0);
    grid->addWidget(TBrowser2,0, 1);
    grid->addWidget(PB_RUN,1,1);

    QWidget *window = new QWidget();
    window->setLayout(grid);
    setCentralWidget(window);

    connect(PB_RUN, SIGNAL (released()), this, SLOT (handleButton()));

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::handleButton()
{

    PB_RUN->setStyleSheet("QPushButton{"
                          "background-color: green;"
                          "border-style: inset;"
                          "border-width: 2px;"
                          "border-radius: 5px;"
                          "border-color: green}");
    TBrowser1->insertPlainText("Start RUN\n");
    PB_RUN->setEnabled(false);

    runprog *rp= new runprog(TBrowser1, TBrowser2, PB_RUN);
    rp->RunCommands();
    delete rp;
}


