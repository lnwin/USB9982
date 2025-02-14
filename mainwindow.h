#ifndef MAINWINDOW_H
#define MAINWINDOW_H
// USB9982.h
#include <windows.h>  // 确保包含LONG的定义
#include <QMainWindow>
#include <USB9982.h>
#include <QFileDialog>
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

private:
    Ui::MainWindow *ui;
    //============================================
    bool isOpened=false;
    BOOL  bSoftTrig; //软件触发
    BOOL    bADRun ;//正在采集标志
    HANDLE  hEvent ;//事件
    LONG    samcnt ;//采集样点数
    int		m_nTrigLen;
    LONG m_lChcnt;  //通道数
    #define  MAX_SEGMENT 3//缓冲区数目
    ULONG* dataBuff[MAX_SEGMENT];//采集信息缓冲，采用Block环形缓冲方式
    //============================================
    void findUSBCard();
    void opendCloseCard(const bool & cardStatus);
    USB9982_PARA_INIT para_init;
    void readMyPara();
    void saveMydata(QString filePath,PUCHAR pBuf, int fileSiz);
};
#endif // MAINWINDOW_H
