#include <QApplication>
#include "nettwork.h"

//@author: Yavuz Samet Topçuoğlu

int main(int argc,char *argv[])
{

   qputenv("QT_STYLE_OVERRIDE",0);

   QApplication a(argc, argv);
   Nettwork my ;
   return a.exec();
}
