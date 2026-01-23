#ifndef HOTELMANAGER_DATABASE_H
#define HOTELMANAGER_DATABASE_H

#include <QSqlQuery>
#include <QSqlError>

class HotelManager;

class HotelManagerDatabase
{
public:
    static void initDatabase(HotelManager *manager);
    static void addMissingColumns(HotelManager *manager);

private:
    static void createTables(HotelManager *manager);
    static void createBookingsTable(HotelManager *manager);
    static void createRoomsTable(HotelManager *manager);
    static void createClientsTable(HotelManager *manager);
    static void createServicesTable(HotelManager *manager);
};

#endif // HOTELMANAGER_DATABASE_H
