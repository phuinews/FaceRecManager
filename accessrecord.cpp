#include "accessrecord.h"
#include "ui_accessrecord.h"
#include "mainwindow.h"

AccessRecord::AccessRecord(QWidget *parent,QString name) :
    QMainWindow(parent),m_name(name),
    ui(new Ui::AccessRecord)
{
    ui->setupUi(this);

    m_parent= (MainWindow*) parentWidget();
    m_chosenItem=m_parent->DeviceListTree->currentItem();

    //由于经常要用到这几个参数，因此设为成员变量，在这里初始化
    m_area=m_chosenItem->parent()->text(0);
    m_device=m_chosenItem->text(0);
    QSqlQuery query;
    QString querySql=QString("SELECT ip FROM deviceTable where area='%1' and name='%2' "
                                     ).arg(m_area).arg(m_device);
    query.exec(querySql);
    if(query.next())
        m_deviceIP= query.value(0).toString();


    ui->startDateEdit->setCalendarPopup(true);
    ui->endDateEdit->setCalendarPopup(true);
    ui->startDateEdit->setDateTime(QDateTime::currentDateTime());
    ui->endDateEdit->setDateTime(QDateTime::currentDateTime());

    //如有需要可设置有效时间限制
    //dateEdit->setMinimumDate(QDate::currentDate().addDays(-365));  // -365天
    ui->startDateEdit->setMaximumDate(QDate::currentDate()); // +365天
    ui->endDateEdit->setMaximumDate(QDate::currentDate());
    ui->startTimeEdit->setDisplayFormat("HH:mm:ss");
    ui->endTimeEdit->setDisplayFormat("HH:mm:ss");
    ui->endTimeEdit->setTime(QTime(23,59,59));
    m_startTime.setDate(ui->startDateEdit->date());
    m_startTime.setTime(ui->startTimeEdit->time());
    m_endTime.setDate(ui->endDateEdit->date());
    m_endTime.setTime(ui->endTimeEdit->time());

    //UIInit();暂时未能实现


    treeWidgetInit();
    connect(ui->treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(itemClicked(QTreeWidgetItem*,int)));
    ui->accessImageL->setScaledContents(true);
    ui->faceLibImageL->setScaledContents(true);

    if(!m_name.isEmpty())
        ui->nameLabel->setText(m_name);

}

AccessRecord::~AccessRecord()
{
    delete ui;
}

void AccessRecord::treeWidgetInit()
{
    ui->treeWidget->setColumnCount(5);
    QStringList header;
    header<<"姓名"<<"性别"<<"职务"<<"相似度"<<"通行时间";
    ui->treeWidget->setHeaderLabels(header);

}

void AccessRecord::itemClicked(QTreeWidgetItem* item,int row)
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

void AccessRecord::on_searchBtn_clicked()
{
    while(ui->treeWidget->topLevelItemCount())
    {
        QTreeWidgetItem* leafItem = ui->treeWidget->takeTopLevelItem(0);
        delete leafItem;
    }


    QMap<QString,MySocketThread*>::const_iterator iter = m_parent->m_sockets.find(m_deviceIP);
    if(iter!=m_parent->m_sockets.end())//如果有连接则联网查询
    {
        QStringList time;
        time<<m_startTime.toString("yyyy-MM-dd hh:mm:ss")<<m_endTime.toString("yyyy-MM-dd hh:mm:ss");
        emit accessRecordSearch(m_deviceIP,time);
    }
    /*
    else//否则只查询本地数据库记录,暂时不实现该查询
    {
        //accessRecList<<name<<gender<<post<<strSimilarity<<strDateTime;
        QMessageBox::information(this,"notice","设备未连接，只能查询已上传记录，可能不全!");
        querySql=QString("SELECT * FROM accessRecord where area='%1' and device='%2' and name='%3' and time='%4'"
                         ).arg(area).arg(device).arg(name).arg(strDateTime);
        query.exec(querySql);
    }
    */
}

void AccessRecord::accessRecordReceive(QStringList accessRecList)
{
    //treeWidgetInit();

    /*
    QList<QTreeWidgetItem *> rootList;
    QTreeWidgetItem *leaf = new QTreeWidgetItem(ui->treeWidget, accessRecList);
    rootList<<leaf;
    ui->treeWidget->insertTopLevelItems(0, rootList);
    */
    //换一种方式构建树
    if(m_name.isEmpty())//查询所有记录
    {
        QTreeWidgetItem *leaf = new QTreeWidgetItem(ui->treeWidget, accessRecList);
        //ui->treeWidget->addTopLevelItem(leaf);
        ui->treeWidget->insertTopLevelItem(0,leaf);
    }
    else//查询单人记录
    {
        if(accessRecList.at(0)==m_name)
        {
            QTreeWidgetItem *leaf = new QTreeWidgetItem(ui->treeWidget, accessRecList);
            ui->treeWidget->insertTopLevelItem(0,leaf);
        }

    }



}

void AccessRecord::on_startDateEdit_dateChanged(const QDate &date)
{
    m_startTime.setDate(ui->startDateEdit->date());
}

void AccessRecord::on_startTimeEdit_timeChanged(const QTime &time)
{
    m_startTime.setTime(ui->startTimeEdit->time());
}

void AccessRecord::on_endDateEdit_dateChanged(const QDate &date)
{
    m_endTime.setDate(ui->endDateEdit->date());
}

void AccessRecord::on_endTimeEdit_timeChanged(const QTime &time)
{
    m_endTime.setTime(ui->endTimeEdit->time());

}

/*
void AccessRecord::UIInit()
{
    QSplitter* mainSplitter = new QSplitter(Qt::Horizontal);
    QLabel* lableAid=new QLabel("",this);
    mainSplitter->addWidget(lableAid);
    mainSplitter->addWidget(ui);
    QLabel* lableAid2=new QLabel("",this);
    mainSplitter->addWidget(lableAid2);

    //设置各部件比例
    QList<int> list;
    mainSplitter->setSizes(list<<1<<6<<1);
    setCentralWidget(mainSplitter);

}
*/
