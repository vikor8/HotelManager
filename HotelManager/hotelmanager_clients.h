#ifndef HOTELMANAGER_CLIENTS_H
#define HOTELMANAGER_CLIENTS_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QInputDialog>
#include <QMessageBox>

class HotelManager;

class HotelManagerClients
{
public:
    static void manageClients(HotelManager *manager);
    static void manageServices(HotelManager *manager);

private:
    struct ClientsDialog : public QDialog {
        ClientsDialog(HotelManager *manager, QWidget *parent = nullptr);

    private:
        void loadClients();
        void addClient();

        HotelManager *manager;
        QTableWidget *clientsTable;
    };

    struct ServicesDialog : public QDialog {
        ServicesDialog(HotelManager *manager, QWidget *parent = nullptr);

    private:
        void loadServices();
        void addService();

        HotelManager *manager;
        QTableWidget *servicesTable;
    };
};

#endif // HOTELMANAGER_CLIENTS_H
