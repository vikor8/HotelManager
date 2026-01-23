#ifndef HOTELMANAGER_BOOKING_H
#define HOTELMANAGER_BOOKING_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QWidget>
#include <QSqlError>

class HotelManager;

class HotelManagerBooking
{
public:
    static void addBooking(HotelManager *manager);
    static void removeBooking(HotelManager *manager);
    static void loadOccupancyFromDB(HotelManager *manager);
    static void saveOccupancyToDB(HotelManager *manager, int roomNumber, int bedNumber,
                                 const QString &gender, const QDate &date, bool occupied);
    static bool isRoomOccupied(int roomNumber, const QDate &date, QSqlDatabase &db);
    static bool isBedOccupied(HotelManager *manager, int roomNumber, int bedNumber, const QDate &date);

private:
    struct BookingDialog : public QDialog {
        BookingDialog(HotelManager *manager, int roomNumber, const QDate &date,
                     int totalBeds, QWidget *parent = nullptr);

        int selectedBedNumber = 1;
        QString selectedGender = "";

    private:
        void setupUI(int totalBeds);
        void loadOccupiedBeds(HotelManager *manager, int roomNumber, const QDate &date);

        QSpinBox *bedSpinBox;
        QComboBox *genderCombo;
        QLabel *infoLabel;
        QStringList occupiedBeds;
    };

    static void showBedSelectionDialog(HotelManager *manager, int roomNumber,
                                      const QDate &date, int totalBeds,
                                      int &bedNumber, QString &gender);
};

#endif // HOTELMANAGER_BOOKING_H
