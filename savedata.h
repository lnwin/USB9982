#ifndef SAVEDATA_H
#define SAVEDATA_H

#include <QObject>

class saveData : public QObject
{
    Q_OBJECT
public:
    explicit saveData(QObject *parent = nullptr);

signals:
};

#endif // SAVEDATA_H
