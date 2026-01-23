#ifndef HOTELMANAGER_ROOMS_H
#define HOTELMANAGER_ROOMS_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QInputDialog>
#include <QMessageBox>

class HotelManager;

class HotelManagerRooms
{
public:
    static void addRoom(HotelManager *manager);
    static void deleteRoom(HotelManager *manager);
    static void loadRoomsFromDB(HotelManager *manager);
    static void showRoomInfo(HotelManager *manager);

private:
    struct AddRoomDialog : public QDialog {
        AddRoomDialog(QWidget *parent = nullptr);

        int roomNumber = 0;
        QString roomType = "Стандарт";
        int totalBeds = 1;
        int capacity = 2;
        double price = 3000.0;
        QString description = "";

        bool execWithValidation();

    private:
        void setupUI();
        bool validateInput();

        QLineEdit *roomNumberEdit;
        QComboBox *roomTypeCombo;
        QSpinBox *totalBedsSpin;
        QSpinBox *capacitySpin;
        QDoubleSpinBox *priceSpin;
        QTextEdit *descriptionEdit;
    };

    static bool checkRoomExists(int roomNumber, class QSqlDatabase &db);
    static QString formatRoomDisplayName(int roomNumber, const QString &roomType, int totalBeds);
};

#endif // HOTELMANAGER_ROOMS_H
