#include "FT2232Drv.h"
#include <QTimer>
#include <QDebug>


FT2232Drv::FT2232Drv(QObject *parent)
    : QObject{parent}
    , pUpdateTimer(nullptr)
    , ftStatus (FT_DEVICE_NOT_FOUND)
    , shutterState(false)
{
}

FT2232Drv::~FT2232Drv()
{
    ftStatus = FT_SetBitMode(ftHandle, 0, 0);
    ftStatus = FT_Close(ftHandle); //close device
}

void FT2232Drv::onInitTimer()
{
    pUpdateTimer = new QTimer();
    connect(pUpdateTimer, SIGNAL(timeout()), this, SLOT(onUpdateTimer()));
    pUpdateTimer->setInterval(300);
    pUpdateTimer->start();
}

void FT2232Drv::onShutterOpenClose(bool bOpenState)
{
    shutterState = bOpenState;
    readIOdata();
}

void FT2232Drv::onUpdateTimer()
{
    if(ftStatus != FT_OK)
        initFtDev();

    readIOdata();
}

// Search device number 查找设备连接号
int FT2232Drv::getISGnum()
{
    int devNum = -1;
    FT_STATUS FT_status;    // status of the FT 232 chip

    DWORD numDevs;
    FT_DEVICE_LIST_INFO_NODE *devInfo;
    FT_status = FT_CreateDeviceInfoList(&numDevs);
    if (FT_status != FT_OK)
        return -1;

    if (numDevs == 0)
        return -1;

    // allocate storage for list based on numDevs
    devInfo = (FT_DEVICE_LIST_INFO_NODE*)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE)*numDevs);
    // get the device information list
    FT_status = FT_GetDeviceInfoList(devInfo, &numDevs);
    if (FT_status != FT_OK)
        return -1;

    qDebug() << " ";
    for (DWORD i = 0; i < numDevs; i++) {
        QString desp = QString(QLatin1String(devInfo[i].Description));
        qDebug() << "descriptrion:" << desp << i;
        if(desp == "ISG-250 A" || desp =="USB <-> Serial Converter B") {
            devNum = i;
            break;
        }
    }

    return  devNum;
}

void FT2232Drv::initFtDev()
{
    // Confirm device 确定器件
    int devNum = getISGnum();
    QString msg = "ISG-250 Not found";
    if(devNum<0) {
        emit openState(false);
        emit statusInfo(msg, devNum);
    }

    // init 初始化
    ftStatus = FT_Open(devNum, &ftHandle);
    ftStatus |= FT_SetUSBParameters(ftHandle, 4096, 4096); // Set USB transfer sizes
    ftStatus |= FT_SetChars(ftHandle, false, 0, false, 0); // Disable event characters
    ftStatus |= FT_SetTimeouts(ftHandle, 5000, 5000); // Set read/write timeouts to 5 sec
    ftStatus |= FT_SetLatencyTimer(ftHandle, 16); // Latency timer at default 16ms
    ftStatus |= FT_SetFlowControl(ftHandle, FT_FLOW_NONE, 0x11, 0x13);
    ftStatus |= FT_SetBaudRate(ftHandle, 62500); //bit rate is x16 this value = 1M
    if (ftStatus != FT_OK) {
        emit openState(false);
        QString msg = "ISG-250 open failed"; //check for error
        emit statusInfo(msg, ftStatus); //check for error
        qDebug() << msg << ftStatus; //check for error
    }
    else {
        emit openState(true);
        // emit statusInfo("ISG-250 open OK", ftStatus); //check for error
    }
}


void FT2232Drv::readIOdata()
{
    if (ftStatus != FT_OK) return;

    pUpdateTimer->stop();
    DWORD w_data_len = 3; //write 3 bytes
    DWORD data_written; // number of bytes written
    UCHAR Mask = 0x00; //Set input/output [input (0) and output (1)]
    UCHAR Mode = 0x04; //0x04 = synchronous bit-bang
    DWORD RxBytes; //number of bytes to be read from the device
    DWORD BytesReceived; //number of bytes read from the device
    byte RxBuffer[8]; //buffer to receive data from FT-X device
    byte data_out[8]; //buffer for data to be sent to FT-X device

    Mode = FT_BITMODE_SYNC_BITBANG; //0x04 = synchronous bit-bang

    //Set input/output [input (0) and output (1)]
    //0 D7 (reserved)
    //0 D6 input, +24VSI 1=hardware HV enabled, 0=disabled
    //0 D5 input, 1=shutter is closed 0=shutter is open
    //1 D4 (output, reserved)
    //1 D3 Output, 1= open shutter, 0= close shutter.
    //1 D2 (output, reserved)
    //0 D1 (reserved)
    //0 D0 input 1=remote enabled 0=not enabled
    Mask = 0x1C;

    // perpare datOut 根据光闸状态调整
    unsigned char datOut = 0;
    if(shutterState)
        datOut |= 1<<bitShutter;
    else
        datOut &= ~(1<<bitShutter);

    data_out[0] = datOut;
    data_out[1] = datOut;
    data_out[2] = datOut;

    w_data_len = 2;

    //enter synchronous bit-bang mode
    ftStatus = FT_SetBitMode(ftHandle, Mask, Mode);
    if(ftStatus != FT_OK) {
        pUpdateTimer->start();
        return;
    }


    //write data to pins
    ftStatus = FT_Write(ftHandle, data_out, w_data_len, &data_written);
    if(ftStatus != FT_OK) {
        pUpdateTimer->start();
        return;
    }

    do{
        //Sleep(1000);
        FT_GetQueueStatus(ftHandle, &RxBytes);
        //Sleep(10); //delay
    } while (RxBytes != (w_data_len)); //check for all bytes to be returned

    //once bytes are received, use FT_Read to place in RxBuffer
    if (RxBytes > 0) {
        ftStatus = FT_Read(ftHandle, RxBuffer, RxBytes, &BytesReceived);
        if(ftStatus != FT_OK) {
            pUpdateTimer->start();
            return;
        }

        unsigned char dataRecv = RxBuffer[w_data_len-1];
        emit recvData(dataRecv);
    }

    pUpdateTimer->start();
}
