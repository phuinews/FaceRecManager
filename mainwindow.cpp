#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include "adddevice.h"
#include "connectdb.h"

#include <qdebug.h>
#include <QList>
#include <QTextEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QTreeWidget>
#include <QTest>


#include <QTimer>



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_workWidget=NULL;
    m_recentRecord=false;

    db=QSqlDatabase::addDatabase("QSQLITE");
    db.setHostName("localhost");
    db.setDatabaseName("data.db");
    db.setUserName("root");
    db.setPassword("call818");
    if(!db.open())
    {
        QMessageBox::critical(NULL, QObject::tr("Collection"), QObject::tr("failed to connect to database!"));
    }

    QSqlQuery query;
    query.exec("create table if not exists deviceTable(id INTEGER PRIMARY KEY autoincrement, "
               "area nvarchar(20), name nvarchar(20),ip nvarchar(40),doorPwd nvarchar(20),settingPwd nvarchar(20),"
               "quitPwd nvarchar(20),threshold nvarchar(8),doorCtlNo nvarchar(20),doorCtlIP nvarchar(40),doorCtlPort nvarchar(10))");
    query.exec("create table if not exists faceLib(id INTEGER PRIMARY KEY autoincrement, "
               "area nvarchar(50), device nvarchar(50),name nvarchar(8),gender nvarchar(2),"
               "post nvarchar(12),image nvarchar(200),signtime nvarchar(50))");
    query.exec("create table if not exists accessRecord(id INTEGER PRIMARY KEY autoincrement, "
               "area nvarchar(50), device nvarchar(50),name nvarchar(8),gender nvarchar(2),"
               "post nvarchar(12),similarity nvarchar(8),image nvarchar(200),time nvarchar(50))");
    //query.exec("insert into faceLib(area,device,name,gender,post,signtime)"
     //          "values('区域1','设备1','张三','男','工程师','2016-11-29 10:10:10' )");

   // query.exec("insert into faceLib(area,device,name,gender,post,signtime)"
     //          "values('区域1','设备1','Griffin','女','业务经理','2016-11-29 11:10:10' )");






    Init();
    socketInit();


    connect(this,SIGNAL(sendDeviceIP(QString)),this,SLOT(receiveDeviceIP(QString)));//中断后重新连接前端设备


}



MainWindow::~MainWindow()
{
    delete ui;
    db.close();

}

/*
void MainWindow::on_DoorOpenBtn_clicked()
{
    DtcpClient->connectToHost(currentDAddr,DtcpPort);
}
*/

void MainWindow::on_video_triggered()
{
    ui->AccessRecord->setChecked(false);
    ui->DeviceSet->setChecked(false);
    ui->FaceLib->setChecked(false);

    m_recentRecord=true;

    if(DeviceListTree->currentItem()&&(DeviceListTree->currentItem()->childCount()==0))//判断当前是否选中设备
    {
        if(m_workWidget)
            delete m_workWidget;
        m_workWidget=new QWidget(this);
        workSplitter->insertWidget(0,m_workWidget);

        m_workWidget=new VideoStream(this);
        connect(m_workWidget,SIGNAL(streamReq(QString)),this,SLOT(streamReq(QString)));
        connect(m_workWidget,SIGNAL(streamStop(QString)),this,SLOT(streamStop(QString)));
        connect(m_workWidget,SIGNAL(recordReq(QString,QStringList)),this,SLOT(accessRecordSearch(QString,QStringList)));
        connect(this,SIGNAL(streamAnswer(QByteArray)),m_workWidget,SLOT(streamAnswer(QByteArray)));
        connect(this,SIGNAL(recentRecordChanged()),m_workWidget,SLOT(recentRecordTreeUpdate()));//更新实时监控窗口的最新通行记录
        dynamic_cast<VideoStream *>(m_workWidget)->initReq();


        //这一段将工作区域的当前显示清空
        //清空前需要加上判断，不然会导致非法访问悬垂指针
        while(workSplitter->count()>0)
        {
            QWidget *old = workSplitter->widget(0);// deparenting removes the widget from the gui
            old->hide();
            old->setParent(NULL);//delete current widget on index 0
            old->deleteLater();
        }
        workSplitter->insertWidget(0,m_workWidget);
        workSplitter->setStretchFactor(0, 1);

    }
    else
        QMessageBox::information(this,"notice","请先选择设备!");

}

void MainWindow::on_AccessRecord_triggered()
{
    ui->video->setChecked(false);
    ui->DeviceSet->setChecked(false);
    ui->FaceLib->setChecked(false);

    m_recentRecord=false;

    if(DeviceListTree->currentItem()&&(DeviceListTree->currentItem()->childCount()==0))//判断当前是否选中设备
    {
        if(m_workWidget)
            delete m_workWidget;
        m_workWidget=new QWidget(this);
        workSplitter->insertWidget(0,m_workWidget);

        m_workWidget=new AccessRecord(this);
        connect(m_workWidget,SIGNAL(accessRecordSearch(QString,QStringList)),this,SLOT(accessRecordSearch(QString,QStringList)));
        connect(this,SIGNAL(accessRecordReceive(QStringList)),m_workWidget,SLOT(accessRecordReceive(QStringList)));


        //这一段将工作区域的当前显示清空
        //清空前需要加上判断，不然会导致非法访问悬垂指针
        while(workSplitter->count()>0)
        {
            QWidget *old = workSplitter->widget(0);// deparenting removes the widget from the gui
            old->hide();
            old->setParent(NULL);//delete current widget on index 0
            old->deleteLater();
        }
        workSplitter->insertWidget(0,m_workWidget);
        workSplitter->setStretchFactor(0, 1);

    }
    else
        QMessageBox::information(this,"notice","请先选择设备!");


}

void MainWindow::on_DeviceSet_triggered()
{
    ui->AccessRecord->setChecked(false);
    ui->video->setChecked(false);
    ui->FaceLib->setChecked(false);

    m_recentRecord=false;

    if(m_workWidget)
        delete m_workWidget;
    m_workWidget=new QWidget(this);
    workSplitter->insertWidget(0,m_workWidget);

    m_workWidget=new DeviceManager(this);
    connect(m_workWidget,SIGNAL(sendData(QString)),this,SLOT(receiveDeviceIP(QString)));
    connect(m_workWidget,SIGNAL(sendDataForDel(QString)),this,SLOT(receiveDeviceIPForDel(QString deviceIP)));



    //这一段将工作区域的当前显示清空
    //清空前需要加上判断，不然会导致非法访问悬垂指针
    while(workSplitter->count()>0)
    {
        QWidget *old = workSplitter->widget(0);// deparenting removes the widget from the gui
        old->hide();
        old->setParent(NULL);//delete current widget on index 0
        old->deleteLater();
    }
    workSplitter->insertWidget(0,m_workWidget);
    workSplitter->setStretchFactor(0, 1);
    //m_workWidget->show();



}

void MainWindow::on_FaceLib_triggered()
{
    ui->AccessRecord->setChecked(false);
    ui->DeviceSet->setChecked(false);
    ui->video->setChecked(false);

    m_recentRecord=false;

    if(DeviceListTree->currentItem()&&(DeviceListTree->currentItem()->childCount()==0))//判断当前是否选中设备
    {
        if(m_workWidget)
            delete m_workWidget;
        m_workWidget=new QWidget(this);
        workSplitter->insertWidget(0,m_workWidget);

        m_workWidget=new FaceManager(this);
        connect(m_workWidget,SIGNAL(faceLibDownloadReq(QString)),this,SLOT(faceLibDownloadReq(QString)));
        connect(m_workWidget,SIGNAL(faceLibUploadReq(QString)),this,SLOT(faceLibUploadReq(QString)));
        connect(this,SIGNAL(regInfoAnswer(QByteArray)),m_workWidget,SLOT(regInfoAnswerHandle(QByteArray)));
        connect(m_workWidget,SIGNAL(recordName(QString)),this,SLOT(recordByName(QString)));




        //这一段将工作区域的当前显示清空
        //清空前需要加上判断，不然会导致非法访问悬垂指针
        while(workSplitter->count()>0)
        {
            QWidget *old = workSplitter->widget(0);// deparenting removes the widget from the gui
            old->hide();
            old->setParent(NULL);//delete current widget on index 0
            old->deleteLater();
        }
        workSplitter->insertWidget(0,m_workWidget);
        workSplitter->setStretchFactor(0, 1);
        //m_workWidget->show();

    }
    else
        QMessageBox::information(this,"notice","请先选择设备!");

}

/*
void MainWindow::on_addDevice_clicked()
{
   ADDDevice* addADDDevice=new ADDDevice(this);
   connect(addADDDevice,SIGNAL(sendData(QStringList)),this,SLOT(receiveDevice(QStringList)));
   addADDDevice->show();
}
*/
/*
void MainWindow::on_deleteDevice_clicked()
{
    QTreeWidgetItem * item=DeviceListTree->currentItem();
    if(!item)
        QMessageBox::information(this,"notice","没有设备被选中!");
    else if(item->childCount()==0)
    {
        QMessageBox::StandardButton del = QMessageBox::warning(NULL, "删除设备", "确定删除所选设备吗?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if(del == QMessageBox::Yes)
        {
            QString name=item->text(0);
            QString area=item->parent()->text(0);
            QSqlQuery query(db);
            QString delSql=QString("DELETE FROM deviceTable where area='%1' and name='%2' ").arg(area).arg(name);
            query.exec(delSql);
            if(item->parent()->childCount()==1)
                delete item->parent();
            else
                delete item;
        }
    }
    else
    {
        QMessageBox::StandardButton del = QMessageBox::warning(NULL, "删除区域", "确定删除该区域所有设备吗?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if(del == QMessageBox::Yes)
        {
            QSqlQuery query(db);
            QString area=item->text(0);
            int count = item->childCount();
            for(int i=0;i<count;++i)
            {
                QString name=item->child(i)->text(0);
                QString delSql=QString("DELETE FROM deviceTable where area='%1' and name='%2' ").arg(area).arg(name);
                query.exec(delSql);
            }
            delete item;
        }
    }

}
*/

void MainWindow::DeviceListUpdate()
{
    while(DeviceListTree->topLevelItemCount())
    {
        QTreeWidgetItem* item = DeviceListTree->takeTopLevelItem(0);
        delete item;
    }

    DeviceListCreate();
}


void MainWindow::DeviceListCreate()
{
    QSqlQuery query(db);
    query.exec("SELECT area, name FROM deviceTable");


    QMap<QString,QStringList> DeviceListMap;
    while (query.next())
    {
            QString area = query.value(0).toString();
            QString name = query.value(1).toString();
            if(!DeviceListMap.contains(area))
            {
                QStringList areaList;
                areaList<<name;
                DeviceListMap.insert(area,areaList);
            }
            else
            {
                QStringList areaList=DeviceListMap[area];
                areaList<<name;
                DeviceListMap[area]=areaList;
            }
    }

    QList<QTreeWidgetItem *> rootList;
    QMap<QString,QStringList>::Iterator iter;
    for(iter=DeviceListMap.begin();iter!=DeviceListMap.end();++iter)
    {
        QTreeWidgetItem *root = new QTreeWidgetItem(DeviceListTree,  QStringList(iter.key()));
        for(int j=0;j<(iter.value()).count();++j){
            QTreeWidgetItem *leaf = new QTreeWidgetItem(root,  QStringList((iter.value()).at(j)));
            root->addChild(leaf);
        }
        rootList<<root;

    }


    DeviceListTree->insertTopLevelItems(0, rootList);


}


void MainWindow::Init()
{

    DeviceListTree = new QTreeWidget(this);
    DeviceListTree->setColumnCount(1);
    DeviceListTree->setHeaderLabels(QStringList(QString("")));
    DeviceListCreate();


    QLabel* DeviceLabel=new QLabel("设备列表",this);
    DeviceLabel->resize(124,52);
    DeviceLabel->setAlignment(Qt::AlignCenter);
    //DeviceLabel->setStyleSheet("QLabel{border:2px solid blue;}");//设置边框宽度及颜色

    //设置字体大小
    QFont ft;
    ft.setPointSize(15);
    ft.setBold(true);
    DeviceLabel->setFont(ft);


    /*//设置背景颜色
    DeviceLabel->setAutoFillBackground(true);
    QPalette palette;
    palette.setColor(QPalette::Background, Qt::lightGray);
    palette.setColor(QPalette::WindowText, Qt::blue);
    DeviceLabel->setPalette(palette);
    */


    //设置按钮和字体大小
    //addDevice=new QPushButton("添加",this);
    //addDevice->resize(101,31);
    //QFont ft2;
    //ft2.setPointSize(12);
    //addDevice->setFont(ft2);
    //deleteDevice=new QPushButton("删除",this);
    //deleteDevice->resize(101,31);
    //deleteDevice->setFont(ft2);

    //QSplitter* BtnSplitter = new QSplitter(Qt::Horizontal,this);
    //BtnSplitter->addWidget(addDevice);
    //BtnSplitter->addWidget(deleteDevice);

    QSplitter* deviceTreeSplitter = new QSplitter(Qt::Vertical,this);
    deviceTreeSplitter->addWidget(DeviceLabel);
    deviceTreeSplitter->addWidget(DeviceListTree);
    deviceTreeSplitter->setStretchFactor(1, 1);

    deviceTreeSplitter->setAutoFillBackground(true);
    QPalette palette;
    palette.setColor(QPalette::Background, QColor(220,220,220));
    palette.setColor(QPalette::WindowText, Qt::blue);
    deviceTreeSplitter->setPalette(palette);


    workSplitter=new QSplitter(Qt::Horizontal,this);

    QWidget* workWidget= new QWidget(this);
    /*这段是手动生成人员列表窗口
    DeviceListTree->setCurrentItem(DeviceListTree->topLevelItem(3)->child(1));
    m_workWidget=new FaceManager(this);
    workSplitter->addWidget(m_workWidget);
    */
    workSplitter->addWidget(workWidget);


    mainSplitter = new QSplitter(Qt::Horizontal,this);
    mainSplitter->addWidget(deviceTreeSplitter);
    mainSplitter->addWidget(workSplitter);
    mainSplitter->setStretchFactor(1, 1);

        //mainSplitter->setStyleSheet("QSplitter::handle { background-color: blue }"); //设置分界线的样式
    mainSplitter->setHandleWidth(0);

    setCentralWidget(mainSplitter);

}

void MainWindow::socketInit()
{
    QSqlQuery query;
    QString querySql=QString("SELECT area, name,ip FROM deviceTable");
    query.exec(querySql);
    while(query.next())
    {
        QString area= query.value(0).toString();
        QString device= query.value(1).toString();
        QString deviceIP= query.value(2).toString();

        if(deviceIP.isEmpty())
            continue;

        MySocketThread* mysocket=new MySocketThread(this,deviceIP);

        m_sockets.insert(deviceIP,mysocket);
        connect(mysocket, SIGNAL(sendMsgRead(QByteArray,QString)), this, SLOT(serverMsgProcess(QByteArray,QString)));
        connect(mysocket, SIGNAL(sendUnconnected(QString)), this, SLOT(serverUnconnectedHandle(QString)));
        mysocket->run();
        //QMessageBox::information(this,"notice",QString("已对%1%2发起连接申请!").arg(area).arg(device));
    }

}

/*
void MainWindow::receiveDevice(QStringList DeviceData)
{
    QString area=DeviceData.at(0);
    QString name=DeviceData.at(1);
    QString ip=DeviceData.at(2);

    QSqlQuery query(db);
    QString querySql=QString("SELECT area, name FROM deviceTable where (area='%1' and name='%2') or ip='%3' "
                             ).arg(area).arg(name).arg(ip);
    query.exec(querySql);
    if(query.next())
        QMessageBox::critical(this,"fail","设备已存在或IP地址冲突!");
    else
    {
        QString insertSql = QString("insert into deviceTable(area,name,ip) values('%1','%2','%3')"
                                    ).arg(area).arg(name).arg(ip);
        bool insert=query.exec(insertSql);
        if(!insert)
            QMessageBox::critical(this,"fail","插入失败!");
        DeviceListUpdate();
    }

}
*/

void MainWindow::receiveDeviceIP(QString deviceIP)
{
    if(deviceIP.isEmpty())
        return;

    QMap<QString,MySocketThread*>::const_iterator iter = m_sockets.find(deviceIP);
    if(iter!=m_sockets.end())
    {
        QMessageBox::information(this,"notice","该设备已连接!");
        return;
    }

    MySocketThread* mysocket=new MySocketThread(this,deviceIP);


    m_sockets.insert(deviceIP,mysocket);
    connect(mysocket, SIGNAL(sendMsgRead(QByteArray,QString)), this, SLOT(serverMsgProcess(QByteArray,QString)));
    connect(mysocket, SIGNAL(sendUnconnected(QString)), this, SLOT(serverUnconnectedHandle(QString)));
    mysocket->run();

    QSqlQuery query;
    QString querySql=QString("SELECT area, name FROM deviceTable where ip='%1' "
                             ).arg(deviceIP);
    query.exec(querySql);
    if(query.next())
    {
        QString area= query.value(0).toString();
        QString name= query.value(1).toString();
        QMessageBox::information(this,"notice",QString("%1%2成功连接!"
                                                  ).arg(area).arg(name));
    }

}

void MainWindow::serverMsgProcess(QByteArray msgBody,QString deviceIP)
{
    QDataStream stream(&msgBody, QIODevice::ReadOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    int msgType;
    stream>>msgType;
    if(msgType==(int)PACKET_REGINFO_COLLECT_ANSWER)
        emit regInfoAnswer(msgBody);
    else if(msgType==(int)PACKET_ACCESS_REC_REQ_ANSWER)
        accessRecAnswerHandle(msgBody,deviceIP);
    else if(msgType==(int)PACKET_STREAM)//处理实时视频包，直接发给实时视频子窗口
        emit streamAnswer(msgBody);
    else if(msgType==(int)PACKET_SETTING)//系统设置包
        settingHandle(msgBody,deviceIP);
    else if(msgType==(int)PACKET_ALARM_NOTICE)//门铃
        doorBellHandle(msgBody,deviceIP);

}

void MainWindow::settingHandle(QByteArray msgBody,QString deviceIP)
{
    QDataStream stream(&msgBody, QIODevice::ReadOnly);
    //stream.setByteOrder(QDataStream::LittleEndian);
    SETTING setingInfo;
    //stream.readRawData((char*)&regInfo, msgBody.size());
    stream.readRawData((char*)&setingInfo, sizeof(setingInfo));

    QString doorPwd=QString::fromLocal8Bit(setingInfo.strEntry);
    QString settingPwd=QString::fromLocal8Bit(setingInfo.strEM);
    QString quitPwd=QString::fromLocal8Bit(setingInfo.strExit);
    QString doorCtlIP=QString::fromLocal8Bit(setingInfo.strDcDeviceIP);

    int threshold=setingInfo.dwThreshold;
    QString strThreshold=QString::number(threshold, 10);
    int doorCtlNo=setingInfo.dwDcDeviceSN;
    QString strdoorCtlNo=QString::number(doorCtlNo, 10);
    int doorCtlPort=setingInfo.dwDcDevicePort;
    QString strdoorCtlPort=QString::number(doorCtlPort, 10);

    QSqlQuery query;
    QString updateSql;
    updateSql.resize(300);
    updateSql=QString("UPDATE deviceTable SET doorPwd='%1',settingPwd='%2',quitPwd='%3',threshold='%4' where ip='%5' "
                      ).arg(doorPwd).arg(settingPwd).arg(quitPwd).arg(strThreshold).arg(deviceIP);
    query.exec(updateSql);
    QString updateSql2;
    updateSql2.resize(300);
    updateSql2=QString("UPDATE deviceTable SET doorCtlNo='%1',doorCtlIP='%2',doorCtlPort='%3' where ip='%4' "
                       ).arg(strdoorCtlNo).arg(doorCtlIP).arg(strdoorCtlPort).arg(deviceIP);
    /*
    QString updateSql=QString("UPDATE deviceTable SET doorPwd='%1' and settingPwd='%2' and quitPwd='%3' and threshold='%4' "
                      "  and doorCtrNo='%5' and doorCtlIP='%6' and doorCtlPort='%7' where ip='%8' ").arg(doorPwd).arg(settingPwd).arg(quitPwd)
                      .arg(strThreshold).arg(strdoorCtlNo).arg(doorCtlIP).arg(doorCtlIP).arg(strdoorCtlPort);
    */
    query.exec(updateSql2);


}

void MainWindow::doorBellHandle(QByteArray msgBody,QString deviceIP)
{
    if(m_sockets.find(deviceIP)!=m_sockets.end())
    {
        QSqlQuery query;
        QString area;
        QString device;
        QString querySql=QString("SELECT area,name FROM deviceTable where ip='%1' ").arg(deviceIP);
        query.exec(querySql);

        if(query.next())
        {
             area = query.value(0).toString();
             device = query.value(1).toString();
        }
        QMessageBox::StandardButton del = QMessageBox::warning(NULL, "门铃", QString("来自%1%2的门铃\n是否打开实时监控？")
                      .arg(area).arg(device),QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if(del == QMessageBox::Yes)//打开实时监控，但要记得先设置列表树的当前选择元素
        {
            //先设置列表树的当前选择元素
            int topCount = DeviceListTree->topLevelItemCount();
            for (int i = 0; i < topCount; i++)
            {
                QTreeWidgetItem *item = DeviceListTree->topLevelItem(i);
                if(item->text(0)==area)
                {
                    int childCount = item->childCount();
                    for (int j = 0; j < childCount; j++)
                    {
                        QTreeWidgetItem *item2 = item->child(i);
                        if(item2->text(0)==device)
                            DeviceListTree->setCurrentItem(item2);
                    }
                }
            }

            //同时将菜单栏转换
            ui->AccessRecord->setChecked(false);
            ui->DeviceSet->setChecked(false);
            ui->FaceLib->setChecked(false);
            ui->video->setChecked(true);

            if(m_workWidget)
                delete m_workWidget;
            m_workWidget=new QWidget(this);
            workSplitter->insertWidget(0,m_workWidget);

            m_workWidget=new VideoStream(this);
            connect(m_workWidget,SIGNAL(streamReq(QString)),this,SLOT(streamReq(QString)));
            connect(m_workWidget,SIGNAL(streamStop(QString)),this,SLOT(streamStop(QString)));
            connect(this,SIGNAL(streamAnswer(QByteArray)),m_workWidget,SLOT(streamAnswer(QByteArray)));
            dynamic_cast<VideoStream *>(m_workWidget)->initReq();


            //这一段将工作区域的当前显示清空
            //清空前需要加上判断，不然会导致非法访问悬垂指针
            while(workSplitter->count()>0)
            {
                QWidget *old = workSplitter->widget(0);// deparenting removes the widget from the gui
                old->hide();
                old->setParent(NULL);//delete current widget on index 0
                old->deleteLater();
            }
            workSplitter->insertWidget(0,m_workWidget);
            workSplitter->setStretchFactor(0, 1);

            //streamReq(deviceIP);
        }
    }

}

void MainWindow::accessRecAnswerHandle(QByteArray msgBody,QString deviceIP)
{
    QDataStream stream(&msgBody, QIODevice::ReadOnly);
    //stream.setByteOrder(QDataStream::LittleEndian);
    ACCESS_REC accessRec;
    //stream.readRawData((char*)&regInfo, msgBody.size());
    stream.readRawData((char*)&accessRec, sizeof(accessRec));

    QSqlQuery query;
    QString querySql=QString("SELECT area,name FROM deviceTable where ip='%1' "
                                     ).arg(deviceIP);
    query.exec(querySql);
    if(query.next())
    {
          QString area = query.value(0).toString();
          QString device = query.value(1).toString();

          QString name=QString::fromLocal8Bit(accessRec.strName);
          QString gender=QString::fromLocal8Bit(accessRec.strGender);
          QString post=QString::fromLocal8Bit(accessRec.strDepartment);
          int similarity=accessRec.dwSimilarity;
          QString strSimilarity=QString::number(similarity, 10);
          QString strDateTime=QString::fromLocal8Bit(accessRec.strDateTime);

          querySql=QString("SELECT * FROM accessRecord where area='%1' and device='%2' and name='%3' and time='%4'"
                           ).arg(area).arg(device).arg(name).arg(strDateTime);
          query.exec(querySql);

          QStringList accessRecList;
          accessRecList<<name<<gender<<post<<strSimilarity<<strDateTime;

          if(query.next())//重复数据，只emit记录到子窗口，不加入数据库
          {
              if(m_recentRecord)
                  emit recentRecordChanged();//更新实时监控窗口的最新通行记录
              else
                  emit accessRecordReceive(accessRecList);

              return;
          }
          else {
                //使用QDateTime类型方便根据日期时间创建通行记录图片文件夹
                QDateTime dateTime = QDateTime::fromString(strDateTime, "yyyy-MM-dd hh:mm:ss");

                QString currentPath= QDir::currentPath();//当前目录
                currentPath+="/accessRecord";
                QString imagePath=currentPath+"/"+area+"/"+device+"/"
                                  +dateTime.toString("yyyy-MM-dd")+"/"+dateTime.toString("hh");
                QDir faceLibPath(imagePath);
                if(!faceLibPath.exists())/**< 如果目标目录不存在，则进行创建 */
                    if(!faceLibPath.mkpath(faceLibPath.absolutePath()))
                        return;



                QString imagename=imagePath+"/"+name+"_"+gender+"_"+post+"_"+strSimilarity+".jpg";

                QFile file(imagename);
                if (!file.open(QIODevice::ReadWrite))
                    return;

                char* imageBuf=new char[accessRec.dwPicLen];
                stream.readRawData(imageBuf, accessRec.dwPicLen);
                file.write(imageBuf,accessRec.dwPicLen);
                delete[] imageBuf;

                QString insertSql=QString("INSERT INTO accessRecord(area,device,name,gender,post,similarity,image,time)"
                                 "values('%1','%2','%3','%4','%5','%6','%7','%8')"
                                 ).arg(area).arg(device).arg(name).arg(gender).arg(post).arg(strSimilarity).arg(imagename).arg(strDateTime);
                query.exec(insertSql);

                if(m_recentRecord)
                    emit recentRecordChanged();//更新实时监控窗口的最新通行记录
                else
                    emit accessRecordReceive(accessRecList);
          }


    }

}

/*
void MainWindow::regInfoAnswerHandle(QByteArray msgBody,QString deviceIP)
{
    QDataStream stream(&msgBody, QIODevice::ReadOnly);
    //stream.setByteOrder(QDataStream::LittleEndian);
    REGINFO regInfo;
    //stream.readRawData((char*)&regInfo, msgBody.size());
    stream.readRawData((char*)&regInfo, sizeof(regInfo));
    char* imageBuf=new char[regInfo.dwPicLen];
    stream.readRawData(imageBuf, regInfo.dwPicLen);

    QSqlQuery query;
    QString querySql=QString("SELECT area,name FROM deviceTable where ip='%1' "
                                     ).arg(deviceIP);
    query.exec(querySql);
    if(query.next())
    {
          QString area = query.value(0).toString();
          QString device = query.value(1).toString();

          QString name=QString::fromLocal8Bit(regInfo.strName);
          QString gender=QString::fromLocal8Bit(regInfo.strGender);
          //QMessageBox::critical(this,"fail",QString("姓名：%1\n性别：%2").arg(name).arg(gender));
          QString post=QString::fromLocal8Bit(regInfo.strDepartment);
          //DWORD picLen=regInfo.dwPicLen;
          //char* pPic()

          QString currentPath= QDir::currentPath();//当前目录
          currentPath+="/faceLib/";
          QString imagePath=currentPath+area+"/"+device;
          QDir faceLibPath(imagePath);
          if(!faceLibPath.exists())   // 如果目标目录不存在，则进行创建
              if(!faceLibPath.mkpath(faceLibPath.absolutePath()))
                  return;

          QString imagename=imagePath+"/"+name+"_"+gender+"_"+post+".jpg";

          QFile file(imagename);
          if (!file.open(QIODevice::ReadWrite))
              return;
          file.write(imageBuf,regInfo.dwPicLen);
          delete[] imageBuf;

          QDateTime time=QDateTime::currentDateTime();//获取系统现在的时间
          QString stime = time.toString("yyyy-MM-dd hh:mm:ss"); //设置显示格式
          QString insertSql=QString("INSERT INTO faceLib(area,device,name,gender,post,image,signtime)"
                                 "values('%1','%2','%3','%4','%5','%6','%7')"
                                 ).arg(area).arg(device).arg(name).arg(gender).arg(post).arg(imagename).arg(stime);
          query.exec(insertSql);

    }
}
*/


void MainWindow::serverUnconnectedHandle(QString deviceIP)
{
    if(m_sockets.find(deviceIP)!=m_sockets.end())
    {
        MySocketThread* mySocketThread=m_sockets[deviceIP];
        mySocketThread->terminate();
        mySocketThread->deleteLater();
        m_sockets[deviceIP]=NULL;
        m_sockets.remove(deviceIP);
    }

    QSqlQuery query;
    QString querySql=QString("SELECT area, name FROM deviceTable where ip='%1' "
                             ).arg(deviceIP);
    query.exec(querySql);
    if(query.next())
    {
        QString area= query.value(0).toString();
        QString name= query.value(1).toString();
        QMessageBox::critical(this,"fail",QString("%1%2已失去连接"
                                                  ).arg(area).arg(name));

        QMessageBox::StandardButton del = QMessageBox::warning(NULL, "设备无法连接或断开", QString("确定重新连接%1%2吗?")
                                          .arg(area).arg(name), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if(del == QMessageBox::Yes)
        {
            if(m_sockets.find(deviceIP)==m_sockets.end())
            {
                MySocketThread* mysocket=new MySocketThread(this,deviceIP);
                m_sockets.insert(deviceIP,mysocket);
                connect(mysocket, SIGNAL(sendMsgRead(QByteArray,QString)), this, SLOT(serverMsgProcess(QByteArray,QString)));
                connect(mysocket, SIGNAL(sendUnconnected(QString)), this, SLOT(serverUnconnectedHandle(QString)));
                mysocket->run();

                querySql=QString("SELECT area, name FROM deviceTable where ip='%1' "
                                     ).arg(deviceIP);
                query.exec(querySql);
                if(query.next())
                {
                    QString area= query.value(0).toString();
                    QString name= query.value(1).toString();
                    QMessageBox::information(this,"notice",QString("%1%2成功连接!"
                                                          ).arg(area).arg(name));
                }

            }

        }
    }


}

/*
void MainWindow::linkerrorHandle(QAbstractSocket::SocketError)
{
    CMySocket *socket = (CMySocket *)sender();
    qDebug()<<socket->errorString();
    QString deviceIp=m_sockets.key(socket);
    socket->close();
    m_sockets.remove(deviceIP);

}
*/

void MainWindow::receiveDeviceIPForDel(QString deviceIP)
{
    if(m_sockets.find(deviceIP)!=m_sockets.end())
    {
        MySocketThread* mySocketThread=m_sockets[deviceIP];
        mySocketThread->terminate();
        delete mySocketThread;
        m_sockets[deviceIP]=NULL;
        m_sockets.remove(deviceIP);
    }

    QSqlQuery query;
    QString querySql=QString("SELECT area, name FROM deviceTable where ip='%1' "
                             ).arg(deviceIP);
    query.exec(querySql);
    if(query.next())
    {
        QString area= query.value(0).toString();
        QString name= query.value(1).toString();
        QMessageBox::critical(this,"fail",QString("%1中的%2已断开!"
                                                  ).arg(area).arg(name));
    }

}

void MainWindow::faceLibDownloadReq(QString deviceIP)
{
    if(m_sockets.find(deviceIP)!=m_sockets.end())
    {
        QString area,device;
        QSqlQuery query;
        QString querySql=QString("SELECT area,name FROM deviceTable where ip='%1' "
                                         ).arg(deviceIP);
        query.exec(querySql);
        if(query.next())
        {
            area= query.value(0).toString();
            device= query.value(1).toString();
        }

        //删除旧的人脸信息
        QString delSql=QString("DELETE FROM faceLib where area='%1' and device='%2' ").arg(area).arg(device);
        query.exec(delSql);


        QByteArray outBlock;
        QDataStream stream(&outBlock, QIODevice::ReadWrite);
        stream.setByteOrder(QDataStream::LittleEndian);

        QByteArray head("\xAA\x55\xAA\x55",4);

        REGINFO_REQ regInfoReq;
        regInfoReq.dwMsgType=PACKET_REGINFO_COLLECT_REQUEST;
        int length=regInfoReq.Size();

        stream.writeRawData(head.data(), head.size());
        stream<<length;
        stream.writeRawData((char*)&regInfoReq,length);

        m_sockets[deviceIP]->m_socket->write(outBlock);
    }
}

void MainWindow::faceLibUploadReq(QString deviceIP)
{
    if(m_sockets.find(deviceIP)!=m_sockets.end())
    {
        QString area,device;
        QSqlQuery query;
        QString querySql=QString("SELECT area,name FROM deviceTable where ip='%1' "
                                         ).arg(deviceIP);
        query.exec(querySql);
        if(query.next())
        {
            area= query.value(0).toString();
            device= query.value(1).toString();
        }


        QString findSql=QString("SELECT name,gender,post,image FROM faceLib where area='%1' and device='%2' "
                                ).arg(area).arg(device);
        query.exec(findSql);

        int i=0;

        while(query.next())//发送每一条人脸注册信息
        {
            QString name=query.value(0).toString();
            QString gender=query.value(1).toString();
            QString post=query.value(2).toString();
            QString imagepath=query.value(3).toString();

            QByteArray outBlock;
            QDataStream stream(&outBlock, QIODevice::ReadWrite);
            stream.setByteOrder(QDataStream::LittleEndian);

            QByteArray head("\xAA\x55\xAA\x55",4);
            REGINFO regInfo;
            regInfo.dwMsgType=PACKET_REGINFO_DISPATCH;

            QByteArray nameBytes=name.toLocal8Bit();
            strcpy(regInfo.strName,nameBytes.data());
            QByteArray genderBytes=gender.toLocal8Bit();
            strcpy(regInfo.strGender,genderBytes.data());
            QByteArray postBytes=post.toLocal8Bit();
            strcpy(regInfo.strDepartment,postBytes.data());

            QFile file(imagepath);
            if (!file.open(QIODevice::ReadOnly))
                return;
            regInfo.dwPicLen=file.size();
            int length=sizeof(regInfo)+regInfo.dwPicLen;
            stream.writeRawData(head.data(), head.size());
            stream<<length;
            stream.writeRawData((char*)&regInfo, sizeof(regInfo));
            QByteArray imageBuf=file.readAll();
            stream.writeRawData(imageBuf.data(), imageBuf.size());


            m_sockets[deviceIP]->m_socket->write(outBlock);
            //m_sockets[deviceIP]->m_socket->waitForBytesWritten();

            ++i;
            QTest::qWait(20);

        }

        QMessageBox::information(this,"notice",QString("成功上传%1条人脸注册信息！").arg(i));


    }
}

void MainWindow::accessRecordSearch(QString deviceIP,QStringList time)
{
    if(m_sockets.find(deviceIP)!=m_sockets.end())
    {
        QString area,device;
        QSqlQuery query;
        QString querySql=QString("SELECT area,name FROM deviceTable where ip='%1' "
                                         ).arg(deviceIP);
        query.exec(querySql);
        if(query.next())
        {
            area= query.value(0).toString();
            device= query.value(1).toString();
        }

        QString timeBegin=time.at(0);
        QString timeEnd=time.at(1);

        QByteArray outBlock;
        QDataStream stream(&outBlock, QIODevice::ReadWrite);
        stream.setByteOrder(QDataStream::LittleEndian);

        QByteArray head("\xAA\x55\xAA\x55",4);

        ACCESS_REC_REQ accessRecReq;
        accessRecReq.dwMsgType=PACKET_ACCESS_REC_REQ;

        QByteArray timeBeginBytes=timeBegin.toLocal8Bit();
        strcpy(accessRecReq.strDateTimeBegin,timeBeginBytes.data());
        QByteArray timeEndBytes=timeEnd.toLocal8Bit();
        strcpy(accessRecReq.strDateTimeEnd,timeEndBytes.data());
        int length=accessRecReq.Size();

        stream.writeRawData(head.data(), head.size());
        stream<<length;
        stream.writeRawData((char*)&accessRecReq,length);

        m_sockets[deviceIP]->m_socket->write(outBlock);
    }
    else
        QMessageBox::critical(this,"fail","该设备尚未连接，无法查看通行记录！");
}

void MainWindow::streamReq(QString deviceIP)
{
    if(m_sockets.find(deviceIP)!=m_sockets.end())
    {
        QByteArray outBlock;
        QDataStream stream(&outBlock, QIODevice::ReadWrite);
        stream.setByteOrder(QDataStream::LittleEndian);

        QByteArray head("\xAA\x55\xAA\x55",4);

        STREAM_REQ streamReq;
        streamReq.dwMsgType=PACKET_STREAM_REQ;
        int length=streamReq.Size();

        stream.writeRawData(head.data(), head.size());
        stream<<length;
        stream.writeRawData((char*)&streamReq,length);

        m_sockets[deviceIP]->m_socket->write(outBlock);
    }
    else
        QMessageBox::critical(this,"fail","该设备尚未连接，无法查看实时视频！");

}

void MainWindow::streamStop(QString deviceIP)
{
    if(m_sockets.find(deviceIP)!=m_sockets.end())
    {
        QByteArray outBlock;
        QDataStream stream(&outBlock, QIODevice::ReadWrite);
        stream.setByteOrder(QDataStream::LittleEndian);

        QByteArray head("\xAA\x55\xAA\x55",4);

        STREAM_STOP streamStop;
        streamStop.dwMsgType=PACKET_STREAM_STOP;
        int length=streamStop.Size();

        stream.writeRawData(head.data(), head.size());
        stream<<length;
        stream.writeRawData((char*)&streamStop,length);

        m_sockets[deviceIP]->m_socket->write(outBlock);
    }

}

void MainWindow::recordByName(QString name)
{
    ui->AccessRecord->setChecked(true);
    ui->FaceLib->setChecked(false);

    if(m_workWidget)
        delete m_workWidget;
    m_workWidget=new QWidget(this);
    workSplitter->insertWidget(0,m_workWidget);

    m_workWidget=new AccessRecord(this,name);
    connect(m_workWidget,SIGNAL(accessRecordSearch(QString,QStringList)),this,SLOT(accessRecordSearch(QString,QStringList)));
    connect(this,SIGNAL(accessRecordReceive(QStringList)),m_workWidget,SLOT(accessRecordReceive(QStringList)));


    //这一段将工作区域的当前显示清空
    //清空前需要加上判断，不然会导致非法访问悬垂指针
    while(workSplitter->count()>0)
    {
        QWidget *old = workSplitter->widget(0);// deparenting removes the widget from the gui
        old->hide();
        old->setParent(NULL);//delete current widget on index 0
        old->deleteLater();
    }
    workSplitter->insertWidget(0,m_workWidget);
    workSplitter->setStretchFactor(0, 1);

}

/*
void MainWindow::recentRecordSearch(QString deviceIP,QStringList times)
{
    accessRecordSearch(deviceIP,times);
}
*/


QStringList imageNameToFaceInfo(QString imageName)
{
    int count=imageName.count('_');
    if((count!=2)/*&&(count!=3)*/)
        return QStringList();
    int i=imageName.lastIndexOf(QString("."));
    QString full=imageName.left(i);
    int j=full.lastIndexOf("_");
    QString post=full.right(full.length()-j-1);
    QString rest=full.left(j);
    int k=rest.lastIndexOf(QString("_"));
    QString gender=rest.right(rest.length()-k-1);
    QString name=rest.left(k);

    QStringList faceInfo;
    faceInfo<<name<<gender<<post;
    return faceInfo;
}
