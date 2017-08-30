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
		DataBase::instance().exec("create",[=](QSqlQuery *query){
		});
		QVariantMap m;
		m["id"] = rand();
		m["lastname"] = "lst";
		m["firstname"] = "first";
		m["address"] = "address";
		m["city"] = "chongqing";
		DataBase::instance().exec("insert",[=](QSqlQuery *query){
			qDebug() << query->executedQuery() << query->lastError().text() << query->lastError().type();
		}，m);
	  
		return a.exec();
  }
  
# other
  
