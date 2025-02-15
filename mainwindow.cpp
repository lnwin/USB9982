#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    myreadDataThread =new readDataThread();
    myreadThread = new QThread();
    myreadDataThread->moveToThread(myreadThread);
    myreadThread->start();
    mysaveData=new saveData();
    mysaveThread=new  QThread ();
    mysaveData->moveToThread(mysaveThread);
    mysaveThread->start();

    connect(this,&MainWindow::startInit,myreadDataThread,&readDataThread::systemInit);
    connect(this,&MainWindow::sendFIlePath,myreadDataThread,&readDataThread::getFileDirPath);
    connect(this,&MainWindow::startSingleAD,myreadDataThread,&readDataThread::readSingleData);
    connect(this,&MainWindow::starContinueAD,myreadDataThread,&readDataThread::readContinueData);
//==================================================================================================
    connect(myreadDataThread,&readDataThread::sendMSG2UI_Read,this,&MainWindow::getMSG);


    emit startInit();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::getMSG(const QString&mmg)
{
    ui->textEdit->append(mmg);
};

void MainWindow::readMyPara()
{
    mmp.TriggerMode = (LONG)ui->TriggerMode->currentIndex();		//触发模式
    mmp.TriggerSource = (LONG) ui->TriggerSource->currentIndex();	//触发源
    mmp.lChCnt   = (LONG)ui->lChCnt->currentIndex()+1;	     		//通道数
    if(ui->m_bEn2G->isChecked())
    {mmp.lChCnt = (LONG)mmp.lChCnt|EN_AD2G; }//2G模式使能
    mmp.TriggerLength = (LONG)ui->TriggerLength->text().toInt();//触发长度
    mmp.TriggerDelay  = (LONG)ui->TriggerDelay->text().toInt(); //触发延时，仅延时触发有效，以FREQsamp/8为单位
    mmp.TriggerLevel  = (LONG)ui->TriggerLevel->text().toInt(); //USB9982B和USB9982通过函数GetDevInfo 版本号参数来区分，USB9982B版本号1 USB9982版本号0
        //(LONG)(m_nfLevel*255.0/1.0)+128 ;//CH1 CH2触发,输入范围-0.5v~+0.5v
        //(LONG)(m_nfLevel*4095.0/10.0)+2048 ;//外触发，USB9982B触发电平范围 -5v~+5v
        //(LONG)(m_nfLevel*4095.0/5.0);//外触发，USB9982触发电平范围 0~5v
    mmp.lADGain = (LONG)ui->lADGain->text().toFloat();//放大倍数和dB换算 Amp=10**(dB/20)，仅USB9982A支持，USB9982B/USB9982C不支持
    mmp.bEnADD  = (LONG)ui->bEnADD->isChecked(); //累加功能使能
    if(ui->m_bInt->isChecked())
    {mmp.bEnADD  =(LONG) ui->bEnADD->isChecked() | 0x20;}//累加功能使能,内部计数器测试
    mmp.lADDcnt = (LONG)ui->lADDcnt->text().toFloat();//累加次数，仅para_init.bEnAdd==TRUE时，该参数有效
    mmp.lADDthd = (LONG)ui->lADDthd->text().toFloat();//累加门限，仅para_init.bEnAdd==TRUE时，该参数有效

};


void MainWindow::on_startAD_clicked()
{

    readMyPara();

}


void MainWindow::on_stopAD_clicked()
{

}



void MainWindow::on_singleAD_clicked()
{
    readMyPara();
    emit startSingleAD(mmp,ui->m_lsicnt->text().toInt());
}


void MainWindow::on_selectPath_clicked()
{

  QString myfileDir=QFileDialog::getExistingDirectory( this, "Rec path", "/");
  ui->savePath->setText(myfileDir);
  emit sendFIlePath(myfileDir);

}

void MainWindow::on_m_bInt_stateChanged(int arg1)
{
    ui->textEdit->append(QString::number((long)ui->m_bInt->isChecked()));
}

