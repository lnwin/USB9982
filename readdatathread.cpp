#include "readdatathread.h"

readDataThread::readDataThread()
{

}
BOOL readUSB(HANDLE hDevice, PUCHAR pBuf, int bufSiz)//存盘
{
    //DWORD tick1,tick2;
    //tick1 = ::GetTickCount();
    //一次读完samcnt长度数据，例如一个触发长度，如果samcnt大于READ_MAX_LEN，则分为多次读取
    ULONG DataOver =0; //缓存区溢出指示
    ULONG rlen=0;//每次读取长度
    ULONG alen=0;//已经读取长度
    int rcnt = 0;//总共需要读取的次数
    if((bufSiz%READ_MAX_LEN) == 0)//如果读取的长度，刚好是最大允许读取长度的整数倍
        rcnt = (bufSiz/READ_MAX_LEN);
    else
        rcnt = (bufSiz/READ_MAX_LEN)+1;
    //分多次读取，每次读最大长度是READ_MAX_LEN
    for(int i=0;i<rcnt;i++)
    {
        if(i==(rcnt-1))//如果是最后一次
        {
            if ((bufSiz%READ_MAX_LEN) ==0)//如果刚好是整数倍
                rlen = READ_MAX_LEN;
            else
                rlen = bufSiz%READ_MAX_LEN;
        }
        else
        {
            rlen = READ_MAX_LEN;
        }
        //等待缓存数据量达到要求
        //读数
        if (!USB9982_ReadAD(hDevice,pBuf+alen,rlen))
        {
            //m_strOutput+="读取数据失败\r\n";
            //UpdateData(FALSE);
            break;
            return FALSE;
        }
        //判断数据是否溢出
        alen += rlen;
    }

    return TRUE;
}
void readDataThread::systemInit()
{

    devNum = 0;
    for( int i=0;i<32;i++)
    {
        hDevice = USB9982_Link(i);
        if (hDevice != INVALID_HANDLE_VALUE) {
            devNum = i;
            emit sendMSG2UI_Read("Find Device!");
            break;
        }
    }

    //判断设备是否为高速USB设备
    UCHAR speed;
    USB9982_IsHighDevice(hDevice, &speed);
    if (speed==0)
    {
        emit sendMSG2UI_Read("设备枚举为非高速USB设备");
    }
    //读取设备信息
    LONG devADbit = 0;
    USB9982_GetDevInfo(hDevice,&MAX_FIFO,&devADbit,&DEV_VERSION);
    QString str = QString("USB9982(%1bit AD)采集卡打开成功, FIFO %2M采样点，硬件版本号Ver: %3")
                      .arg(devADbit).arg(MAX_FIFO).arg(DEV_VERSION);

    emit sendMSG2UI_Read(str);
}
void readDataThread::readSingleData(const USB9982_PARA_INIT&MP,int singLength)
{
    para_init=MP;
    int myLength=singLength;
    #define  SOFT_TRIG_CNT 1 //发出软件触发次数
    ULONGLONG sicnt=0;
    if((para_init.TriggerSource == TRIG_SRC_SOFT) &&
        (para_init.TriggerMode != TRIG_MODE_CONTINUE))
    {
        bSoftTrig = TRUE;
        sicnt = para_init.TriggerLength*TRIG_UNIT* para_init.lChCnt ;
         emit sendMSG2UI_Read("软触发+单次采集");
    }
    else
    {
        sicnt = 1024*1024*myLength;//每次读取1M个采样点
        bSoftTrig = FALSE;
        emit sendMSG2UI_Read("非软触发+连续采集");
    }

    qDebug()<<"myLength=="<<myLength;

    LONG len1=0,trigcnt1=0;
    //如果累加功能使能，重新计算读取长度
    if(para_init.TriggerMode!= TRIG_MODE_CONTINUE &&
        para_init.bEnADD == TRUE )
    {
        len1 = para_init.TriggerLength*TRIG_UNIT*ADD_BW*para_init.lChCnt ; //一次的累加结果，是一次触发长度乘上累加后的位宽
        QString MSG="触发长度:";MSG.append(QString::number(para_init.TriggerLength*TRIG_UNIT*ADD_BW));
        trigcnt1 =myLength;
        sicnt = len1*trigcnt1;
    }

    if(!USB9982_initADCLK(hDevice,para_init.m_bSelClk,para_init.ClkDeci))
    {
        emit sendMSG2UI_Read("初始化采样钟失败!");
    }
    else
    {
        emit sendMSG2UI_Read("初始化采样钟成功!");
    }

    if (!USB9982_InitAD(hDevice, &para_init))
    {
        DWORD err = GetLastError(); // 获取 Windows 错误码
        emit sendMSG2UI_Read("初始化AD失败，错误码: " + QString::number(err));
        return;
    }
    else
    {
        emit sendMSG2UI_Read("初始化AD成功!");
    }
    //等待初始化AD完成，否则无法接收软件触发
    Sleep(100);


    //软件触发
    if (para_init.TriggerSource == TRIG_SRC_SOFT)
    {
        Sleep(200);//等待设备初始化完成,才能接收软件触发信号
        for(int i=0;i<SOFT_TRIG_CNT;i++)
        {
            if(!USB9982_ExeSoftTrig(hDevice))
            {
                emit sendMSG2UI_Read("软件触发失败\r\n");return ;
            }
        }
    }

    //考虑到计算机内存，分多次读取和保存数据
    #define  MAX_PC_MEM 0x20000000 //每次最大内存分配512采样点
    int read_cnt = 0;
    ULONG  read_len = 0;
    if((sicnt%MAX_PC_MEM) == 0)//如果读取的长度，刚好是最大允许读取长度的整数倍
        read_cnt = (LONG)(sicnt/MAX_PC_MEM);
    else
        read_cnt = (LONG)(sicnt/MAX_PC_MEM)+1;
    for(int i=0;i<read_cnt;i++)
    {
        if(i==(read_cnt-1))//如果是最后一次
        {
            if ((sicnt%MAX_PC_MEM) ==0)//如果刚好是整数倍
                read_len = MAX_PC_MEM;
            else read_len = (LONG)(sicnt%MAX_PC_MEM);
        }
        else {read_len = MAX_PC_MEM;}

        //读取数据
        //如果触发模式非连续采集模式，等待触发事件发生
        LONG bBufOver = 0;
        if(para_init.TriggerMode!=TRIG_MODE_CONTINUE)//触发采集，例如外触发方式，等待FIFO非空再读取数据，避免ReadAD函数死等
        {
            while((bBufOver&0x02)==0)//如果FIFO空，则等待FIFO为非空，如果是实时数据流函数，该等待函数可以去掉，直接执行ReadAD函数。
            {
                //读取FIFO指示，bBufStatus bit0--FIFO满指示，1表示满，0表示非满；bit1--FIFO非空指示，1表示非空，0表示FIFO空。
                USB9982_GetBufOver(hDevice,&bBufOver);
                Sleep(10);//预留时间给其它线程
            }
        }

        //分配内存
        PUCHAR inBuffer = NULL;
        inBuffer = new UCHAR[read_len];
        //读取数据
        if(!readUSB(hDevice,inBuffer,read_len))// 读数
        {
            delete[] inBuffer;
            emit sendMSG2UI_Read("ReadAD失败");
            return;
        }
        //如果是单通道
        if(para_init.lChCnt == 1)
        {


            emit sendData2Save(fileDir,inBuffer,read_len);
            qDebug()<<"get here!";
            //如果 触发模式+累加使能
            if((para_init.TriggerMode != TRIG_MODE_CONTINUE) &&
                ((para_init.bEnADD&0x01)==TRUE))
            {
                //检测累加数据
                ULONG acnt = read_len/ADD_BW;
                PFLOAT dbuf = new FLOAT[acnt]; //分析数据
                PULONG abuf = new ULONG[acnt];
                for(ULONG k=0;k<acnt;k++)//重新组合累加数据
                {
                    abuf[k] = (ULONG)inBuffer[ADD_BW*k+0]|((ULONG)inBuffer[ADD_BW*k+1]<<8)|((ULONG)inBuffer[ADD_BW*k+2]<<16);
                    dbuf[k] = (float)abuf[k]/(float)1000.0; //保存为浮点数据，便于软件分析
                }


                //fwrite(dbuf,sizeof(FLOAT), acnt,fpp);
            }



            else//多个通道
            {
                PUCHAR buf0 = new UCHAR[read_len/para_init.lChCnt];
                for(int j=0;j<para_init.lChCnt;j++)
                {
                    //抽取出各个通道数据
                    int ccnt = read_len/para_init.lChCnt;
                    for(int m=0;m<ccnt;m++)
                        buf0[m] = inBuffer[para_init.lChCnt*m+j];
                    // saveDisk(fp[j],buf0,ccnt);

                }
                delete[]buf0;

                if((para_init.bEnADD&0x01)==TRUE)//如果累加功能使能,判断累加数据
                {
                    int ccnt = read_len/2; // 2表示2CH
                    //检测累加数据
                    ULONG acnt = ccnt/ADD_BW;
                    PULONG abuf0= new ULONG[acnt];
                    PULONG abuf1= new ULONG[acnt];
                    int m=0;
                    for(ULONG k=0;k<acnt;k++)//重新组合累加数据
                    {
                        abuf0[k] = (ULONG)inBuffer[ADD_BW*m+0]|((ULONG)inBuffer[ADD_BW*m+1]<<8)|((ULONG)inBuffer[ADD_BW*m+2]<<16);
                        m++;
                        abuf1[k] = (ULONG)inBuffer[ADD_BW*m+0]|((ULONG)inBuffer[ADD_BW*m+1]<<8)|((ULONG)inBuffer[ADD_BW*m+2]<<16);
                        m++;
                    }

                }
            }//多个通道保存方式
            delete[] inBuffer;
        }

    }
    //停止AD
    if(!USB9982_StopAD(hDevice,devNum))
    {
       emit sendMSG2UI_Read("结束AD失败");
        return ;
    }
    else
    {
        emit sendMSG2UI_Read("结束AD成功");
    }
    //判断溢出位
    emit sendMSG2UI_Read("单次采集成功!");
};
void readDataThread::readContinueData(const USB9982_PARA_INIT&MP)
{

};
void readDataThread::stopRead()
{

};
void readDataThread::getFileDirPath(const QString& mp)
{

    fileDir=mp;

};
