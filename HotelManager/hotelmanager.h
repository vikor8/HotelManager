#ifndef HOTELMANAGER_H
#define HOTELMANAGER_H

#include <QMainWindow>
#include <QTableWidget>
#include <QDate>

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
    void updateTableHeaders();
    void fillSampleData();

private:
    Ui::HotelManager *ui;
    QDate startDate;
    QMap<int, QSet<QDate>> roomOccupancy; // ключ - номер комнаты, значение - множество занятых дат
};
#endif // HOTELMANAGER_H
