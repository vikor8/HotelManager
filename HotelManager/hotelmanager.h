#ifndef HOTELMANAGER_H
#define HOTELMANAGER_H

#include <QMainWindow>
#include <QDate>
#include <QSqlDatabase>
#include <QMap>
#include <QPair>
#include <QStatusBar>
#include <QSqlError>  // Добавлено

QT_BEGIN_NAMESPACE
namespace Ui {
class HotelManager;
}
QT_END_NAMESPACE

class HotelManager : public QMainWindow
{
    Q_OBJECT

public:
    explicit HotelManager(QWidget *parent = nullptr);
    ~HotelManager();

    // Геттеры для доступа из других файлов
    Ui::HotelManager* getUI() const { return ui; }
    QDate getStartDate() const { return startDate; }
    QSqlDatabase& getDatabase() { return db; }
    QMap<QPair<int, QDate>, bool>& getOccupancyCache() { return occupancyCache; }

    void setStartDate(const QDate &date) { startDate = date; }
    void updateTableHeaders();

private slots:
    void onDateChanged();
    void onTableClicked(const QModelIndex &index);
    void showContextMenu(const QPoint &pos);

private:
    Ui::HotelManager *ui;
    QDate startDate;
    QSqlDatabase db;
    QMap<QPair<int, QDate>, bool> occupancyCache;
};

// Вспомогательные утилиты
namespace HotelUtils {
    int extractRoomNumber(const QString &roomText);
    QString formatDateDisplay(const QDate &date);
}

#endif // HOTELMANAGER_H
