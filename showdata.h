#ifndef SHOWDATA_H
#define SHOWDATA_H

#include <QObject>
#include <QThread>
class showData: public QThread
{
    Q_OBJECT
public:
    showData();
};

#endif // SHOWDATA_H
