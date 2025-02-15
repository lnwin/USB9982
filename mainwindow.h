#ifndef MAINWINDOW_H
#define MAINWINDOW_H
// USB9982.h
#include <windows.h>  // 确保包含LONG的定义
#include <QMainWindow>
#include <QFileDialog>
#include <QFile>
#include <QDataStream>
#include <QTextStream>
#include <QDateTime>
#include <readdatathread.h>
#include <savedata.h>
#include <showdata.h>
#include <QMetaType>
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_startAD_clicked();

    void on_stopAD_clicked();

    void on_singleAD_clicked();

    void on_selectPath_clicked();

    void on_m_bInt_stateChanged(int arg1);

    void getMSG(const QString&);

private:
    Ui::MainWindow *ui;
    USB9982_PARA_INIT mmp;
    readDataThread *myreadDataThread;
    QThread *myreadThread;
    saveData*mysaveData;
    QThread *mysaveThread;
    void readMyPara();
signals:
    void startInit();
    void startSingleAD(const USB9982_PARA_INIT&,int singleLength);
    void starContinueAD(const USB9982_PARA_INIT&);
    void stopAD(const USB9982_PARA_INIT&);
    void sendFIlePath(const QString&);

};
#endif // MAINWINDOW_H
