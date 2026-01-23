#include "hotelmanager_database.h"
#include "hotelmanager.h"

#include <QMessageBox>
#include <QDebug>
#include <QStatusBar>

void HotelManagerDatabase::initDatabase(HotelManager *manager)
{
    QSqlDatabase &db = manager->getDatabase();
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("hotel_bookings.db");

    if (!db.open()) {
        QMessageBox::critical(manager, "Ошибка",
            "Не удалось открыть базу данных: " + db.lastError().text());
        return;
    }

    createTables(manager);
    addMissingColumns(manager);

    manager->statusBar()->showMessage("База данных подключена", 3000);
}

void HotelManagerDatabase::createTables(HotelManager *manager)
{
    createBookingsTable(manager);
    createRoomsTable(manager);
    createClientsTable(manager);
    createServicesTable(manager);
}

void HotelManagerDatabase::createBookingsTable(HotelManager *manager)
{
    QSqlDatabase &db = manager->getDatabase();
    QSqlQuery query(db);

    QString sql =
        "CREATE TABLE IF NOT EXISTS bookings ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "room_number INTEGER NOT NULL, "
        "bed_number INTEGER DEFAULT 1, "
        "gender TEXT CHECK(gender IN ('М', 'Ж', '')), "
        "booking_date DATE NOT NULL, "
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
        "UNIQUE(room_number, bed_number, booking_date)"
        ")";

    if (!query.exec(sql)) {
        qDebug() << "Ошибка создания таблицы бронирований:" << query.lastError().text();
    }
}

void HotelManagerDatabase::createRoomsTable(HotelManager *manager)
{
    QSqlDatabase &db = manager->getDatabase();
    QSqlQuery query(db);

    QString sql =
        "CREATE TABLE IF NOT EXISTS rooms ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "room_number INTEGER UNIQUE NOT NULL, "
        "room_type TEXT DEFAULT 'Стандарт', "
        "total_beds INTEGER DEFAULT 1, "
        "capacity INTEGER DEFAULT 2, "
        "price_per_night REAL DEFAULT 3000.0, "
        "description TEXT, "
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ")";

    if (!query.exec(sql)) {
        qDebug() << "Ошибка создания таблицы комнат:" << query.lastError().text();
    }
}

void HotelManagerDatabase::createClientsTable(HotelManager *manager)
{
    QSqlDatabase &db = manager->getDatabase();
    QSqlQuery query(db);

    QString sql =
        "CREATE TABLE IF NOT EXISTS clients ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "full_name TEXT NOT NULL, "
        "phone TEXT, "
        "email TEXT, "
        "passport TEXT, "
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ")";

    if (!query.exec(sql)) {
        qDebug() << "Ошибка создания таблицы клиентов:" << query.lastError().text();
    }
}

void HotelManagerDatabase::createServicesTable(HotelManager *manager)
{
    QSqlDatabase &db = manager->getDatabase();
    QSqlQuery query(db);

    QString sql =
        "CREATE TABLE IF NOT EXISTS services ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "service_name TEXT NOT NULL, "
        "price REAL DEFAULT 0.0, "
        "description TEXT"
        ")";

    if (!query.exec(sql)) {
        qDebug() << "Ошибка создания таблицы услуг:" << query.lastError().text();
    }
}

void HotelManagerDatabase::addMissingColumns(HotelManager *manager)
{
    QSqlDatabase &db = manager->getDatabase();
    QSqlQuery query(db);

    query.exec("PRAGMA table_info(bookings)");
    bool hasBedNumber = false;
    bool hasGender = false;
    while (query.next()) {
        QString columnName = query.value(1).toString();
        if (columnName == "bed_number") hasBedNumber = true;
        if (columnName == "gender") hasGender = true;
    }

    if (!hasBedNumber) {
        query.exec("ALTER TABLE bookings ADD COLUMN bed_number INTEGER DEFAULT 1");
    }
    if (!hasGender) {
        query.exec("ALTER TABLE bookings ADD COLUMN gender TEXT CHECK(gender IN ('М', 'Ж', ''))");
    }

    query.exec("PRAGMA table_info(rooms)");
    bool hasTotalBeds = false;
    while (query.next()) {
        if (query.value(1).toString() == "total_beds") hasTotalBeds = true;
    }

    if (!hasTotalBeds) {
        query.exec("ALTER TABLE rooms ADD COLUMN total_beds INTEGER DEFAULT 1");
    }
}
