# DataBase

# How to use
  1.import head and source into you application project.
  or
  2.new a dll project generate the library,import head and the library into you application project

# Example
	#include <QCoreApplication>
	#include <QSqlQuery>
	#include "database.h"

	int main(int argc, char *argv[])
	{
		QCoreApplication a(argc, argv);
		DataBase::instance().exec("create",QVariantMap(),[=](QSqlQuery *query){
		});
		QVariantMap m;
		m["id"] = rand();
		m["lastname"] = "lst";
		m["firstname"] = "first";
		m["address"] = "shangdinggongyuan";
		m["city"] = "chongqing tongnan";
		DataBase::instance().exec("insert",m,[=](QSqlQuery *query){
			qDebug() << query->executedQuery() << query->lastError().text() << query->lastError().type();
		});
	  
		return a.exec();
  }
  
# other
  