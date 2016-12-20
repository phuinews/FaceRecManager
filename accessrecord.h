#ifndef ACCESSRECORD_H
#define ACCESSRECORD_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QDateTime>
#include <QtSql>

namespace Ui {
class AccessRecord;
}

class MainWindow;

class AccessRecord : public QMainWindow
{
    Q_OBJECT

public:
    explicit AccessRecord(QWidget *parent = 0,QString name="");
    ~AccessRecord();

signals:
    void accessRecordSearch(QString,QStringList);


private slots:
    void on_searchBtn_clicked();
    void accessRecordReceive(QStringList);

    void on_startDateEdit_dateChanged(const QDate &date);

    void on_startTimeEdit_timeChanged(const QTime &time);

    void on_endDateEdit_dateChanged(const QDate &date);

    void on_endTimeEdit_timeChanged(const QTime &time);

    void itemClicked(QTreeWidgetItem*,int);

private:
    void treeWidgetInit();
    //void UIInit();

    Ui::AccessRecord *ui;
    MainWindow* m_parent;
    QTreeWidgetItem* m_chosenItem;
    QString m_deviceIP;
    QString m_area;
    QString m_device;

    QString m_name;//增加一个变量用于从名单跳转查询通行记录

    QDateTime m_startTime;
    QDateTime m_endTime;


};

#endif // ACCESSRECORD_H
