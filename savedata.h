#ifndef SAVEDATA_H
#define SAVEDATA_H
#include <Windows.h>
#include <QObject>
#include <QDateTime>
#include <QFile>
#include <QDataStream>
#include <QThread>
#include <QDebug>
class saveData : public QThread
{
    Q_OBJECT
public:
    explicit saveData(QObject *parent = nullptr);

    void saveMyData(QString filePath,PUCHAR pBuf, int fileSiz);

signals:

    void sendMSG2UI();

};

#endif // SAVEDATA_H
