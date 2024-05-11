#include "cryptorwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CryptorWindow w;
    w.show();
    return a.exec();
}
