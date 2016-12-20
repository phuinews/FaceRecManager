#ifndef FACEMANAGER_H
#define FACEMANAGER_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QtSql>
#include <QSplitter>



namespace Ui {
class FaceManager;
}

class MainWindow;

class FaceManager : public QMainWindow
{
    Q_OBJECT

public:
    explicit FaceManager(QMainWindow *parent = 0);
    ~FaceManager();

private slots:
    void on_addFace_clicked();
    void on_deleteFace_clicked();
    void on_faceLibUpload_clicked();
    void on_faceLibDownload_clicked();
    //void on_faceLibUpdate_clicked();
    void on_recordBtn_clicked();

    void regInfoAnswerHandle(QByteArray);
    void itemClickedHandle(QTreeWidgetItem* item,int);

signals:
    void faceLibDownloadReq(QString);
    void faceLibUploadReq(QString);
    void recordName(QString);

private:
    void Init();
    void createFaceLibTree();
    void FaceLibTreeUpdate();


    Ui::FaceManager *ui;
    QTreeWidget* FaceLibTree;
    QTreeWidgetItem *m_chosenItem;
    MainWindow* m_parent;
    QString m_area;
    QString m_device;
    QString m_deviceIP;

    QSplitter* m_allSplitter;
    QSplitter* m_imageSplitter;
    QLabel* m_imageLabel;
    QPushButton* addFace;
    QPushButton* deleteFace;
    QPushButton* faceLibUpload;
    QPushButton* faceLibDownload;
    //QPushButton* faceLibUpdate;

    //QLabel* m_imageLabel;


};

#endif // FACEMANAGER_H
