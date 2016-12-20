#include "facemanager.h"
#include "ui_facemanager.h"
#include "mainwindow.h"

#include <QSplitter>
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>
#include <QDateTime>

extern QStringList imageNameToFaceInfo(QString imageName);

FaceManager::FaceManager(QMainWindow *parent) :
    QMainWindow(parent),
    ui(new Ui::FaceManager)
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

    Init();


    connect(FaceLibTree,SIGNAL(itemClicked(QTreeWidgetItem*,int)),this, SLOT(itemClickedHandle(QTreeWidgetItem*,int)));


}

FaceManager::~FaceManager()
{
    delete ui;
}

void FaceManager::FaceLibTreeUpdate()
{
    while(FaceLibTree->topLevelItemCount())
        {
            QTreeWidgetItem* leafItem = FaceLibTree->takeTopLevelItem(0);
            delete leafItem;
        }
    createFaceLibTree();

}
void FaceManager::createFaceLibTree()
{
    QStringList header;
    header<<"序号"<<"姓名"<<"性别"<<"职务"<<"注册时间";
    FaceLibTree->setHeaderLabels(header);
    QList<QTreeWidgetItem *> rootList;

    //QString area=m_chosenItem->parent()->text(0);
    //QString device=m_chosenItem->text(0);
    QSqlQuery query;
    QString find=QString("SELECT name,gender,post,signtime FROM faceLib WHERE area='%1' and device='%2' "
                         ).arg(m_area).arg(m_device);
    query.exec(find);

    int i=0;
    while (query.next())
    {

        QString name = query.value(0).toString();
        QString gender = query.value(1).toString();
        QString post = query.value(2).toString();
        QString signtime = query.value(3).toString();

        QStringList face;
        ++i;//序号
        face<<QString::number(i)<<name<<gender<<post<<signtime;
        QTreeWidgetItem *leaf = new QTreeWidgetItem(FaceLibTree, face);
        rootList<<leaf;

    }
        FaceLibTree->insertTopLevelItems(0, rootList);
    if(i==0)
        QMessageBox::information(this,"notice","该设备没有关联人员!");

}

void FaceManager::Init()
{
    addFace=new QPushButton("本地添加",this);
    //addDevice->resize(60,30);
    deleteFace=new QPushButton("删除",this);
    faceLibUpload=new QPushButton("名单下发",this);
    faceLibDownload=new QPushButton("名单收集",this);
    //faceLibUpdate=new QPushButton("名单更新",this);

    //设置按钮大小和字体
    QFont ft;
    ft.setPointSize(12);
    addFace->resize(122,26);
    addFace->setFont(ft);
    deleteFace->resize(122,26);
    deleteFace->setFont(ft);
    faceLibUpload->resize(122,26);
    faceLibUpload->setFont(ft);
    faceLibDownload->resize(122,26);
    faceLibDownload->setFont(ft);
    //faceLibUpdate->resize(122,26);
    //faceLibUpdate->setFont(ft);



    QSplitter* BtnSplitter = new QSplitter(Qt::Horizontal);
    BtnSplitter->addWidget(addFace);
    BtnSplitter->addWidget(deleteFace);
    BtnSplitter->addWidget(faceLibUpload);
    BtnSplitter->addWidget(faceLibDownload);
    //BtnSplitter->addWidget(faceLibUpdate);
    BtnSplitter->addWidget(new QLabel("",this));
    BtnSplitter->setStretchFactor(4,1);

    FaceLibTree=new QTreeWidget(this);
    FaceLibTree->setColumnCount(5);
    if(m_chosenItem)
        createFaceLibTree();


    QSplitter* mainSplitter = new QSplitter(Qt::Vertical);
    QLabel* lableAd=new QLabel("",this);
    lableAd->resize(8,8);
    mainSplitter->addWidget(lableAd);

    mainSplitter->addWidget(BtnSplitter);
    mainSplitter->addWidget(FaceLibTree);
    //mainSplitter->setStretchFactor(0, 0);
    mainSplitter->setStretchFactor(2,1);
    //setCentralWidget(mainSplitter);
    //mainSplitter->show();

    /*
    QSplitter* ImageSplitter = new QSplitter(Qt::Vertical);
    QLabel* lableAd2=new QLabel("",this);
    lableAd2->resize(122,300);
    //m_imageLabel=new QLabel(this);
    //m_imageLabel->resize(331,351);
    //m_imageLabel->setScaledContents(true);
    new QPushButton("删除",this);
    ImageSplitter->addWidget(lableAd2);
    ImageSplitter->addWidget(ui->faceImageL);
    ImageSplitter->addWidget(lableAd3);
    //ImageSplitter->setStretchFactor(1, 0);
    //ImageSplitter->setStretchFactor(2, 1);
    */

    m_imageSplitter=NULL;

    m_allSplitter = new QSplitter(Qt::Horizontal);
    QLabel* lableAd4=new QLabel("",this);
    lableAd4->resize(8,8);
    m_allSplitter->addWidget(lableAd4);
    m_allSplitter->addWidget(mainSplitter);
    //m_allSplitter->addWidget(ImageSplitter);
    m_allSplitter->setStretchFactor(1, 1);

    setCentralWidget(m_allSplitter);



    connect(addFace,SIGNAL(clicked()),this,SLOT(on_addFace_clicked()));
    connect(deleteFace,SIGNAL(clicked()),this,SLOT(on_deleteFace_clicked()));
    connect(faceLibUpload,SIGNAL(clicked()),this,SLOT(on_faceLibUpload_clicked()));
    connect(faceLibDownload,SIGNAL(clicked()),this,SLOT(on_faceLibDownload_clicked()));
    //connect(faceLibUpdate,SIGNAL(clicked()),this,SLOT(on_faceLibUpdate_clicked()));


}

void FaceManager::on_addFace_clicked()
{
    QString dirPath = QFileDialog::getExistingDirectory(this, tr("选择注册头像文件夹"), "/", QFileDialog::ShowDirsOnly);
    QDir dir(dirPath);
    //QString area=m_chosenItem->parent()->text(0);
    //QString device=m_chosenItem->text(0);

    QString currentPath= QDir::currentPath();//当前路径
    currentPath+="/faceLib/";
    QString faceLibPathstr=currentPath+"/"+m_area+"/"+m_device;
    QDir faceLibPath(faceLibPathstr);
    if(!faceLibPath.exists())   /**< 如果目标目录不存在，则进行创建 */
        if(!faceLibPath.mkpath(faceLibPath.absolutePath()))
            return;


    int success=0;
    int repeat=0;
    int badname=0;
    QStringList namefilter;
    namefilter<<"*.jpg"<<"*.jpeg";
    //dir.setNameFilters(namefilter);
    foreach (QFileInfo fileInfo, dir.entryInfoList(namefilter,QDir::Files))
    {
        QString imageName = fileInfo.fileName();
        /*已经过滤了，不需要再对文件判断
        if ((imageName == QString(".")) || (imageName == QString("..")))
            continue;
        if (fileInfo.isDir())
        continue;
        */

        QStringList faceInfo=imageNameToFaceInfo(imageName);
        if(faceInfo.empty())
        {
            ++badname;
            continue;
        }
        QString name=faceInfo.at(0);
        QString gender=faceInfo.at(1);
        QString post=faceInfo.at(2);
        QString image=faceLibPathstr+"/"+imageName;
        QDateTime time=QDateTime::currentDateTime();//获取系统现在的时间
        QString stime = time.toString("yyyy-MM-dd hh:mm:ss"); //设置显示格式

        QSqlQuery query;
        QString querySql=QString("SELECT area,device FROM faceLib where area='%1' and device='%2' and name='%3' "
                                 ).arg(m_area).arg(m_device).arg(name);
        query.exec(querySql);
        if(query.next())
        {
            ++repeat;
            continue;
        }

        QString addSql=QString("INSERT INTO faceLib(area,device,name,gender,post,image,signtime)"
                               "values('%1','%2','%3','%4','%5','%6','%7')"
                               ).arg(m_area).arg(m_device).arg(name).arg(gender).arg(post).arg(image).arg(stime);
        query.exec(addSql);//对数据库的操作应该写成事务，失败可以回滚，暂时先这样写了

        //暂时不判断是否copy成功，这样不用考虑头像文件重名的问题
        QFile::copy(fileInfo.filePath(),faceLibPath.filePath(fileInfo.fileName()));
        ++success;
    }
    QMessageBox::information(this,"notice",QString("成功添加%1条记录!\n添加失败：重名%2条，命名不规范%3条!"
                                                   ).arg(success).arg(repeat).arg(badname));
    FaceLibTreeUpdate();

}

void FaceManager::on_deleteFace_clicked()
{
    QTreeWidgetItem * item=FaceLibTree->currentItem();
    if(!item)
        QMessageBox::information(this,"notice","没有人脸信息被选中!");
    else
    {
        QString name=item->text(1);
        QMessageBox::StandardButton del = QMessageBox::warning(NULL, "删除人员", QString("确定删除%1的注册信息吗?").arg(name), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if(del == QMessageBox::Yes)
        {
            //QString area=m_chosenItem->parent()->text(0);
            //QString device=m_chosenItem->text(0);

            QString signtime=item->text(4);//暂时根据姓名和注册时间标识一条人脸注册信息

            QSqlQuery query;
            //删除对应人脸注册图像
            QString findimage=QString("SELECT image FROM faceLib where area='%1' and device='%2' and name='%3' ").arg(m_area).arg(m_device).arg(name);
            query.exec(findimage);
            if(query.next())
            {
                QString imagepath = query.value(0).toString();
                QFile::remove(imagepath);
            }

            //删除数据库对应记录
            QString delSql=QString("DELETE FROM faceLib where area='%1' and device='%2' and name='%3' and signtime='%4'").arg(m_area).arg(m_device).arg(name).arg(signtime);
            query.exec(delSql);
                //QMessageBox::information(this,"notice","成功删除!");//不再提示删除成功
            delete item;
            FaceLibTreeUpdate();
        }
    }
}

void FaceManager::on_faceLibUpload_clicked()
{
     emit faceLibUploadReq(m_deviceIP);
}

void FaceManager::on_faceLibDownload_clicked()
{
    emit faceLibDownloadReq(m_deviceIP);

}

/*
void FaceManager::on_faceLibUpdate_clicked()
{
    FaceLibTreeUpdate();
}
*/

void FaceManager::itemClickedHandle(QTreeWidgetItem* item,int)
{
    if(m_imageSplitter==NULL)
    {
         m_imageSplitter=new QSplitter(Qt::Vertical,this);
         QLabel* lableAd5=new QLabel("",this);
         lableAd5->setFixedSize(331,28);
         m_imageSplitter->addWidget(lableAd5);

         QPushButton* recordBtn=new QPushButton("通行记录查询",this);
         //设置按钮大小和字体
         QFont ft;
         ft.setPointSize(12);
         //recordBtn->resize(331,26);
         recordBtn->setFixedSize(331,26);
         recordBtn->setFont(ft);
         connect(recordBtn,SIGNAL(clicked()),this,SLOT(on_recordBtn_clicked()));

         m_imageSplitter->addWidget(recordBtn);


         m_imageLabel=new QLabel("",this);
         m_imageLabel->setFixedSize(331,351);
         m_imageLabel->setScaledContents(true);
         m_imageSplitter->addWidget(m_imageLabel);

         //水平包裹空隙及图片窗口
         QLabel* LabelAid7=new QLabel("",this);
         LabelAid7->setFixedSize(20,20);
         QSplitter* wrap_imageSplitter= new QSplitter(Qt::Horizontal,this);
         wrap_imageSplitter->addWidget(LabelAid7);
         wrap_imageSplitter->addWidget(m_imageSplitter);
         QLabel* LabelAid8=new QLabel("",this);
         LabelAid8->setFixedSize(40,40);
         wrap_imageSplitter->addWidget(LabelAid8);

         QSplitter* wrap_imageSplitter2= new QSplitter(Qt::Vertical,this);
         QLabel* LabelAid9=new QLabel("",this);
         QLabel* LabelAid10=new QLabel("",this);
         wrap_imageSplitter2->addWidget(LabelAid9);
         wrap_imageSplitter2->addWidget(wrap_imageSplitter);
         wrap_imageSplitter2->addWidget(LabelAid10);
         wrap_imageSplitter2->setStretchFactor(2, 1);

         m_allSplitter->addWidget(wrap_imageSplitter2);
         m_allSplitter->setStretchFactor(1, 1);

    }


    QString name=item->text(1);
    QSqlQuery query;
    QString querySql=QString("SELECT image FROM faceLib where area='%1' and device='%2' and name='%3' "
                                 ).arg(m_area).arg(m_device).arg(name);
    query.exec(querySql);
    if(query.next())
    {
        QString imagepath=query.value(0).toString();
        QPixmap pix;
        pix.load(imagepath);

        m_imageLabel->setPixmap(pix);

        /*
        QPixmap pix;
        pix.load(imagepath);
        img1->scaled(image1Label->size(), Qt::KeepAspectRatio);
        ui->faceLibImageL->setPixmap(pix);
        ui->faceLibImageL->resize(pix.width(),pix.height());
        */
    }
    else
        m_imageLabel->clear();
}

void FaceManager::regInfoAnswerHandle(QByteArray msgBody)
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
                                     ).arg(m_deviceIP);
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
          if(!faceLibPath.exists())   /**< 如果目标目录不存在，则进行创建 */
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

          FaceLibTreeUpdate();

    }
}

void FaceManager::on_recordBtn_clicked()
{
    if(FaceLibTree->currentItem())
    {
        emit recordName(FaceLibTree->currentItem()->text(1));
    }
}
