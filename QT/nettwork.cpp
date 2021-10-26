#include "nettwork.h"

Nettwork::Nettwork(QWidget *parent) : QLabel(parent)
{
    manager = new QNetworkAccessManager(this);  //network manager for id-currency request.
    list_manager = new QNetworkAccessManager(this); //network manager for coin-list request.

    connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(replyFinished(QNetworkReply *)));    //connection to replyFinished method.
    connect(list_manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(listReplyFinished(QNetworkReply *)));   //connection to listReplyFinished metohd.

    list_manager->get(QNetworkRequest(QUrl("https://api.coingecko.com/api/v3/coins/list")));    //force manager to get request.

}

void Nettwork::replyFinished(QNetworkReply *reply){
    QByteArray read = reply->readAll();
    string line = read.toStdString();   //return of http-id request.

    QMap<QString, QVector<QString>> qmap;   //store coin-id and currencies in a map.
    regex reg("\"([\\w\\-]+)\":\\{\"usd\":([\\d\\.e\\-]+),\"eur\":([\\d\\.e\\-]+),\"gbp\":([\\d\\.e\\-]+)}");
    smatch matches;
    while(regex_search(line, matches, reg)){ //while there is a match.
        QVector<QString> currency;
        QString coin = QString::fromStdString(matches.str(1));
        QString usd = QString::fromStdString(matches.str(2));
        QString eur = QString::fromStdString(matches.str(3));
        QString gbp = QString::fromStdString(matches.str(4));
        currency.append(usd);
        currency.append(eur);
        currency.append(gbp);
        qmap.insert(coin, currency); //insert coin-id and currency vector to the map.
        line = matches.suffix().str(); //rip off used part.
    }

    Table *table = new Table(qmap); //create table with respect to map.
    table->show();
}

void Nettwork::listReplyFinished(QNetworkReply *list_reply){
    QByteArray red = list_reply->readAll();
    string paragraph = red.toStdString(); //return of http-list request.

    QMap<QString, QString> id_name_symbol; //store name or symbol and id in a map.
    regex list_reg("\\{\"id\"\\:\"([A-Za-z0-9\\-\\_\\.\\s]+)\",\"symbol\"\\:\"([A-Za-z0-9\\-\\_\\.\\s]+)\",\"name\"\\:\"([A-Za-z0-9\\-\\_\\.\\s]+)\"}");
    smatch list_matches;
    while(regex_search(paragraph, list_matches, list_reg)){
        QString id = QString::fromStdString(list_matches.str(1));
        QString name = QString::fromStdString(list_matches.str(3));
        QString symbol = QString::fromStdString(list_matches.str(2));
        id_name_symbol.insert(name, id);    //insert name and id to the map.
        id_name_symbol.insert(symbol, id);  //insert symbol and id to the map.
        paragraph = list_matches.suffix().str(); //rip off used part.
    }

    const char* file = getenv("MYCRYPTOCONVERT"); //retrieve file name from environment variable.
    ifstream input(file);
    string names;
    string append;
    while(getline(input, append)){ //while there is a line in input.
        string coin_id;
        if(id_name_symbol.contains(QString::fromStdString(append))){ //if map contains the symbol or name,
            coin_id = id_name_symbol[QString::fromStdString(append)].toStdString(); //retrieve its id.
            names += coin_id + ",";
        }
    }

    string coins = names.substr(0, names.length()-1); //obtain all coin ids seperated by comma.
    string url = "https://api.coingecko.com/api/v3/simple/price?ids=" + coins + "&vs_currencies=usd,eur,gbp";
    QString urll = QString::fromStdString(url);

    manager->get(QNetworkRequest(QUrl(urll)));  //force manager to get request.
}
