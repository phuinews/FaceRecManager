#include "devicesetting.h"
#include "ui_devicesetting.h"
//#include "devicemanager.h"

#include<QMessageBox>
#include <QTest>

DeviceSetting::DeviceSetting(QWidget *parent,QStringList list) :
    QDialog(parent),
    ui(new Ui::DeviceSetting)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    if(list.size()==7)
    {
        ui->doorOpenPwd->setText(list.at(0));
        ui->settingPwd->setText(list.at(1));
        ui->quitPwd->setText(list.at(2));
        float threshold=(float)list.at(3).toInt()/1000;
        ui->threshold->setText(QString::number(threshold,'f',3));
        int doorCtlNo=list.at(4).toInt();
        QString hxNo=QString("%1").arg(doorCtlNo,8,16,QLatin1Char('0'));
        ui->doorCtlNo->setText("0x"+hxNo.toUpper());
        ui->doorCtlIP->setText(list.at(5));
        ui->doorCtlPort->setText(list.at(6));
    }

    m_changed=false;
    connect(this, SIGNAL(changed()), this, SLOT(setChanged()));
}

DeviceSetting::~DeviceSetting()
{
    delete ui;
}

void DeviceSetting::on_doorOpenPwd_textChanged(const QString &arg1)
{
    //m_doorOpenPwd=ui->doorOpenPwd->text();
    m_doorOpenPwd=arg1;
    emit changed();
}

void DeviceSetting::on_settingPwd_textChanged(const QString &arg1)
{
    m_settingPwd=arg1;
    emit changed();
}

void DeviceSetting::on_quitPwd_textChanged(const QString &arg1)
{
    m_quitPwd=arg1;
    emit changed();
}

void DeviceSetting::on_threshold_textChanged(const QString &arg1)
{
    float threshold=arg1.toFloat();
    int ithreshold=threshold*1000;
    if(ithreshold>1000)
    {
        QTest::qWait(10);
        m_changed=false;
        QMessageBox::critical(this,"阈值错误","请确保阈值在0-1之间!");
    }
    else
    {
        m_threshold=QString::number(ithreshold,10);
        emit changed();
    }
}

void DeviceSetting::on_doorCtlNo_textChanged(const QString &arg1)
{
    if(arg1.left(2)=="0x")//判断门控设备编号是十进制还是十六进制显示
    {
        int doorCtlNo;
        bool ok;
        doorCtlNo=arg1.right(arg1.size()-2).toInt(&ok,16);
        m_doorCtlNo=QString::number(doorCtlNo,10);
    }
    else
        m_doorCtlNo=arg1;
    emit changed();
}

void DeviceSetting::on_doorCtlIP_textChanged(const QString &arg1)
{
    m_doorCtlIP=arg1;
    emit changed();
}

void DeviceSetting::on_doorCtlPort_textChanged(const QString &arg1)
{
    m_doorCtlPort=arg1;
    emit changed();
}

void DeviceSetting::on_timeSyncCBox_clicked(bool checked)
{
    if(checked)
        emit timeSync();
}

void DeviceSetting::setChanged()
{
    m_changed=true;
}

void DeviceSetting::on_OKBtn_clicked()
{
    //QTest::qWait(20);
    if(m_changed)
    {
        QStringList deviceConfig;
        deviceConfig<<m_doorOpenPwd<<m_settingPwd<<m_quitPwd<<m_threshold<<m_doorCtlNo<<m_doorCtlIP<<m_doorCtlPort;
        emit configData(deviceConfig);
    }
    delete this;
}

void DeviceSetting::on_CancelBtn_clicked()
{
    delete this;
}
