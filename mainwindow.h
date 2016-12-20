#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QTreeWidgetItem>
#include <QtSql>
#include <QSplitter>
#include <QLabel>

//#include "adddevice.h"
#include "facemanager.h"
#include "devicemanager.h"
#include "mysocketthread.h"
#include "accessrecord.h"
#include "videostream.h"


#include <QHostAddress>
#include <QString>
#include <QTcpSocket>

#include <QAbstractSocket>
#include <QHostAddress>
#include <QMap>

#define TCPPORT 43001
#define TIMEOUT 4*5*1000

typedef unsigned long DWORD;
const DWORD PACKET_SETTING_REQ = 0x00001101;
const DWORD PACKET_SETTING = 0x00001102;
const DWORD PACKET_SETTING_DISPATCH = 0x00001103;
const DWORD PACKET_REGINFO_COLLECT_REQUEST = 0x00001201;	//人脸注册信息收集请求(来自后台管理中心的请求) AA 55 AA 55 04 00 00 00 01 12 00 00
const DWORD PACKET_REGINFO_COLLECT_ANSWER  = 0x00001202;	//人脸注册信息收集应答(前端应答会发送所有人员的注册信息,分多包发送,每个包一条注册信息)
const DWORD PACKET_REGINFO_DISPATCH = 0x00001203;			//人脸注册信息下发(来自后台管理中心,下发所有人员的注册信息，分多包发送，每个包一条注册信息)
const DWORD PACKET_ACCESS_REC_REQ = 0x00001301;
const DWORD PACKET_ACCESS_REC_REQ_ANSWER = 0x00001302;
const DWORD PACKET_STREAM_REQ = 0x00001401;
const DWORD PACKET_STREAM = 0x00001402;
const DWORD PACKET_STREAM_STOP = 0x00001403;
const DWORD PACKET_TIMESYNC = 0x00001501;
const DWORD PACKET_ALARM_NOTICE = 0x00001601;
const DWORD PACKET_DOOR_OPEN = 0x00001701;


typedef struct tagSETTING_REQ
{
    DWORD dwMsgType; //0x1101
    int Size() const
    {
    return sizeof(*this);
    }
}SETTING_REQ, *LP_SETTING_REQ;

typedef struct tagSETTING
{
    DWORD dwMsgType; //0x1102
    char strEntry[20]; //门禁解锁密码(最大长度:10 位)
    char strEM[20]; //工程设置密码(最大长度:10 位)
    char strExit[20]; //系统退出密码(最大长度:10 位)
    DWORD dwThreshold; //比对相似度阈值: (0,1000)
    DWORD dwDcDeviceSN; //门控设备序列号
    char strDcDeviceIP[16]; //门控设备 IP
    DWORD dwDcDevicePort; //门控设备端口号
    int Size() const
    {
        return sizeof(*this);
    }
}SETTING, *LP_SETTING;

typedef struct tagREGINFO_REQ
{
    DWORD	dwMsgType;

    int Size() const
    {
        return sizeof(*this);
    }
}REGINFO_REQ, *LP_REGINFO_REQ;

typedef struct tagREGINFO
{
    DWORD	dwMsgType;
    char	strName[16];
    char	strGender[8];
    char	strDepartment[24];
    DWORD	dwPicLen;

    char* pPic() const
    {
        char*  pic = ((char*)this + sizeof(*this));
        return pic;
    }

    int Size() const
    {
        return sizeof(*this) + dwPicLen;
    }
}REGINFO, *LP_REGINFO;

typedef struct tagACCESS_REC_REQ
{
    DWORD dwMsgType;
    char strDateTimeBegin[24]; //YYYY-mm-dd HH:MM:SS, 例如:2016-12-08 14:00:00
    char strDateTimeEnd[24]; //同上
    int Size() const
    {
    return sizeof(*this);
    }
}ACCESS_REC_REQ, *LP_ACCESS_REC_REQ;

typedef struct tagACCESS_REC
{
    DWORD dwMsgType;
    char strName[16]; //姓名
    char strGender[8]; //性别
    char strDepartment[24]; //部门
    DWORD dwSimilarity; //相似度: (0,1000)
    char strDateTime[24]; //YYYY-mm-dd HH:MM:SS, 例如:2016-12-08 14:00:00
    DWORD dwPicLen;
    char* pPic() const
    {
    char* pic = ((char*)this + sizeof(*this));
    return pic;
    }
    int Size() const
    {
        return sizeof(*this) + dwPicLen;
    }
}ACCESS_REC, *LP_ACCESS_REC;

typedef struct tagSTREAM_REQ
{

    DWORD dwMsgType;
    int Size() const
    {
        return sizeof(*this);
    }
}STREAM_REQ, *LP_STREAM_REQ;

typedef struct tagSTREAM
{
    DWORD dwMsgType;
    DWORD dwPicLen;
    char* pPic() const
    {
        char* pic = ((char*)this + sizeof(*this));
        return pic;
    }
    size_t Size() const
    {
        return sizeof(*this) + dwPicLen;
    }
}STREAM, *LP_STREAM;

typedef struct tagSTREAM_STOP
{
    DWORD dwMsgType;
    int Size() const
    {
        return sizeof(*this);
    }
}STREAM_STOP, *LP_STREAM_STOP;

typedef struct tagTIMESYNC
{
    DWORD dwMsgType;
    char strSyncDateTime[24]; //YYYY-mm-dd HH:MM:SS, 例如:2016-12-08 14:00:00
    int Size() const
    {
        return sizeof(*this);
    }
}TIMESYNC, *LP_TIMESYNC;

typedef struct tagALARM_NOTICE
{
    DWORD dwMsgType;
    int Size() const
    {
        return sizeof(*this);
    }
}ALARM_NOTICE, *LP_ALARM_NOTICE;

typedef struct tagDOOR_OPEN
{
    DWORD dwMsgType;
    int Size() const
    {
        return sizeof(*this);
    }
}DOOR_OPEN, *LP_DOOR_OPEN;







namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    //friend class ADDDevice;
    friend class FaceManager;
    friend class DeviceManager;
    friend class AccessRecord;
    friend class VideoStream;

public:
    //static QTreeWidget* DeviceListTree;

signals:
    void sendDeviceIP(QString);
    void accessRecordReceive(QStringList);
    void streamAnswer(QByteArray);
    void regInfoAnswer(QByteArray);

    void recentRecordChanged();//更新实时监控窗口的最新通行记录

private slots:

    void on_video_triggered();
    void on_AccessRecord_triggered();
    void on_DeviceSet_triggered();
    void on_FaceLib_triggered();

    //void on_addDevice_clicked();
    //void on_deleteDevice_clicked();

    //void receiveDevice(QStringList DeviceData);
    void receiveDeviceIP(QString DeviceIP);
    void receiveDeviceIPForDel(QString DeviceIP);

    //void serverMsgRead();
    //void linkerrorHandle(QAbstractSocket::SocketError);

    void serverMsgProcess(QByteArray,QString);
    void serverUnconnectedHandle(QString);
    void faceLibDownloadReq(QString);
    void faceLibUploadReq(QString);

    void accessRecordSearch(QString,QStringList);
    //void recentRecordSearch(QString,QStringList);

    //实时视频处理
    void streamReq(QString);
    void streamStop(QString);

    void recordByName(QString);//根据名单中选定的名字查询通行记录


private:
    //界面初始化
    void Init();
    void DeviceListUpdate();
    void DeviceListCreate();

    //初始化与前端平板的tcp连接
    void socketInit();

    //处理与前端平板的通讯
    //void regInfoAnswerHandle(QByteArray,QString);
    void accessRecAnswerHandle(QByteArray,QString);
    void settingHandle(QByteArray,QString);
    void doorBellHandle(QByteArray,QString);


private:

    Ui::MainWindow *ui;
    //ADDDevice* addADDDevice;

    QWidget* m_workWidget;
    //AccessRecord* m_accessRecord;

    //QPushButton* addDevice;
    //QPushButton* deleteDevice;
    QTreeWidget* DeviceListTree;
    QSplitter* workSplitter;
    QSplitter* mainSplitter;

    QSqlDatabase db;
    QMap<QString,MySocketThread*> m_sockets;

    bool m_recentRecord;



    /*
    QHostAddress currentHAddr;
    qint16 HtcpPort;
    QTcpSocket *HtcpClient;
    QHostAddress currentDAddr;
    qint16 DtcpPort;
    QTcpSocket *DtcpClient;
    */

};

QStringList imageNameToFaceInfo(QString imageName);

#endif // MAINWINDOW_H
