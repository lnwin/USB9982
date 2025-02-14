#include "mainwindow.h"
#include "ui_mainwindow.h"
LONG   MAX_FIFO   = 1024; //1024M(1GB)
ULONG  DIS_MAXVAL = 0xff;//显示最大值，AD数据最大255 累加数据最大255乘以累加次数
#define CH_CNT 2		//最大通道数
LONG    DEV_VERSION = 0; //A版本0   B版本1
#define  ADD_BW 3 //累加后数据是3个字节
#define  DISPLAY_CNT 1024
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    findUSBCard();



}

MainWindow::~MainWindow()
{
    delete ui;
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
HANDLE hDevice = INVALID_HANDLE_VALUE;
UCHAR  devNum ;  //当前打开设备号
void MainWindow::findUSBCard()
{


     hDevice = USB9982_Link(1);

        if (hDevice != INVALID_HANDLE_VALUE)
       {

            ui->textEdit->append("Find No Device!");

        }
        else
        {
            ui->textEdit->append("Find Device!");
            devNum = 1;

        }



    //判断设备是否为高速USB设备
    UCHAR speed;
    USB9982_IsHighDevice(hDevice, &speed);
    if (speed==0)
    {
        ui->textEdit->append("设备枚举为非高速USB设备");
    }
    //读取设备信息
    LONG devADbit = 0;
    USB9982_GetDevInfo(hDevice,&MAX_FIFO,&devADbit,&DEV_VERSION);
    QString mmg="USB9982(8bit AD)采集卡打开成功,硬件版本号Ver:";mmg.append(QString::number(DEV_VERSION));
    mmg.append("MAX_FIFO:").append(QString::number(MAX_FIFO));
    ui->textEdit->append(mmg);
    readMyPara();


};
void MainWindow::readMyPara()
{

    // para_init.TriggerMode = (LONG)ui->TriggerMode->currentIndex();		//触发模式
    // para_init.TriggerSource = (LONG) ui->TriggerSource->currentIndex();	//触发源
    // para_init.lChCnt   = (LONG)ui->lChCnt->currentIndex()+1;	     		//通道数
    // if(ui->m_bEn2G->isChecked())
    // {para_init.lChCnt = (LONG)para_init.lChCnt|EN_AD2G; }//2G模式使能
    // para_init.TriggerLength = (LONG)ui->TriggerLength->text().toInt();//触发长度
    // para_init.TriggerDelay  = (LONG)ui->TriggerDelay->text().toInt(); //触发延时，仅延时触发有效，以FREQsamp/8为单位
    // para_init.TriggerLevel  = (LONG)ui->TriggerLevel->text().toInt(); //USB9982B和USB9982通过函数GetDevInfo 版本号参数来区分，USB9982B版本号1 USB9982版本号0
    //     //(LONG)(m_nfLevel*255.0/1.0)+128 ;//CH1 CH2触发,输入范围-0.5v~+0.5v
    //     //(LONG)(m_nfLevel*4095.0/10.0)+2048 ;//外触发，USB9982B触发电平范围 -5v~+5v
    //     //(LONG)(m_nfLevel*4095.0/5.0);//外触发，USB9982触发电平范围 0~5v
    // para_init.lADGain = (LONG)ui->lADGain->text().toFloat();//放大倍数和dB换算 Amp=10**(dB/20)，仅USB9982A支持，USB9982B/USB9982C不支持
    // para_init.bEnADD  = (LONG)ui->bEnADD->isChecked(); //累加功能使能
    // if(ui->m_bInt->isChecked())
    // {para_init.bEnADD  =(LONG) ui->bEnADD->isChecked() | 0x20;}//累加功能使能,内部计数器测试
    // para_init.lADDcnt = (LONG)ui->lADDcnt->text().toFloat();//累加次数，仅para_init.bEnAdd==TRUE时，该参数有效
    // para_init.lADDthd = (LONG)ui->lADDthd->text().toFloat();//累加门限，仅para_init.bEnAdd==TRUE时，该参数有效
    para_init.TriggerMode = 0;		    //触发模式
    para_init.TriggerSource = 0;		//触发源
    para_init.lChCnt   = 1;						//通道数
    para_init.TriggerLength = 1024;//触发长度
    para_init.TriggerDelay  = 100; //触发延时，仅延时触发有效，以FREQsamp/8为单位
    para_init.TriggerLevel  = 2048; //USB9982B和USB9982通过函数GetDevInfo 版本号参数来区分，USB9982B版本号1 USB9982版本号0
        //(LONG)(m_nfLevel*255.0/1.0)+128 ;//CH1 CH2触发,输入范围-0.5v~+0.5v
        //(LONG)(m_nfLevel*4095.0/10.0)+2048 ;//外触发，USB9982B触发电平范围 -5v~+5v
        //(LONG)(m_nfLevel*4095.0/5.0);//外触发，USB9982触发电平范围 0~5v
    para_init.lADGain = 0;//放大倍数和dB换算 Amp=10**(dB/20)，仅USB9982A支持，USB9982B/USB9982C不支持
    para_init.bEnADD  = 0; //累加功能使能
    para_init.lADDcnt = 1000;//累加次数，仅para_init.bEnAdd==TRUE时，该参数有效
    para_init.lADDthd = 255;//累加门限，仅para_init.bEnAdd==TRUE时，该参数有效

    ui->textEdit->append("触发模式: " + QString::number(para_init.TriggerMode));
    ui->textEdit->append("触发源:" + QString::number(para_init.TriggerSource));
    ui->textEdit->append("通道数:" + QString::number(para_init.lChCnt));
    ui->textEdit->append("触发长度: " + QString::number(para_init.TriggerLength));
    ui->textEdit->append("触发延时: " + QString::number(para_init.TriggerDelay));
    ui->textEdit->append("触发电平: " + QString::number(para_init.TriggerLevel));
    ui->textEdit->append("AD增益: " + QString::number(para_init.lADGain));
    ui->textEdit->append("累加功能使能: " + QString::number(para_init.bEnADD));
    ui->textEdit->append("累加次数: " + QString::number(para_init.lADDcnt));
    ui->textEdit->append("累加门限: " + QString::number(para_init.lADDthd));


    #define  SOFT_TRIG_CNT 1 //发出软件触发次数

    if((para_init.TriggerSource == TRIG_SRC_SOFT) &&
        (para_init.TriggerMode != TRIG_MODE_CONTINUE))
    {
        bSoftTrig = TRUE;
        samcnt = m_nTrigLen*TRIG_UNIT;
        ui->textEdit->append("软触发+单次采集");
    }
    else
    {
        samcnt = 1024*1024*para_init.lChCnt;//每次读取1M个采样点
        bSoftTrig = FALSE;
         ui->textEdit->append("非软触发+采集");
    }
    DIS_MAXVAL = 0xff;//显示最大值

    //如果累加功能使能，重新计算显示最大值和读取长度
    if(para_init.TriggerMode!= TRIG_MODE_CONTINUE &&
        para_init.bEnADD == TRUE )
    {
        DIS_MAXVAL = 0xff*para_init.lADDcnt;//计算最大值 AD最大值*累加次数
        samcnt = para_init.TriggerLength*TRIG_UNIT*ADD_BW*m_lChcnt; //一次的累加结果，是一次触发长度乘上累加后的位宽
        QString MSG="触发长度:";MSG.append(QString::number(ui->TriggerLength->text().toInt()*TRIG_UNIT*ADD_BW));
        ui->textEdit->append(MSG);
    }

    ui->textEdit->append("时钟选择:" + QString::number(ui->m_bSelClk->currentIndex()));
    ui->textEdit->append("分频因子:" + ui->m_nClkdeci->text());
    //if(!USB9982_initADCLK(hDevice,(LONG)ui->m_bSelClk->currentIndex(),(LONG)ui->m_nClkdeci->text().toInt()))
    if(!USB9982_initADCLK(hDevice,0,1))
    {
        ui->textEdit->append("初始化采样钟失败!");
    }
    else
    {
         ui->textEdit->append("初始化采样钟成功!");
    }

    //初始化采集
    if(!USB9982_InitAD(hDevice,&para_init))
    {
        ui->textEdit->append("初始化AD失败!");
    }
    else
    {
        ui->textEdit->append("初始化AD成功!");
    }
    //等待初始化AD完成，否则无法接收软件触发
    Sleep(100);






};
void MainWindow::opendCloseCard(const bool &ST)
{

};

void MainWindow::on_startAD_clicked()
{


}


void MainWindow::on_stopAD_clicked()
{
    if(!USB9982_StopAD(hDevice,devNum))
    {
        ui->textEdit->append("结束AD失败");
    }
}

#define  SOFT_TRIG_CNT 1 //发出软件触发次数

void MainWindow::on_singleAD_clicked()
{



    //软件触发
    if (para_init.TriggerSource == TRIG_SRC_SOFT)
    {
        Sleep(200);//等待设备初始化完成,才能接收软件触发信号
        for(int i=0;i<SOFT_TRIG_CNT;i++)
        {
            if(!USB9982_ExeSoftTrig(hDevice))
            { ui->textEdit->append("软件触发失败\r\n");return ;
            }
        }
    }
    //考虑到计算机内存，分多次读取和保存数据
    ULONGLONG sicnt=0;
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
            ui->textEdit->append("ReadAD失败");
            return;
        }

        //如果是单通道
        if(m_lChcnt == 1)
        {

           // saveDisk("dsd",inBuffer,read_len); //======================保存数据

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


                //fwrite(dbuf,sizeof(FLOAT), acnt,fpp); 需要保存数据
            }



        else//多个通道
        {
            PUCHAR buf0 = new UCHAR[read_len/m_lChcnt];
            for(int j=0;j<m_lChcnt;j++)
            {
                //抽取出各个通道数据
                int ccnt = read_len/m_lChcnt;
                for(int m=0;m<ccnt;m++)
                    buf0[m] = inBuffer[m_lChcnt*m+j];
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
       ui->textEdit->append("结束AD失败");
        return ;
    }
    //判断溢出位
    ui->textEdit->append("单次采集成功!");
}


void MainWindow::on_selectPath_clicked()
{

   QString fileDir=QFileDialog::getExistingDirectory( this, "Rec path", "/");

}

void MainWindow::saveMydata(QString filePath,PUCHAR pBuf, int fileSiz)
{

};

void MainWindow::on_m_bInt_stateChanged(int arg1)
{
    ui->textEdit->append(QString::number((long)ui->m_bInt->isChecked()));
}

