#include "hotelmanager.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    HotelManager w;
    w.show();
    return a.exec();
}
