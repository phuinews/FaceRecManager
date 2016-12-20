#ifndef ADDDEVICE_H
#define ADDDEVICE_H

#include <QDialog>
#include <QtSql>

namespace Ui {
    class ADDDevice;
}

class ADDDevice : public QDialog
{
    Q_OBJECT

public:
    explicit ADDDevice(QWidget *parent = 0);
    ~ADDDevice();

private:
    Ui::ADDDevice *ui;

signals:
    void sendData(QStringList);

private slots:
    void on_OKBtn_clicked();

    void on_CancelBtn_clicked();


};

#endif // ADDDevice_H
