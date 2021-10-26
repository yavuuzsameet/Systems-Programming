#include <QApplication>
#include "nettwork.h"

//@author: Ali Kaan Biber - 2018400069
//@author: Yavuz Samet Topçuoğlu - 2019400285

int main(int argc,char *argv[])
{

   qputenv("QT_STYLE_OVERRIDE",0);

   QApplication a(argc, argv);
   Nettwork my ;
   return a.exec();
}
