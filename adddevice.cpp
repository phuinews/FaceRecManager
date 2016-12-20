#include "adddevice.h"
#include "ui_ADDDevice.h"

ADDDevice::ADDDevice(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ADDDevice)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
}

ADDDevice::~ADDDevice()
{
    delete ui;
}

void ADDDevice::on_OKBtn_clicked()
{
    QStringList DeviceData;
    DeviceData<<ui->DeviceArea->text()<<ui->DeviceName->text()<<ui->DeviceIP->text();
    emit sendData(DeviceData);
    delete this;
}

void ADDDevice::on_CancelBtn_clicked()
{
    delete this;
}
