#ifndef MYSOCKETTHREAD_H
#define MYSOCKETTHREAD_H

#include <QTcpSocket>
#include <QString>
#include <QByteArray>
#include <QTimer>
#include <QThread>


#define TCPPORT 43001



class MySocketThread: public QThread
{
    Q_OBJECT

public:

    MySocketThread(QObject *parent = 0,QString deviceIp="");
   ~MySocketThread();

    void setMessage(const QString &msg);
    void stop();

     virtual void run();//需要在主窗口中调用


public:
    QTcpSocket* m_socket;

private:

    QString m_deviceIP;
    QTimer* m_heartTimer;

    bool m_isStart;
    QByteArray inBlock;
    quint32 m_blockSize;




signals:
    void sendMsgRead(QByteArray,QString);
    void sendUnconnected(QString);


private slots:
    void HeartBeat();
    void serverMsgRead();
    void unconnected();
    void linkerrorHandle(QAbstractSocket::SocketError);


};

#endif // MYSOCKETTHREAD_H
