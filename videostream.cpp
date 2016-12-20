#include "videostream.h"
#include "ui_videostream.h"
#include "mainwindow.h"

#include<QMessageBox>
#include <QDateTime>
#include <QTest>

VideoStream::VideoStream(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VideoStream)
{
    ui->setupUi(this);

    m_parent= (MainWindow*) parentWidget();
    QTreeWidgetItem* item=m_parent->DeviceListTree->currentItem();

    //由于经常要用到这几个参数，因此设为成员变量，在这里初始化
    m_area=item->parent()->text(0);
    m_device=item->text(0);
    QSqlQuery query;
    QString querySql=QString("SELECT ip FROM deviceTable where area='%1' and name='%2' "
                                     ).arg(m_area).arg(m_device);
    query.exec(querySql);
    if(query.next())
        m_deviceIP= query.value(0).toString();


    ui->videoLable->setScaledContents(true);
    ui->videoLable->setStyleSheet("border: 1px solid blue");
    ui->accessImageL->setScaledContents(true);
    ui->faceLibImageL->setScaledContents(true);

    recentRecordTree= new QTreeWidget(this);
    splitterInit();

    emit streamReq(m_deviceIP);
    //recentRecordReq();
    //QTest::qWait(50);

    connect(recentRecordTree, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(itemClickedHandle(QTreeWidgetItem*,int)));




}

VideoStream::~VideoStream()
{
    emit streamStop(m_deviceIP);
    delete ui;
}


void VideoStream::on_streamReqBtn_clicked()
{
    emit streamReq(m_deviceIP);
}



void VideoStream::on_streamStopBtn_clicked()
{
    emit streamStop(m_deviceIP);
}

void VideoStream::on_doorOpenBtn_clicked()
{
    QMap<QString,MySocketThread*>::const_iterator iter = m_parent->m_sockets.find(m_deviceIP);
    if(iter!=m_parent->m_sockets.end())//如果有连接，则给前端发送TCP开门包
    {
        QByteArray outBlock;
        QDataStream stream(&outBlock, QIODevice::ReadWrite);
        stream.setByteOrder(QDataStream::LittleEndian);

        QByteArray head("\xAA\x55\xAA\x55",4);

        DOOR_OPEN doorOpen;
        doorOpen.dwMsgType=PACKET_DOOR_OPEN;


        int length=doorOpen.Size();

        stream.writeRawData(head.data(), head.size());
        stream<<length;
        stream.writeRawData((char*)&doorOpen,length);

        m_parent->m_sockets[m_deviceIP]->m_socket->write(outBlock);

    }
    else//否则直接给前端门控设备发送UDP包
    {

    }

}

void VideoStream::streamAnswer(QByteArray msgBody)
{

    QPixmap pixmap;
    pixmap.loadFromData(msgBody.right(msgBody.size()-sizeof(STREAM)));
    ui->videoLable->setPixmap(pixmap);

}

void VideoStream::splitterInit()
{
    //emit streamReq(m_deviceIP);//不需要按钮，直接发送监控请求

    QSplitter* headSplitter = new QSplitter(Qt::Vertical);
    headSplitter->addWidget(new QLabel("",this));

    QSplitter* BtnSplitter = new QSplitter(Qt::Horizontal);
    ui->streamReqBtn->resize(122,26);
    ui->streamStopBtn->resize(122,26);
    ui->doorOpenBtn->resize(122,26);
    BtnSplitter->addWidget(new QLabel("",this));
    BtnSplitter->addWidget(ui->streamReqBtn);
    BtnSplitter->addWidget(ui->streamStopBtn);
    BtnSplitter->addWidget(ui->doorOpenBtn);
    //BtnSplitter->addWidget(ui->recordBtn);
    BtnSplitter->addWidget(new QLabel("",this));
    BtnSplitter->setStretchFactor(4, 1);

    headSplitter->addWidget(BtnSplitter);

    QSplitter* videoSplitter = new QSplitter(Qt::Vertical);

    //mainSplitter->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    //视频和通行记录表
    videoSplitter->addWidget(headSplitter);
    videoSplitter->addWidget(ui->videoLable);
    recentRecordTreeInit();
    videoSplitter->addWidget(recentRecordTree);
    videoSplitter->setStretchFactor(1, 3);
    videoSplitter->setStretchFactor(2, 1);
    //QList<int> list;
    //videoSplitter->setSizes(list<<1<<200<<3);

    //图片显示
    QSplitter* iamgeSplitter = new QSplitter(Qt::Vertical);
    ui->accessImageL->setFixedSize(331,351);
    ui->faceLibImageL->setFixedSize(331,351);
    iamgeSplitter->addWidget(ui->accessImageL);
    QLabel* lableAid2=new QLabel("",this);
    lableAid2->setFixedSize(331,28);
    iamgeSplitter->addWidget(lableAid2);
    iamgeSplitter->addWidget(ui->faceLibImageL);
    iamgeSplitter->addWidget(new QLabel("",this));
    iamgeSplitter->setStretchFactor(3, 1);

    QSplitter* mainSplitter = new QSplitter(Qt::Horizontal);
    mainSplitter->addWidget(new QLabel("",this));
    mainSplitter->addWidget(videoSplitter);
    mainSplitter->addWidget(new QLabel("",this));
    mainSplitter->addWidget(iamgeSplitter);
    mainSplitter->addWidget(new QLabel("",this));
    mainSplitter->setStretchFactor(1, 1);

    QSplitter* allSplitter = new QSplitter(Qt::Vertical);
    allSplitter->addWidget(headSplitter);
    allSplitter->addWidget(mainSplitter);
    allSplitter->setStretchFactor(1, 1);

    setCentralWidget(allSplitter);

}

void VideoStream::recentRecordTreeInit()
{
    recentRecordTree->setColumnCount(5);
    QStringList header;
    header<<"姓名"<<"性别"<<"职务"<<"相似度"<<"通行时间";
    recentRecordTree->setHeaderLabels(header);
}

void VideoStream::recentRecordTreeUpdate()
{
    //先清除
    while(recentRecordTree->topLevelItemCount())
    {
        QTreeWidgetItem* leafItem = recentRecordTree->takeTopLevelItem(0);
        delete leafItem;
    }

    QDateTime startTime=QDateTime::currentDateTime();//获取系统现在的时间
    startTime.setTime(QTime(0,0,0));
    QDateTime endTime=QDateTime::currentDateTime().addSecs(600);//考虑到时间可能不同步，调整查找时段

    QSqlQuery query;
    QString search=QString("SELECT name,gender,post,similarity,time FROM accessRecord WHERE area='%1' and device='%2' "
                   "and time>'%3' and time<'%4' ORDER BY time DESC LIMIT 20 ").arg(m_area).arg(m_device)
                   .arg(startTime.toString("yyyy-MM-dd hh:mm:ss")).arg(endTime.toString("yyyy-MM-dd hh:mm:ss"));
    query.exec(search);
    while((query.next()))
    {
        QString name= query.value(0).toString();
        QString gender= query.value(1).toString();
        QString post= query.value(2).toString();
        QString similarity= query.value(3).toString();
        QString time= query.value(4).toString();

        QStringList accessInfoList;
        accessInfoList<<name<<gender<<post<<similarity<<time;
        QTreeWidgetItem *leaf = new QTreeWidgetItem(recentRecordTree, accessInfoList);
        recentRecordTree->insertTopLevelItem(0,leaf);

    }

}

/*
void VideoStream::recentRecordReq()
{
    QMap<QString,MySocketThread*>::const_iterator iter = m_parent->m_sockets.find(m_deviceIP);
    if(iter!=m_parent->m_sockets.end())//如果有连接则联网查询
    {
        QDateTime endTime=QDateTime::currentDateTime();//获取系统现在的时间
        QDateTime startTime=endTime;
        startTime.setTime(QTime(0,0,0));

        QStringList times;
        times<<startTime.toString("yyyy-MM-dd hh:mm:ss")<<endTime.toString("yyyy-MM-dd hh:mm:ss");

        emit recordReq(m_deviceIP,times);
    }
}
*/

void VideoStream::itemClickedHandle(QTreeWidgetItem* item,int row)
{
    QString name=item->text(0);
    QString time=item->text(4);

    QSqlQuery query;
    QString querySql=QString("SELECT image FROM accessRecord where area='%1' and device='%2' and name='%3' and time='%4' "
                             ).arg(m_area).arg(m_device).arg(name).arg(time);
    query.exec(querySql);
    if(query.next())
    {
        QString imagepath=query.value(0).toString();
        QPixmap pix;
        pix.load(imagepath);

        ui->accessImageL->setPixmap(pix);
    }

    querySql=QString("SELECT image FROM faceLib where area='%1' and device='%2' and name='%3' "
                                 ).arg(m_area).arg(m_device).arg(name);
    query.exec(querySql);
    if(query.next())
    {
        QString imagepath=query.value(0).toString();
        QPixmap pix;
        pix.load(imagepath);

        ui->faceLibImageL->setPixmap(pix);

        /*
        QPixmap pix;
        pix.load(imagepath);
        img1->scaled(image1Label->size(), Qt::KeepAspectRatio);
        ui->faceLibImageL->setPixmap(pix);
        ui->faceLibImageL->resize(pix.width(),pix.height());
        */
    }
    else
        ui->faceLibImageL->clear();
}


/*
void VideoStream::on_recordBtn_clicked()
{
    QMap<QString,MySocketThread*>::const_iterator iter = m_parent->m_sockets.find(m_deviceIP);
    if(iter!=m_parent->m_sockets.end())//如果有连接则联网查询
    {
        QDateTime endTime=QDateTime::currentDateTime();//获取系统现在的时间
        QDateTime startTime=endTime;
        startTime.setTime(QTime(0,0,0));

        QStringList times;
        times<<startTime.toString("yyyy-MM-dd hh:mm:ss")<<endTime.toString("yyyy-MM-dd hh:mm:ss");

        emit recordReq(m_deviceIP,times);
    }
}
*/

void VideoStream::initReq()
{
    //请求视频流
    emit streamReq(m_deviceIP);

    //请求最近通行记录
    QMap<QString,MySocketThread*>::const_iterator iter = m_parent->m_sockets.find(m_deviceIP);
    if(iter!=m_parent->m_sockets.end())//如果有连接则联网查询
    {
        QDateTime endTime=QDateTime::currentDateTime();//获取系统现在的时间
        QDateTime startTime=endTime;
        startTime.setTime(QTime(0,0,0));

        QStringList times;
        times<<startTime.toString("yyyy-MM-dd hh:mm:ss")<<endTime.toString("yyyy-MM-dd hh:mm:ss");

        emit recordReq(m_deviceIP,times);
    }
}
