#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QThread;
class FT2232Drv;


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    // open or close shutter 开关光闸信号
    void shutterOpenClose(bool actOpen=true);

public slots:
    // receive Open device state 收到 打开设备状态 的槽函数
    void onOpenState(bool bOpenState);

    // receive a data [*] 收到 IO 数据的槽函数 [重点处理位置]
    void onRecvData(unsigned char recDat);

    // status infomation 收到 状态信息的槽函数
    void onStatusInfo(QString str,int status);

private slots:
    void on_pbShutterOpen_clicked();
    void on_pbShutterClose_clicked();


// ========= test ftdi device in MainWindow ===========
// ========= 以下是在 MainWindow 中进行对 ftdi 设备测试部分 不必加入到程序中 ===========
private slots:
    void on_pbFtInfo_clicked();
    void on_FTD2XX_AsyncBitBang_clicked();
    void on_FTD2XX_SyncBitBang_clicked();
    void on_pbSerPortWrite_clicked();

private:
    int getFTnum();
//  ====================================================================

private:
    Ui::MainWindow *ui;
    FT2232Drv *pWorker;
    QThread *pThread;

};
#endif // MAINWINDOW_H
