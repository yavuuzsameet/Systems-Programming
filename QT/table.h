#ifndef TABLE_H
#define TABLE_H

#include <QWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMap>
#include <QVector>
#include <QString>
#include <iostream>
using namespace std;

class Table : public QWidget
{
    Q_OBJECT
public:
    Table(QMap<QString, QVector<QString>> qmap);
    ~Table();

private:
    QTableWidget *table;
};

#endif // TABLE_H
