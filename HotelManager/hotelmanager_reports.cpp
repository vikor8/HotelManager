#include "hotelmanager_reports.h"
#include "hotelmanager.h"
#include "hotelmanager.h"

#include <QSqlQuery>

HotelManagerReports::ReportDialog::ReportDialog(HotelManager *manager, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Отчеты и обзор");
    resize(400, 300);

    QVBoxLayout *layout = new QVBoxLayout(this);

    reportText = new QTextEdit(this);
    reportText->setReadOnly(true);
    layout->addWidget(reportText);

    generateReport(manager);

    QPushButton *closeButton = new QPushButton("Закрыть", this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);
    layout->addWidget(closeButton);
}

void HotelManagerReports::ReportDialog::generateReport(HotelManager *manager)
{
    QString report;
    report += "<h2>Отчет по отелю</h2>";
    report += "<h3>Статистика на " + QDate::currentDate().toString("dd.MM.yyyy") + "</h3>";

    report += generateStatistics(manager);
    report += generateUpcomingBookings(manager);

    reportText->setHtml(report);
}

QString HotelManagerReports::ReportDialog::generateStatistics(HotelManager *manager)
{
    QString stats;
    QSqlDatabase &db = manager->getDatabase();

    // Общее количество комнат
    QSqlQuery roomQuery(db);
    roomQuery.exec("SELECT COUNT(*) FROM rooms");
    if (roomQuery.next()) {
        stats += "<p><b>Всего комнат:</b> " + roomQuery.value(0).toString() + "</p>";
    }

    // Общее количество кроватей
    QSqlQuery bedsQuery(db);
    bedsQuery.exec("SELECT SUM(total_beds) FROM rooms");
    if (bedsQuery.next()) {
        stats += "<p><b>Всего кроватей:</b> " + bedsQuery.value(0).toString() + "</p>";
    }

    // Занятость на сегодня
    QSqlQuery occupancyQuery(db);
    occupancyQuery.prepare("SELECT COUNT(DISTINCT room_number) FROM bookings WHERE booking_date = ?");
    occupancyQuery.addBindValue(QDate::currentDate().toString("yyyy-MM-dd"));

    if (occupancyQuery.exec() && occupancyQuery.next()) {
        int occupiedToday = occupancyQuery.value(0).toInt();
        roomQuery.exec("SELECT COUNT(*) FROM rooms");
        int totalRooms = 0;
        if (roomQuery.next()) {
            totalRooms = roomQuery.value(0).toInt();
        }

        double occupancyRate = totalRooms > 0 ? (occupiedToday * 100.0 / totalRooms) : 0;

        stats += "<p><b>Занято сегодня:</b> " + QString::number(occupiedToday) + " из " +
                QString::number(totalRooms) + " (" + QString::number(occupancyRate, 'f', 1) + "%)</p>";
    }

    // Занятость кроватей на сегодня
    QSqlQuery bedsOccupancyQuery(db);
    bedsOccupancyQuery.prepare("SELECT COUNT(*) FROM bookings WHERE booking_date = ?");
    bedsOccupancyQuery.addBindValue(QDate::currentDate().toString("yyyy-MM-dd"));

    if (bedsOccupancyQuery.exec() && bedsOccupancyQuery.next()) {
        int occupiedBeds = bedsOccupancyQuery.value(0).toInt();
        bedsQuery.exec("SELECT SUM(total_beds) FROM rooms");
        int totalBeds = 0;
        if (bedsQuery.next()) {
            totalBeds = bedsQuery.value(0).toInt();
        }

        double bedsOccupancyRate = totalBeds > 0 ? (occupiedBeds * 100.0 / totalBeds) : 0;

        stats += "<p><b>Занято кроватей:</b> " + QString::number(occupiedBeds) + " из " +
                QString::number(totalBeds) + " (" + QString::number(bedsOccupancyRate, 'f', 1) + "%)</p>";
    }

    return stats;
}

QString HotelManagerReports::ReportDialog::generateUpcomingBookings(HotelManager *manager)
{
    QString bookings;
    QSqlDatabase &db = manager->getDatabase();

    bookings += "<h3>Предстоящие бронирования (7 дней)</h3>";

    QSqlQuery upcomingQuery(db);
    upcomingQuery.prepare("SELECT room_number, bed_number, booking_date FROM bookings "
                         "WHERE booking_date BETWEEN ? AND ? "
                         "ORDER BY booking_date, room_number, bed_number");

    QDate today = QDate::currentDate();
    upcomingQuery.addBindValue(today.toString("yyyy-MM-dd"));
    upcomingQuery.addBindValue(today.addDays(7).toString("yyyy-MM-dd"));

    if (upcomingQuery.exec()) {
        QMap<QDate, QStringList> bookingsByDate;
        while (upcomingQuery.next()) {
            QDate date = upcomingQuery.value(2).toDate();
            int room = upcomingQuery.value(0).toInt();
            int bed = upcomingQuery.value(1).toInt();
            bookingsByDate[date].append(QString("%1/%2").arg(room).arg(bed));
        }

        if (bookingsByDate.isEmpty()) {
            bookings += "<p>Нет предстоящих бронирований на ближайшие 7 дней</p>";
        } else {
            for (auto it = bookingsByDate.begin(); it != bookingsByDate.end(); ++it) {
                bookings += "<p><b>" + it.key().toString("dd.MM") + ":</b> " +
                           it.value().join(", ") + "</p>";
            }
        }
    }

    return bookings;
}

void HotelManagerReports::viewReports(HotelManager *manager)
{
    ReportDialog dialog(manager);
    dialog.exec();
}
