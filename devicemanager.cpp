#include "devicemanager.h"
#include "ui_devicemanager.h"
#include "mainwindow.h"


#include <QMessageBox>
#include <QTest>
#include <QDateTime>

DeviceManager::DeviceManager(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DeviceManager)
{
    ui->setupUi(this);

    m_parent= (MainWindow*) parentWidget();
    //m_chosenItem=m_parent->DeviceListTree->currentItem();
    Init();



}

DeviceManager::~DeviceManager()
{
    delete ui;


}

void DeviceManager::Init()
{
    m_addBtn=new QPushButton("添加",this);
    m_deleteBtn=new QPushButton("删除",this);

    m_connectDeviceBtn=new QPushButton("连接设备",this);
    //addDevice->resize(60,30);
    m_disconnectDeviceBtn=new QPushButton("断开设备",this);
    m_settingBtn=new QPushButton("参数设置",this);
    m_openDoorBtn=new QPushButton("远程开门",this);
    m_updateBtn=new QPushButton("刷新",this);


    //设置按钮大小和字体
    QFont ft;
    ft.setPointSize(12);
    m_addBtn->resize(122,26);
    m_addBtn->setFont(ft);
    m_deleteBtn->resize(122,26);
    m_deleteBtn->setFont(ft);
    m_connectDeviceBtn->resize(122,26);
    m_connectDeviceBtn->setFont(ft);
    m_disconnectDeviceBtn->resize(122,26);
    m_disconnectDeviceBtn->setFont(ft);
    m_settingBtn->resize(122,26);
    m_settingBtn->setFont(ft);
    m_openDoorBtn->resize(122,26);
    m_openDoorBtn->setFont(ft);
    m_updateBtn->resize(122,26);
    m_updateBtn->setFont(ft);




    QSplitter* BtnSplitter = new QSplitter(Qt::Horizontal);
    BtnSplitter->addWidget(m_addBtn);
    BtnSplitter->addWidget(m_deleteBtn);
    BtnSplitter->addWidget(m_connectDeviceBtn);
    BtnSplitter->addWidget(m_disconnectDeviceBtn);
    BtnSplitter->addWidget(m_settingBtn);
    BtnSplitter->addWidget(m_openDoorBtn);
    BtnSplitter->addWidget(m_updateBtn);

    BtnSplitter->addWidget(new QLabel("",this));
    BtnSplitter->setStretchFactor(7,1);

    m_deviceTable=new QTreeWidget(this);
    m_deviceTable->setColumnCount(5);
    createDeviceTable();


    QSplitter* mainSplitter = new QSplitter(Qt::Vertical);
    QLabel* lableAd=new QLabel("",this);
    lableAd->resize(8,8);
    mainSplitter->addWidget(lableAd);

    mainSplitter->addWidget(BtnSplitter);
    mainSplitter->addWidget(m_deviceTable);
    //mainSplitter->setStretchFactor(0, 0);
    mainSplitter->setStretchFactor(2,1);
    //setCentralWidget(mainSplitter);
    //mainSplitter->show();

    QSplitter* allSplitter = new QSplitter(Qt::Horizontal);
    QLabel* lableAd2=new QLabel("",this);
    lableAd2->resize(8,8);
    allSplitter->addWidget(lableAd2);
    allSplitter->addWidget(mainSplitter);
    allSplitter->setStretchFactor(1, 1);

    setCentralWidget(allSplitter);


    connect(m_addBtn,SIGNAL(clicked()),this,SLOT(on_m_addBtn_clicked()));
    connect(m_deleteBtn,SIGNAL(clicked()),this,SLOT(on_m_deleteBtn_clicked()));
    connect(m_connectDeviceBtn,SIGNAL(clicked()),this,SLOT(on_m_connectDeviceBtn_clicked()));
    connect(m_disconnectDeviceBtn,SIGNAL(clicked()),this,SLOT(on_m_disconnectDeviceBtn_clicked()));
    connect(m_settingBtn,SIGNAL(clicked()),this,SLOT(on_m_settingBtn_clicked()));
    connect(m_openDoorBtn,SIGNAL(clicked()),this,SLOT(on_m_openDoorBtn_clicked()));
    connect(m_updateBtn,SIGNAL(clicked()),this,SLOT(on_m_updateBtn_clicked()));

    connect(m_deviceTable,SIGNAL(itemClicked(QTreeWidgetItem*,int)),this,SLOT(setMainWinTreeItem(QTreeWidgetItem*,int)));

}

void DeviceManager::on_m_addBtn_clicked()
{
    ADDDevice* addDevice=new ADDDevice(this);
    connect(addDevice,SIGNAL(sendData(QStringList)),this,SLOT(addDeviceHandle(QStringList)));
    addDevice->show();

}

void DeviceManager::on_m_deleteBtn_clicked()
{
    QTreeWidgetItem* item=m_deviceTable->currentItem();
    if(!item)
        QMessageBox::information(this,"notice","没有设备被选中!");
    else
    {
        QMessageBox::StandardButton del = QMessageBox::warning(NULL, "删除设备", "确定删除所选设备吗?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if(del == QMessageBox::Yes)
        {
            QString device=item->text(1);
            QString area=item->text(2);
            QSqlQuery query;
            QString delSql=QString("DELETE FROM deviceTable where area='%1' and name='%2' ").arg(area).arg(device);
            query.exec(delSql);

            delete item;
            //列表和主窗口列表树更新
            deviceTableUpdate();
            m_parent->DeviceListUpdate();
        }
    }
}

void DeviceManager::on_m_connectDeviceBtn_clicked()
{
    QTreeWidgetItem* item=m_deviceTable->currentItem();
    if(!item)
        QMessageBox::information(this,"notice","没有设备被选中!");
    else
        emit sendData(item->text(3));

}

void DeviceManager::on_m_disconnectDeviceBtn_clicked()
{
    QTreeWidgetItem* item=m_deviceTable->currentItem();
    if(!item)
        QMessageBox::information(this,"notice","没有设备被选中!");
    else
        emit sendDataForDel(item->text(3));

}

void DeviceManager::on_m_updateBtn_clicked()
{
    deviceTableUpdate();
}

void DeviceManager::on_m_settingBtn_clicked()
{

    QTreeWidgetItem* item=m_deviceTable->currentItem();
    if(!item)
        QMessageBox::information(this,"notice","没有设备被选中!");
    else
    {
        QString area=item->text(2);
        QString name=item->text(1);
        QString deviceIP=item->text(3);

        QMap<QString,MySocketThread*>::const_iterator iter = m_parent->m_sockets.find(deviceIP);
        if(iter!=m_parent->m_sockets.end())
        {
            settingRequest();
            QTest::qWait(100);
        }

        QSqlQuery query;
        QString doorPwd;
        QString settingPwd;
        QString quitPwd;
        QString threshold;
        QString doorCtlNo;
        QString doorCtlIP;
        QString doorCtlPort;
        QStringList list;
        QString find=QString("SELECT doorPwd,settingPwd,quitPwd,threshold "
                             " FROM deviceTable where area='%1' and name='%2' ").arg(area).arg(name);
        query.exec(find);
        if(query.next())
        {

           doorPwd=query.value(0).toString();
           settingPwd=query.value(1).toString();
           quitPwd=query.value(2).toString();
           threshold=query.value(3).toString();
        }
        QString find2=QString("SELECT doorCtlNo,doorCtlIP,doorCtlPort "
                             " FROM deviceTable where area='%1' and name='%2' ").arg(area).arg(name);
        query.exec(find2);
        if(query.next())
        {

           doorCtlNo=query.value(0).toString();
           doorCtlIP=query.value(1).toString();
           doorCtlPort=query.value(2).toString();
        }

        list<<doorPwd<<settingPwd<<quitPwd<<threshold;
        list<<doorCtlNo<<doorCtlIP<<doorCtlPort;
        DeviceSetting* deviceSetting=new DeviceSetting(this,list);
        deviceSetting->show();

        connect(deviceSetting,SIGNAL(configData(QStringList)),this,SLOT(configModify(QStringList)));
        connect(deviceSetting,SIGNAL(timeSync()),this,SLOT(timeSync()));
    }
}

void DeviceManager::on_m_openDoorBtn_clicked()
{
    QTreeWidgetItem* item=m_deviceTable->currentItem();
    QString deviceIP=item->text(3);
    QMap<QString,MySocketThread*>::const_iterator iter = m_parent->m_sockets.find(deviceIP);
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

        m_parent->m_sockets[deviceIP]->m_socket->write(outBlock);

    }
    else//否则直接给前端门控设备发送UDP包
    {

    }

}

void DeviceManager::addDeviceHandle(QStringList deviceData)
{
    QString area=deviceData.at(0);
    QString name=deviceData.at(1);
    QString ip=deviceData.at(2);

    QSqlQuery query;
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

        //列表和主窗口列表树更新
        deviceTableUpdate();
        m_parent->DeviceListUpdate();
    }
}

void DeviceManager::createDeviceTable()
{
    QStringList header;
    header<<"序号"<<"设备名称"<<"区域"<<"设备IP"<<"设备状态";
    m_deviceTable->setHeaderLabels(header);
    QList<QTreeWidgetItem *> rootList;

    QSqlQuery query;
    //QString find=QString("SELECT area, name FROM deviceTable where (area='%1' and name='%2') or ip='%3' "
                         //).arg(area).arg(name).arg(ip);
    query.exec("SELECT area, name,ip FROM deviceTable");

    int i=0;
    while (query.next())
    {

        QString area = query.value(0).toString();
        QString device = query.value(1).toString();
        QString deviceIP = query.value(2).toString();

        QString deviceStatus;
        QMap<QString,MySocketThread*>::const_iterator iter = m_parent->m_sockets.find(deviceIP);
        if(iter!=m_parent->m_sockets.end())
            deviceStatus="在线";
        else
            deviceStatus="断开";

        QStringList deviceData;
        ++i;//序号
        deviceData<<QString::number(i)<<device<<area<<deviceIP<<deviceStatus;
        QTreeWidgetItem *leaf = new QTreeWidgetItem(m_deviceTable, deviceData);
        rootList<<leaf;

    }

    m_deviceTable->insertTopLevelItems(0, rootList);

}

void DeviceManager::deviceTableUpdate()
{
    while(m_deviceTable->topLevelItemCount())
    {
        QTreeWidgetItem* item = m_deviceTable->takeTopLevelItem(0);
        delete item;
    }

    createDeviceTable();
}

void DeviceManager::configModify(QStringList deviceConfig)
{
    QTreeWidgetItem* item=m_deviceTable->currentItem();
    QString deviceIP=item->text(3);
    QMap<QString,MySocketThread*>::const_iterator iter = m_parent->m_sockets.find(deviceIP);
    if(iter!=m_parent->m_sockets.end())
    {
        QByteArray outBlock;
        QDataStream stream(&outBlock, QIODevice::ReadWrite);
        stream.setByteOrder(QDataStream::LittleEndian);

        QByteArray head("\xAA\x55\xAA\x55",4);

        SETTING settingInfo;
        settingInfo.dwMsgType=PACKET_SETTING_DISPATCH;

        int length=settingInfo.Size();

        QByteArray doorOpenPwdBytes=deviceConfig.at(0).toLocal8Bit();
        strcpy(settingInfo.strEntry,doorOpenPwdBytes.data());
        QByteArray settingPwdBytes=deviceConfig.at(1).toLocal8Bit();
        strcpy(settingInfo.strEM,settingPwdBytes.data());
        QByteArray quitBytes=deviceConfig.at(2).toLocal8Bit();
        strcpy(settingInfo.strExit,quitBytes.data());
        int threshold=deviceConfig.at(3).toInt();
        settingInfo.dwThreshold=threshold;
        settingInfo.dwDcDeviceSN=deviceConfig.at(4).toInt();


        QByteArray doorCtlIPBytes=deviceConfig.at(5).toLocal8Bit();
        strcpy(settingInfo.strDcDeviceIP,doorCtlIPBytes.data());
        int doorCtlPort=deviceConfig.at(6).toInt();
        settingInfo.dwDcDevicePort=doorCtlPort;

        stream.writeRawData(head.data(), head.size());
        stream<<length;
        stream.writeRawData((char*)&settingInfo,length);

        m_parent->m_sockets[deviceIP]->m_socket->write(outBlock);

        QSqlQuery query;
        QString updateSql=QString("UPDATE deviceTable SET doorPwd='%1',settingPwd='%2',quitPwd='%3',threshold='%4', "
                          " doorCtrNo='%5',doorCtlIP='%6',doorCtlPort='%7' where ip='%8' ").arg(deviceConfig.at(0))
                          .arg(deviceConfig.at(1)).arg(deviceConfig.at(2)).arg(deviceConfig.at(3))
                          .arg(deviceConfig.at(4)).arg(deviceConfig.at(5)).arg(deviceConfig.at(6)).arg(deviceIP);
        query.exec(updateSql);
    }
    else
        QMessageBox::critical(this,"fail","该设备未连接，无法进行参数设置!");


}

void DeviceManager::settingRequest()
{
    QTreeWidgetItem* item=m_deviceTable->currentItem();
    QString deviceIP=item->text(3);
    QMap<QString,MySocketThread*>::const_iterator iter = m_parent->m_sockets.find(deviceIP);
    if(iter!=m_parent->m_sockets.end())
    {
        QByteArray outBlock;
        QDataStream stream(&outBlock, QIODevice::ReadWrite);
        stream.setByteOrder(QDataStream::LittleEndian);

        QByteArray head("\xAA\x55\xAA\x55",4);

        SETTING_REQ settingReq;
        settingReq.dwMsgType=PACKET_SETTING_REQ;

        int length=settingReq.Size();

        stream.writeRawData(head.data(), head.size());
        stream<<length;
        stream.writeRawData((char*)&settingReq,length);

        m_parent->m_sockets[deviceIP]->m_socket->write(outBlock);
    }
}

void DeviceManager::timeSync()
{
    QTreeWidgetItem* item=m_deviceTable->currentItem();
    QString deviceIP=item->text(3);
    QMap<QString,MySocketThread*>::const_iterator iter = m_parent->m_sockets.find(deviceIP);
    if(iter!=m_parent->m_sockets.end())
    {
        QByteArray outBlock;
        QDataStream stream(&outBlock, QIODevice::ReadWrite);
        stream.setByteOrder(QDataStream::LittleEndian);

        QByteArray head("\xAA\x55\xAA\x55",4);

        TIMESYNC timeSync;
        timeSync.dwMsgType=PACKET_TIMESYNC;


        int length=timeSync.Size();

        QDateTime time=QDateTime::currentDateTime();//获取系统现在的时间
        QString stime = time.toString("yyyy-MM-dd hh:mm:ss"); //设置显示格式
        QByteArray timeBytes=stime.toLocal8Bit();
        strcpy(timeSync.strSyncDateTime,timeBytes.data());


        stream.writeRawData(head.data(), head.size());
        stream<<length;
        stream.writeRawData((char*)&timeSync,length);

        m_parent->m_sockets[deviceIP]->m_socket->write(outBlock);
    }

}

void DeviceManager::setMainWinTreeItem(QTreeWidgetItem* item,int row)
{
    QString area=item->text(2);
    QString device=item->text(1);

    //设置列表树的当前选择元素,在响应门铃那有同样的操作
    int topCount = m_parent->DeviceListTree->topLevelItemCount();
    for (int i = 0; i < topCount; i++)
    {
        QTreeWidgetItem *item = m_parent->DeviceListTree->topLevelItem(i);
        if(item->text(0)==area)
        {
            int childCount = item->childCount();
            for (int j = 0; j < childCount; j++)
            {
                QTreeWidgetItem *item2 = item->child(j);
                if(item2->text(0)==device)
                    m_parent->DeviceListTree->setCurrentItem(item2);
            }
        }
    }
}


