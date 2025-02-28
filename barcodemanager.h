#ifndef BARCODEMANAGER_H
#define BARCODEMANAGER_H

#include <QObject>

class BarcodeManager : public QObject
{
    Q_OBJECT
public:
    explicit BarcodeManager(QObject *parent = nullptr);

signals:

};

#endif // BARCODEMANAGER_H
