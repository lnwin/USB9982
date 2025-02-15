#ifndef READDATATHREAD_H
#define READDATATHREAD_H

#include <QObject>
#include <savedata.h>
#include <QThread>
#include <USB9982.h>
LONG   MAX_FIFO   = 1024; //1024M(1GB)
ULONG  DIS_MAXVAL = 0xff;//显示最大值，AD数据最大255 累加数据最大255乘以累加次数
#define CH_CNT 2		//最大通道数
LONG    DEV_VERSION = 0; //A版本0   B版本1
#define  ADD_BW 3 //累加后数据是3个字节
#define  DISPLAY_CNT 1024
class readDataThread : public QThread
{
    Q_OBJECT
public:
    readDataThread();
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
    #define  SOFT_TRIG_CNT 1 //发出软件触发次数
    HANDLE hDevice = INVALID_HANDLE_VALUE;
    UCHAR  devNum ;  //当前打开设备号
    USB9982_PARA_INIT para_init;
    QString fileDir;
    //============================================

signals:
    void sendMSG2UI_Read(QString);
    void sendData2Save(QString filePath,PUCHAR pBuf, int fileSiz);

public slots:
    void getFileDirPath(const QString&);
    void systemInit();
    void readSingleData(const USB9982_PARA_INIT&MP,int singLength);
    void readContinueData(const USB9982_PARA_INIT&MP);
    void stopRead();

};

#endif // READDATATHREAD_H
