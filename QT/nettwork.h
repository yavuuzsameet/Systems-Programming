#ifndef NETTWORK_H
#define NETTWORK_H

#include "table.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QLabel>
#include <QWidget>
#include <QTableWidget>
#include <string>
#include <QString>
#include <regex>
#include <QMap>
#include <iostream>
#include <fstream>

using namespace std;

class Nettwork : public QLabel
{
    Q_OBJECT

    public:
        Nettwork(QWidget *parent = 0);

    public slots:
        void replyFinished(QNetworkReply *reply);
        void listReplyFinished(QNetworkReply *listReply);

    private:
        QLabel *label;
        QTableWidget *table;
        QNetworkAccessManager *manager;
        QNetworkAccessManager *list_manager;

};

#endif // NETTWORK_H
