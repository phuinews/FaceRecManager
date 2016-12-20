#ifndef DEVICESETTING_H
#define DEVICESETTING_H

#include <QDialog>

namespace Ui {
class DeviceSetting;
}

//class DeviceManager;

class DeviceSetting : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceSetting(QWidget *parent = 0,QStringList list=QStringList());
    ~DeviceSetting();

    friend class DeviceManager;

signals:
    void changed();
    void configData(QStringList);
    void timeSync();


private slots:
    void on_doorOpenPwd_textChanged(const QString &arg1);

    void on_settingPwd_textChanged(const QString &arg1);

    void on_quitPwd_textChanged(const QString &arg1);

    void on_threshold_textChanged(const QString &arg1);

    void on_doorCtlNo_textChanged(const QString &arg1);

    void on_doorCtlIP_textChanged(const QString &arg1);

    void on_doorCtlPort_textChanged(const QString &arg1);

    void on_timeSyncCBox_clicked(bool checked);

    void on_OKBtn_clicked();

    void on_CancelBtn_clicked();

    void setChanged();

private:
    Ui::DeviceSetting *ui;

    bool m_changed;

    QString m_doorOpenPwd;
    QString m_settingPwd;
    QString m_quitPwd;
    QString m_threshold;
    QString m_doorCtlNo;
    QString m_doorCtlIP;
    QString m_doorCtlPort;
};

#endif // DEVICESETTING_H
