#include "savedata.h"

saveData::saveData(QObject *parent)
    : QObject{parent}
{}


void saveData::saveMyData(QString filePath,PUCHAR pBuf, int fileSiz)
{

    QDateTime currentTime=QDateTime::currentDateTime();
    QString MT="/"+currentTime.toString("yyyy_hh_mm_ss")+".bin";
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
        out << pBuf[i];
    }

    // 检查写入是否成功
    if (out.status() != QDataStream::Ok) {
       // ui->textEdit->append(".....");
    }

    // 关闭文件
    file.close();


};
