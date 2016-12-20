#ifndef CONNECTDB
#define CONNECTDB

#include <QtSql>
#include <QDebug>

bool connectDB()
{
    QSqlDatabase db=QSqlDatabase::addDatabase("QSQLITE");

    db.setHostName("localhost");
    db.setDatabaseName("data.db");
    db.setUserName("root");
    db.setPassword("call818");
    if(!db.open()){
        return false;
    }

    return true;
}



#endif // CONNECTDB

