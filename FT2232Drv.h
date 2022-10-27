#ifndef FT2232DRV_H
#define FT2232DRV_H

#include <QObject>
#include "CDM/ftd2xx.h"

class QTimer;

class FT2232Drv : public QObject
{
    Q_OBJECT
public:
    explicit FT2232Drv(QObject *parent = nullptr);
    ~FT2232Drv();


signals:
    void openState(bool);
    void recvData(unsigned char recDat);
    void statusInfo(QString str,int status);


public slots:
    void onInitTimer();
    void onShutterOpenClose(bool bOpenState);
    void onUpdateTimer();

private:
    void initFtDev();
    void readIOdata();
    int getISGnum();

private:
    // bit to control shutter 控制光闸位
    const unsigned char bitShutter = 3;

    QTimer *pUpdateTimer;

    FT_STATUS ftStatus;
    FT_HANDLE ftHandle;

    // shutter state: 1:Open; 0: Close 光闸的设置状态
    bool shutterState;
};

#endif // FT2232DRV_H
