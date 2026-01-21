#ifndef HOTELMANAGER_H
#define HOTELMANAGER_H

#include <QMainWindow>
#include <QTableWidget>
#include <QDate>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QAction>
#include <QMap>
#include <QSet>
#include <QPair>

QT_BEGIN_NAMESPACE
namespace Ui { class HotelManager; }
QT_END_NAMESPACE

class HotelManager : public QMainWindow
{
    Q_OBJECT

public:
    HotelManager(QWidget *parent = nullptr);
    ~HotelManager();

private slots:
    void onDateChanged();
    void onTableClicked(const QModelIndex &index);
    void addBooking();
    void removeBooking();
    void addRoom();
    void deleteRoom();
    void manageClients();
    void manageServices();
    void viewReports();

private:
    void initDatabase();
    void initMenuBar();
    void updateTableHeaders();
    void loadOccupancyFromDB();
    void loadRoomsFromDB();
    void saveOccupancyToDB(int roomNumber, const QDate &date, bool occupied);
    bool isRoomOccupied(int roomNumber, const QDate &date);
    bool isValidRoomName(const QString &name);
    QString getRoomNameFromUser(const QString &title, const QString &label, const QString &defaultValue = "");

    Ui::HotelManager *ui;
    QDate startDate;
    QSqlDatabase db;

    // Кэш занятости для быстрого доступа
    QMap<QPair<int, QDate>, bool> occupancyCache;
};
#endif // HOTELMANAGER_H
