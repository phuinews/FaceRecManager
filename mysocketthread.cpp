#include "mysocketthread.h"

#include"string.h"

#include <QDebug>
#include <QDataStream>
#include <QMessageBox>

#define TIMEOUT 5*1000
#define TCPPORT 43001


MySocketThread::MySocketThread(QObject *parent,QString deviceIP)
        :QThread(parent)
{
    m_deviceIP=deviceIP;
    m_isStart=true;
    m_blockSize=0;


}


MySocketThread::~MySocketThread()
{
    m_socket->close();
    m_socket->deleteLater();

}




void MySocketThread::run()
{
    m_socket=new QTcpSocket(this);
    m_socket->abort();
    m_socket->connectToHost(m_deviceIP,TCPPORT);
    /*
    if(!m_socket->waitForConnected(TIMEOUT))
    {
        QMessageBox::information(this,"notice","设备连接失败!");
        return;
    }
    */

    connect(m_socket, SIGNAL(readyRead()), this, SLOT(serverMsgRead()));
    connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(linkerrorHandle(QAbstractSocket::SocketError)));

    QTimer *timer = new QTimer(this);
    timer->setInterval(TIMEOUT);
    connect(timer, SIGNAL(timeout()),this,SLOT(HeartBeat()) );
    timer->start();
    m_heartTimer = new QTimer(this);
    connect(m_heartTimer, SIGNAL(timeout()),this,SLOT(unconnected()) );
    m_heartTimer->start(4*TIMEOUT);

    //QMessageBox::information(this,"notice","设备连接成功!");


}

void MySocketThread::HeartBeat()
{
    QByteArray outBlock;
    QDataStream stream(&outBlock, QIODevice::ReadWrite);
    stream.setByteOrder(QDataStream::LittleEndian);
    /*
    Bytes.resize(24);
    Bytes.replace(0,4,"\xAA\x55\xAA\x55");
    Bytes.replace(4,4,"\x10\x00\x00\x00");
    Bytes.replace(8,16,"\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA");
    */

    QByteArray head("\xAA\x55\xAA\x55",4);
    int length=16;
    QByteArray heart("\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA",16);
    stream.writeRawData(head.data(), head.size());
    stream<<length;
    stream.writeRawData(heart.data(), heart.size());

    //QByteArray BytesofL;
    //memcpy(BytesofL.data(),&length,sizeof(length));
    //Bytes.insert(4,BytesofL);
    //Bytes.insert(8,"\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA");


    m_socket->write(outBlock);
}

void MySocketThread::serverMsgRead()
{
    /*
    if(m_isStart)
    {
        m_isStart=false;
        QByteArray head=m_socket->read(4);
        if(!((head[0]==0xAA)&&(head[1]==0x55)&&(head[2]==0xAA)&&(head[3]==0x55)))
        {
           m_isStart=true;
           return;
        }

    }
    */

    QTcpSocket *tcpSocket = (QTcpSocket*)sender();
    QDataStream msgStream(tcpSocket);
    msgStream.setByteOrder(QDataStream::LittleEndian);


    while(true)
    {
            if (!m_blockSize) {
                if (tcpSocket->bytesAvailable() < 4+4 ) { // are size data available
                    break;
                }

                QByteArray head;
                head.resize(4);
                //msgStream >> head;
                msgStream.readRawData(head.data(), head.size());

                if(!((head.data()[0]=='\xAA')&&(head.data()[1]=='\x55')&&(head.data()[2]=='\xAA')&&(head.data()[3]=='\x55')))//判定包头
                    break;

                msgStream>>m_blockSize;

            }

            if (tcpSocket->bytesAvailable() < m_blockSize) {
                break;
            }


            QByteArray msgBody;
            msgBody.resize(m_blockSize);
            msgStream.readRawData(msgBody.data(),m_blockSize);
            if(m_blockSize==16)
            {
                if(msgBody==QByteArray(16,'\xAA'))//如果接收到的是心跳包,计时器重新开始计时
                {
                    m_heartTimer->start(4*TIMEOUT);
                    m_blockSize = 0;
                    break;
                }

            }
            m_blockSize = 0;

            emit sendMsgRead(msgBody,m_deviceIP);
    }


}

void MySocketThread::unconnected()
{
    emit sendUnconnected(m_deviceIP);
}

void MySocketThread::linkerrorHandle(QAbstractSocket::SocketError error)
{
    //qDebug()<<m_socket->errorString();
    //QMessageBox::information(this,"notice",QString::number(error,10));
    emit sendUnconnected(m_deviceIP);
}

