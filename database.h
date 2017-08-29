/**************************************************************************
 @file          database.h
 @author        Galen
 @date          2017/8/28
 @brief         DataBase
 @param
 @verbatim
     <author>    <time>      <version>        <desc>
      Galen    2017/8/28      0.1.0     begin this module
 @endverbatim
**************************************************************************/
#ifndef DATABASE_H
#define DATABASE_H

#include <QScopedPointer>
#include <QHash>
#include <QVariantMap>
#include <functional>

class QSqlQuery;
class DataBasePrivate;

class DataBase
{
    Q_DECLARE_PRIVATE(DataBase)
    using Callback = std::function<void (QSqlQuery *query)>;
public:
    static DataBase &instance();
    void exec(const QString &sqlKey,Callback callback,const QVariantMap &params = QVariantMap());
private:
    QScopedPointer<DataBasePrivate> d_ptr;
    DataBase();
    ~DataBase();
    Q_DISABLE_COPY(DataBase)
};

#endif // DATABASE_H
