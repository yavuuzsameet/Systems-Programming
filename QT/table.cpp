#include "table.h"

Table::Table(QMap<QString, QVector<QString>> qmap) : QWidget()
{
    QTableWidget *table = new QTableWidget(qmap.size(), 3); //decide the size of table.

    QMap<QString, QVector<QString>>::iterator i;
    int count = 0;
    for(i = qmap.begin(); i != qmap.end(); ++i){ //for every row

        QTableWidgetItem *coinname = new QTableWidgetItem(i.key().toUpper());
        table->setVerticalHeaderItem(count, coinname); //put coin-id to the header.

        for (int j = 0; j < 3; j++) { //for every column
            QTableWidgetItem *item = new QTableWidgetItem(i.value()[j]);
            table->setItem(count, j, item); //put item to the table.
        }

        count++; //increase row count.

    }

    //put currency names to the headers.
    QTableWidgetItem *usd = new QTableWidgetItem("USD");
    QTableWidgetItem *eur = new QTableWidgetItem("EUR");
    QTableWidgetItem *gbp = new QTableWidgetItem("GBP");
    table->setHorizontalHeaderItem(0, usd);
    table->setHorizontalHeaderItem(1, eur);
    table->setHorizontalHeaderItem(2, gbp);

    table->resizeColumnsToContents();
    table->resizeRowsToContents();



    this->resize(1080,720);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(table); //add table to window.

    setLayout(layout);
}
 Table::~Table() { //destructor.
     delete table;
}


