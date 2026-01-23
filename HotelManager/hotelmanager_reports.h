#ifndef HOTELMANAGER_REPORTS_H
#define HOTELMANAGER_REPORTS_H

#include <QDialog>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>

class HotelManager;

class HotelManagerReports
{
public:
    static void viewReports(HotelManager *manager);

private:
    struct ReportDialog : public QDialog {
        ReportDialog(HotelManager *manager, QWidget *parent = nullptr);

    private:
        void generateReport(HotelManager *manager);
        QString generateStatistics(HotelManager *manager);
        QString generateUpcomingBookings(HotelManager *manager);

        QTextEdit *reportText;
    };
};

#endif // HOTELMANAGER_REPORTS_H
