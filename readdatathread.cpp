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
    for(int i=0;i<MAX_SEGMENT;i++)
    {
        dataBuff[i] = NULL;
    }

    emit sendMSG2UI_Read(str);
}
void readDataThread::readSingleData(const USB9982_PARA_INIT&MP,int singLength)
{
    para_init=MP;
    int myLength=singLength;
    LONG bBufOver = 0;
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
        emit sendMSG2UI_Read(MSG);
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


             saveMyData(fileDir,inBuffer,read_len,"single");

            //如果 触发模式+累加使能
            if((para_init.TriggerMode != TRIG_MODE_CONTINUE) &&
                ((para_init.bEnADD&0x01)==TRUE))
            {
                //检测累加数据
                qDebug()<<"ljljlj===========";
                ULONG acnt = read_len/ADD_BW;
                PFLOAT dbuf = new FLOAT[acnt]; //分析数据
                PULONG abuf = new ULONG[acnt];
                for(ULONG k=0;k<acnt;k++)//重新组合累加数据
                {
                    abuf[k] = (ULONG)inBuffer[ADD_BW*k+0]|((ULONG)inBuffer[ADD_BW*k+1]<<8)|((ULONG)inBuffer[ADD_BW*k+2]<<16);
                    dbuf[k] = (float)abuf[k]/(float)1000.0; //保存为浮点数据，便于软件分析
                }
                qDebug()<<"ljljlj";
                saveMyLJData(fileDir,dbuf,acnt,"LJ_single");
            }

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
                     //saveMyData(fileDir,buf0,ccnt);
                }
                //delete[]buf0;

                // if((para_init.bEnADD&0x01)==TRUE)//如果累加功能使能,判断累加数据
                // {
                //     int ccnt = read_len/2; // 2表示2CH
                //     //检测累加数据
                //     ULONG acnt = ccnt/ADD_BW;
                //     PULONG abuf0= new ULONG[acnt];
                //     PULONG abuf1= new ULONG[acnt];
                //     int m=0;
                //     for(ULONG k=0;k<acnt;k++)//重新组合累加数据
                //     {
                //         abuf0[k] = (ULONG)inBuffer[ADD_BW*m+0]|((ULONG)inBuffer[ADD_BW*m+1]<<8)|((ULONG)inBuffer[ADD_BW*m+2]<<16);
                //         m++;
                //         abuf1[k] = (ULONG)inBuffer[ADD_BW*m+0]|((ULONG)inBuffer[ADD_BW*m+1]<<8)|((ULONG)inBuffer[ADD_BW*m+2]<<16);
                //         m++;
                //     }

                // }
            }

    }
    //读取溢出指示
    if (!USB9982_GetBufOver(hDevice, &bBufOver))
    {
        emit sendMSG2UI_Read("Error");
        return;
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

int myCount=0;
void readDataThread::readContinueData(const USB9982_PARA_INIT&MP)
{
    para_init=MP;
    if((para_init.TriggerSource == TRIG_SRC_SOFT) &&
        (para_init.TriggerMode != TRIG_MODE_CONTINUE))
    {
        bSoftTrig = TRUE;
        samcnt = para_init.TriggerLength*TRIG_UNIT;
    }
    else
    {
        samcnt = 1024*1024*para_init.lChCnt;//每次读取1M个采样点
        bSoftTrig = FALSE;
        bSoftTrig = FALSE;
    }

    DIS_MAXVAL = 0xff;//显示最大值
    //如果累加功能使能，重新计算显示最大值和读取长度
    if(para_init.bEnADD)
    {
        DIS_MAXVAL = 0xff*para_init.lADDcnt;//计算最大值 AD最大值*累加次数
        samcnt = para_init.TriggerLength*TRIG_UNIT*ADD_BW*para_init.lChCnt; //一次的累加结果，是一次触发长度乘上累加后的位宽
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
    Sleep(100);


    // //重新分配缓冲区
    // if(dataBuff)
    // {
    //     for(int i=0;i<MAX_SEGMENT;i++)
    //     {delete [] dataBuff[i];dataBuff[i] = NULL;}
    // }

    // //多缓冲
    // for( int i=0;i<MAX_SEGMENT;i++)
    //     dataBuff[i] = new ULONG[samcnt];//每个缓冲区存放一个通道AD数据
    // //初始化多缓冲标志


    PUCHAR inBuffer = NULL;
    ULONG status=FALSE;
    inBuffer = new UCHAR[samcnt];
    LONG i=0;
    LONG  bBufOver = 0;

    memset(inBuffer,0,samcnt*sizeof(UCHAR));
    emit sendMSG2UI_Read("start reading!");
    QDateTime currentTime=QDateTime::currentDateTime();
    QString MT1=fileDir+"/_continueHC1_"+QString::number(myCount)+".bin";
    QString MT2=fileDir+"/_continueHC2_"+QString::number(myCount)+".bin";
    FILE *fp0;
    FILE *fp1;
    std::string fileName1=MT1.toStdString();
    std::string fileName2=MT2.toStdString();
    fp0=fopen(fileName1.c_str(),"wb");
    fp1=fopen(fileName2.c_str(),"wb");
    myCount+=1;
    while(bADRun)
    {


        //如果是软件触发，发出一个触发脉冲
        if (bSoftTrig)
        {
            USB9982_ExeSoftTrig(hDevice);
        }

      //  if(bADRun==FALSE) goto exit_read;

        if(!readUSB(hDevice,inBuffer,samcnt))
        {
            emit sendMSG2UI_Read("Error: ReadAD faile");
            goto exit_read;
        }

        //读取溢出
        if (!USB9982_GetBufOver(hDevice,&bBufOver)) {
            emit sendMSG2UI_Read("Error: du qu yi chu");
            goto exit_read;
        }

        if((bBufOver&0x01)==1)
        {
            emit sendMSG2UI_Read("huan cun yi chu");
        }




        LONG mm = 1;
        int display_ch=0;
        if(para_init.lChCnt==2 && para_init.m_bEn2G==FALSE)
        {
             mm = 2;
             display_ch=1;
        }
        qDebug()<<"mm ===="<<mm;
        //如果双通道，使能了AD的2G工作方式，CH1,CH2（和CH1相位差180度）组成一个通道数据，一起显示
        //仅2G工作方式未使能,CH1 CH2单独显示
        LONG tcnt;
        QList<float> ch1_data;
        QList<float> ch2_data;
        //如果使能累加功能,重新组织数据
        if(para_init.bEnADD){
            tcnt = samcnt/ADD_BW/mm;

            if(mm==1)
            {
                PUCHAR myBuffer = NULL;
                myBuffer = new UCHAR[tcnt];
                for(i=0;i<tcnt;i++)
                {myBuffer[i] =  inBuffer[ADD_BW*(mm*i+display_ch)]|((inBuffer[ADD_BW*(mm*i+display_ch)+1])<<8)|(inBuffer[ADD_BW*(mm*i+display_ch)+2]<<16);}
                saveMyContinueData(fileDir,myBuffer,tcnt,"LJ_CH1");
            }
            else
            {
                PUCHAR myBuffer1 = NULL;
                myBuffer1 = new UCHAR[tcnt];
                PUCHAR myBuffer2 = NULL;
                myBuffer2 = new UCHAR[tcnt];
                for(i=0;i<tcnt;i++)//从通道中抽取数据
                {
                    myBuffer1[i] =  inBuffer[ADD_BW*(mm*i)]|((inBuffer[ADD_BW*(mm*i)+1])<<8)|(inBuffer[ADD_BW*(mm*i)+2]<<16);
                    myBuffer2[i] =  inBuffer[ADD_BW*(mm*i+display_ch)]|((inBuffer[ADD_BW*(mm*i+display_ch)+1])<<8)|(inBuffer[ADD_BW*(mm*i+display_ch)+2]<<16);
                }
                 saveMyContinueData(fileDir,myBuffer1,tcnt,"LJ_CH1");
                 saveMyContinueData(fileDir,myBuffer2,tcnt,"LJ_CH2");
            }
        }
        else
        {
            tcnt = samcnt/mm;
            if(mm==1)
            {
                PUCHAR myBuffer = NULL;
                myBuffer = new UCHAR[tcnt];
                 for(i=0;i<tcnt;i++)//从通道中抽取数据
                {
                     myBuffer[i] = (ULONG)inBuffer[mm*i + display_ch];
                }
                //saveMyContinueData(fileDir,myBuffer,tcnt,"continue_CH1");
                fwrite(myBuffer,sizeof(UCHAR), tcnt,fp0);
            }
            else
            {
                PUCHAR myBuffer1 = NULL;
                myBuffer1 = new UCHAR[tcnt];
                PUCHAR myBuffer2 = NULL;
                myBuffer2 = new UCHAR[tcnt];
                for(i=0;i<tcnt;i++)//从通道中抽取数据
                {
                    myBuffer1[i] = (ULONG)inBuffer[mm*i ];
                    myBuffer2[i] = (ULONG)inBuffer[mm*i + display_ch];
                }
                saveMyContinueData(fileDir,myBuffer1,tcnt,"continue_CH1");
                saveMyContinueData(fileDir,myBuffer2,tcnt,"continue_CH2");
            }


        }


    }


    exit_read:
    delete[] inBuffer;
    inBuffer = NULL;
    fclose(fp0);
    emit sendMSG2UI_Read("read exit!");


};
void readDataThread::stopRead()
{
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
};
void readDataThread::getFileDirPath(const QString& mp)
{

    fileDir=mp;

};
void readDataThread::saveMyData(QString filePath,PUCHAR pBuf, int fileSiz,QString type)
{

    PUCHAR myBuf = pBuf;
    QDateTime currentTime=QDateTime::currentDateTime();
    QString MT="/"+currentTime.toString("yyyy_hh_mm_ss")+"_"+type+".bin";
    filePath.append(MT);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        // ui->textEdit->append("无法打开文件");
        return ;
    }

    // 创建QDataStream对象并关联到文件
    QDataStream out(&file);
    // 设置数据的字节序和版本
    out.setByteOrder(QDataStream::LittleEndian);
    out.setVersion(QDataStream::Qt_5_15);

    // 写入数据到文件
    for (int i = 0; i < fileSiz; ++i) {
        out << myBuf[i];
    }

    // 检查写入是否成功
    if (out.status() != QDataStream::Ok) {
        // ui->textEdit->append(".....");
    }

    // 关闭文件
    file.close();
    // emit sendMSG2UI("save File success");

};
void readDataThread::saveMyContinueData(QString filePath,PUCHAR pBuf, int fileSiz,QString type)
{
    PUCHAR myBuf = pBuf;
    QDateTime currentTime=QDateTime::currentDateTime();
   // QString MT="/"+currentTime.toString("yyyy_hh_mm")+"_"+type+".bin";
    // QString MT="/"+type+".bin";
    // filePath.append(MT);
    // file1.setFileName(filePath);
    // if (!file1.open(QIODevice::Append | QIODevice::WriteOnly)) {
    //     // ui->textEdit->append("无法打开文件");
    //     return ;
    // }

    // // 创建QDataStream对象并关联到文件
    // out1.setDevice(&file1);
    // // 设置数据的字节序和版本
    // out1.setByteOrder(QDataStream::LittleEndian);
    // out1.setVersion(QDataStream::Qt_5_15);

    // 写入数据到文件
    for (int i = 0; i < fileSiz; ++i) {
        out1 << myBuf[i];
    }

    // 检查写入是否成功
    if (out1.status() != QDataStream::Ok) {
        // ui->textEdit->append(".....");
    }

    // 关闭文件
   // file.close();

}
void readDataThread::saveMyLJData(QString filePath,PFLOAT pBuf, int fileSiz,QString type)
{

    PFLOAT myBuf = pBuf;
    QDateTime currentTime=QDateTime::currentDateTime();
    //QString MT="/"+currentTime.toString("yyyy_hh_mm_ss")+"_"+type+"_LJ.bin";
    QString MT="/"+type+".bin";
    filePath.append(MT);
    file2.setFileName(filePath);
    if (!file2.open(QIODevice::Append | QIODevice::WriteOnly)) {
        // ui->textEdit->append("无法打开文件");
        return ;
    }

    // 创建QDataStream对象并关联到文件
    QDataStream out(&file2);
    // 设置数据的字节序和版本
    out.setByteOrder(QDataStream::LittleEndian);
    out.setVersion(QDataStream::Qt_5_15);

    // 写入数据到文件
    for (int i = 0; i < fileSiz; ++i) {
        out << myBuf[i];
    }

    // 检查写入是否成功
    if (out.status() != QDataStream::Ok) {
        // ui->textEdit->append(".....");
    }

    // 关闭文件
   // file.close();
    // emit sendMSG2UI("save LJFile success");
};

