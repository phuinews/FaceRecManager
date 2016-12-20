#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include "adddevice.h"
#include "devicesetting.h"

#include <QMainWindow>
#include <QPushButton>
#include <QtSql>
#include <QTreeWidgetItem>
#include <QTreeWidget>


namespace Ui {
class DeviceManager;
}

class MainWindow;

class DeviceManager : public QMainWindow
{
    Q_OBJECT

public:
    explicit DeviceManager(QWidget *parent = 0);
    ~DeviceManager();

signals:
    void sendData(QString);
    void sendDataForDel(QString);


private slots:
    void on_m_connectDeviceBtn_clicked();
    void on_m_disconnectDeviceBtn_clicked();
    void on_m_openDoorBtn_clicked();
    void on_m_updateBtn_clicked();
    void on_m_addBtn_clicked();
    void on_m_deleteBtn_clicked();
    void on_m_settingBtn_clicked();

    void addDeviceHandle(QStringList);
    void configModify(QStringList);
    void timeSync();

    void setMainWinTreeItem(QTreeWidgetItem*,int);

private:
    void Init();
    void deviceTableUpdate();
    void createDeviceTable();

    void settingRequest();


    Ui::DeviceManager *ui;

    //QTreeWidgetItem *m_chosenItem;
    MainWindow* m_parent;
    QTreeWidget* m_deviceTable;
    //QString m_deviceIP;

    QPushButton* m_addBtn;
    QPushButton* m_deleteBtn;
    QPushButton* m_connectDeviceBtn;
    QPushButton* m_disconnectDeviceBtn;
    QPushButton* m_settingBtn;
    QPushButton* m_openDoorBtn;
    QPushButton* m_updateBtn;


};

#endif // DEVICEMANAGER_H
