#ifndef VIDEOSTREAM_H
#define VIDEOSTREAM_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QTreeWidgetItem>

namespace Ui {
class VideoStream;
}

class MainWindow;

class VideoStream : public QMainWindow
{
    Q_OBJECT

public:
    explicit VideoStream(QWidget *parent = 0);
    ~VideoStream();

    void initReq();

signals:
    void streamReq(QString);
    void streamStop(QString);
    void recordReq(QString,QStringList);



private slots:

    void on_streamReqBtn_clicked();
    void on_streamStopBtn_clicked();
    void on_doorOpenBtn_clicked();

    void streamAnswer(QByteArray);

    void recentRecordTreeUpdate();

    void itemClickedHandle(QTreeWidgetItem*,int);



    //void on_recordBtn_clicked();

private:

    void splitterInit();
    void recentRecordTreeInit();
    void recentRecordReq();


    Ui::VideoStream *ui;
    MainWindow* m_parent;
    QTreeWidget* recentRecordTree;
    QString m_area;
    QString m_device;
    QString m_deviceIP;




};

#endif // VIDEOSTREAM_H
