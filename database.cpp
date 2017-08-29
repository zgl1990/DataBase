/**************************************************************************
 @file          database.cpp
 @author        Galen
 @date          2017/8/28
 @brief         DataBase
 @param
 @verbatim
     <author>    <time>      <version>        <desc>
      Galen    2017/8/28      0.1.0     begin this module
 @endverbatim
**************************************************************************/
#include "database.h"
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
#include <QSqlDatabase>
#include <QSqlQuery>

using SqlKeyValue = QHash<QString,QString>;

namespace CfgKey {
const QString db_obj_key("db");
const QString host("host");
const QString driver("driver");
const QString user("user");
const QString pwd("pwd");
const QString db_file("filename");
const QString cmd_file("sqlFiles");
const QString port("port");

const QString sql_obj_key("sql");
}

namespace Json {

template <class T>
static void  writeJsonObject  ( QJsonObject &obj, const QString &key,const T &value)
{
    QJsonValue val( value );  obj.insert(key, val );
}

bool findObject(QJsonObject &obj, QJsonObject &subObj, const QString &key)
{
    QJsonValue  _val = obj.value(key);
    if ( !( _val.isNull( ) || _val.isUndefined( ) )) {
        subObj = _val.toObject(); return true;
    } else {
        return false;
    }
}

QJsonValue readJsonObject(QJsonObject &obj, const QString &key)
{
    return obj.value(key);
}

bool jsonVaild(QJsonValue val)
{
    return (! ( val.isUndefined( ) || val.isNull( )));
}

bool readToString(QJsonObject &obj, const QString &key, QString &value)
{
    QJsonValue  val = readJsonObject(obj,key); bool ret = true;
    ret = jsonVaild(val);
    if (ret) { value = val.toString(); }
    return ret;
}

bool readToBool(QJsonObject &obj, const QString &key, bool &value)
{
    QJsonValue  val = readJsonObject(obj,key); bool ret = true;
    ret = jsonVaild(val);
    if (ret) { value = val.toBool(); }
    return ret;
}

bool readToInt(QJsonObject &obj, const QString &key, int &value)
{
    QJsonValue  val = readJsonObject(obj,key); bool ret = true;
    ret = jsonVaild(val);
    if (ret) { value = val.toInt(); }
    return ret;
}

}

class Config {
public:
    Config(const QString &filename);

    bool loadFile(const QString &filename);

    QString driver() const;
    QString filename() const;
    QString user() const;
    QString passwd() const;
    int port() const;
    QString host() const;
    QString sqlfile() const;

private:
    void restore();
    bool save(const QString &filename);

    QString _driver;
    QString _filename;
    QString _user;
    QString _passwd;
    int     _port;
    QString _host;
    QString _sqlfile;
};

class DataBasePrivate {
public:
    DataBasePrivate(const QString &filename);
    void exec(const QString &sqlKey,const QVariantMap &params,DataBase::Callback callback);
private:
    void connect();
    void load(const QString &filename);
    void initSqlKeyValue();
    QString value(const QString &key) const;
    void bindValues(QSqlQuery *query, const QVariantMap &params);
    void save();

    Config       _cfg;
    QSqlDatabase _db;
    bool         _flag;
    SqlKeyValue  _sql;
};

DataBase &DataBase::instance()
{
    static DataBase self;
    return self;
}

void DataBase::exec(const QString &sqlKey, const QVariantMap &params, Callback callback)
{
    Q_D(DataBase);
    d->exec(sqlKey,params,callback);
}

DataBase::DataBase() :
    d_ptr(new DataBasePrivate("./dbconfig.json"))
{
}

DataBase::~DataBase()
{

}

Config::Config(const QString &filename) :
    _driver("QSQLITE")
  ,_filename("db.sqlite")
  ,_user("root")
  ,_passwd("root")
  ,_port(3306)
  ,_host("127.0.0.1")
  ,_sqlfile("./sql.json")
{
    if (!loadFile(filename)) {
        save(filename);
    }
}

bool Config::loadFile(const QString &filename)
{
    if (!QFile::exists(filename)) return this->save(filename);

    QFile  file(filename);
    QString text;

    if ( file.open( QIODevice::ReadOnly | QIODevice::Text )) {
        QTextStream  str( & file );
        text = str.readAll( );
        file.close( );
    } else {
        return this->save(filename);
    }

    bool ret = false;
    if ( text.length( ) > 0 ) {
        QJsonDocument json_doc = QJsonDocument::fromJson( text.toUtf8( ));
        if ( !( json_doc.isEmpty( ) || json_doc.isNull( ))) {
            QJsonObject obj = json_doc.object();
            QJsonObject db;

            if ( Json::findObject(obj, db ,CfgKey::db_obj_key) ) {
                ret = Json::readToString(db,CfgKey::host,this->_host);
                ret = Json::readToString(db,CfgKey::driver,this->_driver);
                ret = Json::readToString(db,CfgKey::user,this->_user);
                ret = Json::readToString(db,CfgKey::pwd,this->_passwd);
                ret = Json::readToString(db,CfgKey::db_file,this->_filename);
                ret = Json::readToInt(db,CfgKey::port,this->_port);
                ret = Json::readToString(db,CfgKey::cmd_file,_sqlfile);
            } else {
                ret = false;
            }
        } else {
            ret = false;
        }
    }

    return ret;
}

bool Config::save(const QString &filename)
{
    restore();
    QJsonObject rootobj,appobj;
    Json::writeJsonObject(appobj,CfgKey::host,this->_host);
    Json::writeJsonObject(appobj,CfgKey::driver,this->_driver);
    Json::writeJsonObject(appobj,CfgKey::user,this->_user);
    Json::writeJsonObject(appobj,CfgKey::pwd,this->_passwd);
    Json::writeJsonObject(appobj,CfgKey::db_file,this->_filename);
    Json::writeJsonObject(appobj,CfgKey::port,this->_port);
    Json::writeJsonObject(appobj,CfgKey::cmd_file,this->_sqlfile);
    rootobj.insert(CfgKey::db_obj_key,QJsonValue(appobj));
    QJsonDocument doc(rootobj);
    QString txt(doc.toJson());
    QFile file(filename);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream out(&file);
        out << txt;
        out.flush();
        file.close();
        return true;
    } else {
        return false;
    }
}

QString Config::sqlfile() const
{
    return _sqlfile;
}

QString Config::driver() const
{
    return _driver;
}

QString Config::filename() const
{
    return _filename;
}

QString Config::user() const
{
    return _user;
}

QString Config::passwd() const
{
    return _passwd;
}

int Config::port() const
{
    return _port;
}

QString Config::host() const
{
    return _host;
}

void Config::restore()
{
    _driver = "QSQLITE";
    _filename = "db.sqlite";
    _user = "root";
    _passwd = "root";
    _port = 3306;
    _host = "127.0.0.1";
    _sqlfile = "./sql.json";
}

DataBasePrivate::DataBasePrivate(const QString &filename) :
    _cfg(filename)
  , _flag(false)
{
    connect();
    load(_cfg.sqlfile());
}

void DataBasePrivate::exec(const QString &sqlKey, const QVariantMap &params, DataBase::Callback callback)
{
    QSqlQuery query(_db);
    query.prepare(value(sqlKey));
    bindValues(&query, params);
    if (query.exec()) {
        callback(&query);
    } else {
        callback(&query);
    }
}

void DataBasePrivate::connect()
{
    _db = QSqlDatabase::addDatabase(_cfg.driver());
    _db.setDatabaseName(_cfg.filename());
    _db.setHostName(_cfg.host());
    _db.setUserName(_cfg.user());
    _db.setPassword(_cfg.passwd());
    _db.setPort(_cfg.port());
    if (_db.isOpen()) {
        _db.commit();
        _db.close();
    }
    if (_db.open()) {
        _flag = true;
    } else {
        _flag = false;
    }
}

void DataBasePrivate::load(const QString &filename)
{
    if (!QFile::exists(filename)) return initSqlKeyValue();
    QFile  file(filename);
    QString text;

    if ( file.open( QIODevice::ReadOnly | QIODevice::Text )) {
        QTextStream  str( & file );
        text = str.readAll( );
        file.close( );
    } else {
        return initSqlKeyValue();
    }

    if ( text.length( ) > 0 ) {
        QJsonDocument json_doc = QJsonDocument::fromJson( text.toUtf8( ));
        if ( !( json_doc.isEmpty( ) || json_doc.isNull( ))) {
            QJsonObject obj = json_doc.object();
            QJsonObject sql;

            if ( Json::findObject(obj, sql ,CfgKey::sql_obj_key) ) {
                for (auto begin = sql.begin(); begin != sql.end(); begin++) {
                    _sql[begin.key()] = begin.value().toString();
                }
            } else {
                return initSqlKeyValue();
            }
        } else {
            return initSqlKeyValue();
        }
    }
}

void DataBasePrivate::initSqlKeyValue()
{
    _sql["create"] = "CREATE TABLE Persons(Id_P int,LastName varchar(255),FirstName varchar(255),Address varchar(255),City varchar(255))";
    _sql["insert"] = "INSERT INTO Persons VALUES (:id,:lastname, :firstname,:address,:city)";
    save();
}

QString DataBasePrivate::value(const QString &key) const
{
    return _sql.value(key);
}

void DataBasePrivate::bindValues(QSqlQuery *query, const QVariantMap &params)
{
    for (auto i = params.constBegin(); i != params.constEnd(); ++i) {
        query->bindValue(":" + i.key(), i.value());
    }
}

void DataBasePrivate::save()
{
    QJsonObject rootobj,appobj;
    for(auto begin = _sql.begin(); begin != _sql.end(); begin++) {
        Json::writeJsonObject(appobj,begin.key(),begin.value());
    }
    rootobj.insert(CfgKey::sql_obj_key,QJsonValue(appobj));
    QJsonDocument doc(rootobj);
    QString txt(doc.toJson());
    QFile file(_cfg.sqlfile());
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream out(&file);
        out << txt;
        out.flush();
        file.close();
    } else {

    }
}

