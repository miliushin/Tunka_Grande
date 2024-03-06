#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QTextBrowser>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:
    void handleButton();

private:
    Ui::MainWindow *ui;
    QPushButton *PB_RUN;
    QTextBrowser *TBrowser1;
    QTextBrowser *TBrowser2;


};



#endif // MAINWINDOW_H
