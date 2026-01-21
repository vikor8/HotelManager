#ifndef HOTELMANAGER_H
#define HOTELMANAGER_H

#include <QMainWindow>
#include <QTableWidget>
#include <QDate>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

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

private:
    void initDatabase();
    void updateTableHeaders();
    void loadOccupancyFromDB();
    void saveOccupancyToDB(int roomNumber, const QDate &date, bool occupied);
    bool isRoomOccupied(int roomNumber, const QDate &date);

    Ui::HotelManager *ui;
    QDate startDate;
    QSqlDatabase db;

    // Кэш занятости для быстрого доступа
    QMap<QPair<int, QDate>, bool> occupancyCache;
};
#endif // HOTELMANAGER_H
