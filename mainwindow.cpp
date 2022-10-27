#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QThread>
#include <QTimer>
#include <QDebug>

#include "FT2232Drv.h"
#include "CDM/ftd2xx.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    ,    ui(new Ui::MainWindow)
    ,    pWorker(nullptr)
    ,    pThread(nullptr)
{
    ui->setupUi(this);

    // init thread object 初始化多线程
    pWorker = new FT2232Drv();
    connect(this,  &MainWindow::shutterOpenClose,  pWorker,  &FT2232Drv::onShutterOpenClose );
    connect(pWorker,  &FT2232Drv::openState,  this,  &MainWindow::onOpenState);
    connect(pWorker,  &FT2232Drv::recvData,  this,  &MainWindow::onRecvData);
    connect(pWorker,  &FT2232Drv::statusInfo,  this,  &MainWindow::onStatusInfo);

    pThread = new QThread();
    connect(pThread,  &QThread::finished,  pThread,  &QThread::deleteLater);
    connect(pThread,  &QThread::finished,  pWorker,  &QObject::deleteLater);
    connect(pThread,  &QThread::started,  pWorker,  &FT2232Drv::onInitTimer);

    pWorker->moveToThread(pThread);
    pThread->start();
}

MainWindow::~MainWindow()
{
    pThread->quit();
    pThread->wait();

    delete ui;
}

// receive Open device state 收到 打开设备状态 的槽函数
void MainWindow::onOpenState(bool bOpenState)
{
    qDebug() << "Open ISG-250 state: " << bOpenState;
}

// receive a data [*] 收到 IO 数据的槽函数 [重点处理位置]
void MainWindow::onRecvData(unsigned char recDat)
{
    QString msg =  QString::number(recDat, 16)
            + " - " + QString::number(recDat, 2)
            + " - " + QString::number(recDat);
    ui->ioData->setText(msg);
}

// status infomation 收到 状态信息的槽函数
void MainWindow::onStatusInfo(QString str, int status)
{
    QString msg =  QString::number(status, 16)
            + " - " + QString::number(status, 2)
            + " - " + QString::number(status);

    msg = str + msg;
    ui->statusbar->showMessage(msg);
}


// 开光闸
void MainWindow::on_pbShutterOpen_clicked()
{
    emit shutterOpenClose(true);
}


// 关光闸
void MainWindow::on_pbShutterClose_clicked()
{
    emit shutterOpenClose(false);
}


// ========= test ftdi device in MainWindow ===========
// ========= 以下是在 MainWindow 中进行对 ftdi 设备测试部分 不必加入到程序中 ===========
// Search device number 查找设备连接号
int MainWindow::getFTnum()
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
        if(desp =="USB <-> Serial Converter A") {
            devNum = i;
            break;
        }
    }

    return  devNum;
}

// get ftdi device information 获取 ftdi 设备信息
void MainWindow::on_pbFtInfo_clicked()
{
    FT_STATUS FT_status;    // status of the FT 232 chip

    DWORD numDevs;
    FT_DEVICE_LIST_INFO_NODE *devInfo;
    FT_status = FT_CreateDeviceInfoList(&numDevs);
    if (FT_status == FT_OK) {
        ui->ftInfo->clear();
        ui->ftInfo->append( "Number of devices is " + QString::number(numDevs));
    }

    if (numDevs > 0) {
        // allocate storage for list based on numDevs
        devInfo = (FT_DEVICE_LIST_INFO_NODE*)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE)*numDevs);
        // get the device information list
        FT_status = FT_GetDeviceInfoList(devInfo,&numDevs);
        if (FT_status == FT_OK) {
            for (auto i = 0; i < numDevs; i++) {
                ui->ftInfo->append("Dev :" +QString::number( i));
                ui->ftInfo->append(" Flags=" + QString::number( devInfo[i].Flags, 16));
                ui->ftInfo->append(" Type=" + QString::number( devInfo[i].Type, 16));
                ui->ftInfo->append(" ID=" + QString::number( devInfo[i].ID, 16));
                ui->ftInfo->append(" LocId=" + QString::number( devInfo[i].LocId, 16));            // *
                ui->ftInfo->append(" SerialNumber=" + QString(QLatin1String(devInfo[i].SerialNumber)));
                ui->ftInfo->append(" Description=" + QString(QLatin1String(devInfo[i].Description)));
//                ui->ftInfo->append(" ftHandle=" + QString::number(devInfo[i].ftHandle));
                ui->ftInfo->append("");
            }
        }
    }
}


//  FTD2XX Asynchronous Bit Bang 模式测试
void MainWindow::on_FTD2XX_AsyncBitBang_clicked()
{
    FT_HANDLE FT_handle;    // handle to FT 232 chip
    FT_STATUS FT_status;    // status of the FT 232 chip

    UCHAR Mask;             // for selecting which pins are input/output
    UCHAR Mode;             // Selects the mode of operation for the chip
    DWORD BaudRate;         // for selecting the Baud Rate
    UCHAR Buffer = 0x32;    // data to be written to the port
    DWORD BytesWritten = 0; // No of bytes written by FT_Write()

    qDebug() << Qt::endl;
    DWORD numDevs;
    FT_DEVICE_LIST_INFO_NODE *devInfo;
    FT_status = FT_CreateDeviceInfoList(&numDevs);
    if (FT_status == FT_OK) {
        qDebug() << "Number of devices is " << numDevs;
    }

    if (numDevs==0) {
        qDebug() << "No deveice detected! Exit." << Qt::endl;
        return;
    }

    FT_status = FT_Open(0, &FT_handle); // Open  a connection to FT232RL

    if(FT_status == FT_OK)             // Error Checking
        qDebug() << "\t  Connection to FT232 opened ";
    else
        qDebug() << "\t  ERROR! in Opening connection";

    Mode = FT_BITMODE_ASYNC_BITBANG;
    Mode = 0x01;              // Select Chip mode as Asynchronous Bit Bang
    Mask = 0xff;              // Direction Mask, 8 bit port all output

    FT_status = FT_SetBitMode(FT_handle, Mask, Mode); //sets Asynchronous BBM

    if(FT_status == FT_OK)             // Error Checking
        {
            qDebug() <<"\t  Mode = " << Mode << ", Asynchronous Bit Bang Mode Selected";
            qDebug() <<"\t  Mask = " << Mask << ", Direction = All 8 bits output";
        }
    else
        qDebug() << "\t  ERROR! in setting Asynchronous Bit Bang Mode";


    // 以下转换成串口模式，发送一个字符
    BaudRate = 9600;       // Setting the Baud rate to 9600

    FT_status = FT_SetBaudRate(FT_handle, BaudRate); //Sets FT232 to 9600 baud

    if(FT_status == FT_OK)             // Error Checking
    {
        qDebug() << "\t  Baudrate Set at bps: " << BaudRate;
    }
    else
        qDebug() << "\t  ERROR! in setting Baudrate";

    // Write 0xAA to the 8 bit data port
    FT_status = FT_Write( FT_handle,       // handle to the chip
                          &Buffer,         // address of the data
                          sizeof(Buffer),  // Size of the Buffer
                          &BytesWritten   // Number of bytes written
                        );
    if(FT_status == FT_OK)             // Error Checking
        qDebug() << "\t  0x" << Qt::hex << Buffer << " Written to 8 bit port @ bps: " << BaudRate;
    else
        qDebug() << "\t  ERROR! in Writing to Port";

    Mode = 0x00;         // Mask = 0x00 will reset the chip and-
                     // -get it out of Asynchronous mode

    FT_status = FT_SetBitMode(FT_handle, Mask, Mode); // Reset the chip

    if(FT_status == FT_OK)             // Error Checking
        {
            qDebug() << "\t  FT232 exited from Asynchronous Bit Bang Mode";
            qDebug() << "\t+-------------------------------------------------+";
        }

    FT_Close(FT_handle); // Close the Serial port connection
}


//  FTD2XX synchronous Bit Bang 模式测试
void MainWindow::on_FTD2XX_SyncBitBang_clicked()
{
    FT_STATUS ftStatus;
    FT_HANDLE ftHandle;
    DWORD w_data_len = 3; //write 3 bytes
    DWORD data_written; // number of bytes written
    UCHAR Mask = 0x0F; //Set D7-D4 input, D3-D0 output [input (0) and output (1)]
    UCHAR Mode = 0x04; //0x04 = synchronous bit-bang
    DWORD RxBytes; //number of bytes to be read from the device
    DWORD BytesReceived; //number of bytes read from the device
    byte RxBuffer[8]; //buffer to receive data from FT-X device
    byte data_out[8]; //buffer for data to be sent to FT-X device
    unsigned int i;

    ftStatus = FT_Open(0, &ftHandle);
    ftStatus |= FT_SetUSBParameters(ftHandle, 4096, 4096); // Set USB transfer sizes
    ftStatus |= FT_SetChars(ftHandle, false, 0, false, 0); // Disable event characters
    ftStatus |= FT_SetTimeouts(ftHandle, 5000, 5000); // Set read/write timeouts to 5 sec
    ftStatus |= FT_SetLatencyTimer(ftHandle, 16); // Latency timer at default 16ms
    ftStatus |= FT_SetFlowControl(ftHandle, FT_FLOW_NONE, 0x11, 0x13);
    ftStatus |= FT_SetBaudRate(ftHandle, 62500); //bit rate is x16 this value = 1M
    if (ftStatus != FT_OK) {
        qDebug() << "打开 ftdi 失败 ftStatus not ok. " <<  ftStatus; //check for error
    } else {
        Mode = FT_BITMODE_SYNC_BITBANG; //0x04 = synchronous bit-bang
        Mask = 0x0F; //Set D7-D4 input, D3-D0 output [input (0) and output (1)]

        //upper nibble is input pins, value (0 here) is 'don't care' for writing
        //lower nibble is output pins, D0 is sent 1 0 1, D1 is sent 0 1 1
        // D2 is sent 0 0 0 & D3 is sent 0 0 0
        data_out[0] = 0x01; // write 0x01 to pins: 0000 0001
        data_out[1] = 0x02; // write 0x02 to pins: 0000 0010
        data_out[2] = 0x03; // write 0x03 to pins: 0000 0011

        data_out[0] = byte(ui->vi1->value()); // write 0x01 to pins: 0000 0001
        data_out[1] = byte(ui->vi2->value()); // write 0x02 to pins: 0000 0010
        data_out[2] = byte(ui->vi3->value()); // write 0x03 to pins: 0000 0011

        //enter synchronous bit-bang mode
        ftStatus = FT_SetBitMode(ftHandle, Mask, Mode);
        //write data to pins
        ftStatus = FT_Write(ftHandle, data_out, w_data_len, &data_written);
        do{
            //Sleep(1000);
            FT_GetQueueStatus(ftHandle, &RxBytes);
            //Sleep(10); //delay
        } while (RxBytes != (w_data_len)); //check for all bytes to be returned
        //once bytes are received, use FT_Read to place in RxBuffer
        if (RxBytes > 0) {
            ftStatus = FT_Read(ftHandle, RxBuffer, RxBytes, &BytesReceived);
            qDebug() << BytesReceived << "bytes received:";
            for (i = 0;i < BytesReceived;i++)
                qDebug() << "RxBuffer :" << i << Qt::hex << RxBuffer[i] << "\t" << Qt::bin << RxBuffer[i];
            //
            //After enumeration, if device in loopback mode, the RxBuffer values will be
            // RxBuffer 0 = 0x00 (read prior to write of first byte)
            // RxBuffer 1 = 0x11 (read after 1st byte written)
            // RxBuffer 2 = 0x22 (read after 2nd byte written)
        }

        Mode = 0x00; //reset bit mode - pins return to values before setbitmode
        ftStatus = FT_SetBitMode(ftHandle, Mask, Mode);
    }

    ftStatus = FT_Close(ftHandle); //close device
}


//  send data in UART use ftd2XX lib. 使用 FTD2XX 进行串口数据发送
void MainWindow::on_pbSerPortWrite_clicked()
{
    FT_HANDLE ft_handle;               		// Declaring the handle to the chip
    FT_STATUS ft_status;               		// Variable for storing status of the operation
    DWORD     BaudRate;                		// Variable to hold the baud rate
    char      TxByte;                       // Variable to hold the byte to be tx'ed
    DWORD     NoOfBytesWritten = 0;         // No of Bytes written to the port


    int devNum = getFTnum();
    if(devNum <0)
        return;

    //-------------------------------  Opening the Connection to the chip -----------------------------------//

    qDebug() << Qt::endl;
    ft_status = FT_Open(devNum, &ft_handle); 		// Opening a connection to the connected FT232RL chip

    if(ft_status == FT_OK)             		// Error checking for FT_Open()
        qDebug() << "\tConnection to the chip opened successfully";
    else
        qDebug() << "\terror in opening connection, Chip not connected or loose cable";

    //--------------------------------------------------------------------------------------------------------//


    //-------------------------------  Setting the baud rate of the chip -------------------------------------//

    BaudRate = 9600;                   		// Set BaudRate = 9600
    FT_SetBaudRate(ft_handle, BaudRate);		// Setting the baudrate for the chip for 9600bps

    if(ft_status == FT_OK)             		//Error checking for FT_SetBaudRate()
        qDebug() << "\tBaud rate set to " << BaudRate;
    else
        qDebug() << "\tError in setting baud rate";



    //--------------------------- Setting Data bits, Stop bits, Parity Bits ------------------------------------//

    ft_status = FT_SetDataCharacteristics( ft_handle,  		// Handle of the chip(FT232)
                                           FT_BITS_8,        // No of Data bits = 8
                                           FT_STOP_BITS_1,   // No of Stop Bits = 1
                                           FT_PARITY_NONE	// Parity = NONE
                                          );
    if(ft_status == FT_OK)
        qDebug() << "\tFormat-> 8 DataBits, No Parity, 1 Stop Bit (8N1)";
    else
        qDebug() << "\tError in setting Data Format ";


    //--------------------------------- Setting Flow Control bits -------------------------------------------//

    ft_status = FT_SetFlowControl( ft_handle, 		// Handle of the chip(FT232)
                                   FT_FLOW_NONE,     // No Flow control
                                   NULL,             // XON  = Null since no Flow control
                                   NULL             // XOFF = Null since no Flow control
                                  );

    if(ft_status == FT_OK)
        qDebug() << "\tFlow Control = None ";
    else
        qDebug() << "\tError in setting Flow Control ";


    //-------------------------------  Writing a byte to serial port -----------------------------------------//
    char chs[] = "Hello, FT. \t\tWrite to serial port.\n";
    TxByte = 'A';

    ft_status = FT_Write( ft_handle,          // Handle to the chip
                          &chs,            // Variable Containing the byte to be Txed
                          sizeof(chs)-1,     // sizeof(TxByte) = char
                          &NoOfBytesWritten  // No of Bytes written to the port
                          );

    if(ft_status == FT_OK)             		 // Error checking for FT_Writee()
        qDebug() << "\t" << chs << "written to the serial port at " << BaudRate << " bps";
    else
        qDebug() << "\tError in writing to port";
    //--------------------------------------------------------------------------------------------------------//


    FT_Close(ft_handle);                      // Closing the handle to the chip
}
