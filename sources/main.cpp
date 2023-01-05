#include "ryzen_control.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    RyzenControl w;
    w.show();
    return a.exec();
}
