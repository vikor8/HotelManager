#include "hotelmanager_clients.h"
#include "hotelmanager.h"
#include "hotelmanager.h"

#include <QHeaderView>
#include <QSqlQuery>
#include <QSqlError>

HotelManagerClients::ClientsDialog::ClientsDialog(HotelManager *manager, QWidget *parent)
    : QDialog(parent), manager(manager)
{
    setWindowTitle("Управление клиентами");
    resize(600, 400);

    QVBoxLayout *layout = new QVBoxLayout(this);

    clientsTable = new QTableWidget(this);
    clientsTable->setColumnCount(5);
    clientsTable->setHorizontalHeaderLabels(
        QStringList() << "ID" << "ФИО" << "Телефон" << "Email" << "Паспорт");
    clientsTable->horizontalHeader()->setStretchLastSection(true);

    loadClients();
    layout->addWidget(clientsTable);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *addButton = new QPushButton("Добавить клиента", this);
    QPushButton *closeButton = new QPushButton("Закрыть", this);

    buttonLayout->addWidget(addButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    layout->addLayout(buttonLayout);

    connect(addButton, &QPushButton::clicked, this, &ClientsDialog::addClient);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);
}

void HotelManagerClients::ClientsDialog::loadClients()
{
    QSqlQuery query(manager->getDatabase());
    query.exec("SELECT id, full_name, phone, email, passport FROM clients ORDER BY full_name");

    clientsTable->setRowCount(0);
    int row = 0;

    while (query.next()) {
        clientsTable->insertRow(row);
        for (int col = 0; col < 5; col++) {
            clientsTable->setItem(row, col,
                new QTableWidgetItem(query.value(col).toString()));
        }
        row++;
    }
}

void HotelManagerClients::ClientsDialog::addClient()
{
    bool ok;
    QString name = QInputDialog::getText(this, "Добавить клиента",
                                       "Введите ФИО:", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) return;

    QString phone = QInputDialog::getText(this, "Телефон",
                                        "Введите телефон:", QLineEdit::Normal, "", &ok);
    if (!ok) return;

    QString email = QInputDialog::getText(this, "Email",
                                        "Введите email:", QLineEdit::Normal, "", &ok);
    if (!ok) return;

    QString passport = QInputDialog::getText(this, "Паспорт",
                                           "Введите паспортные данные:", QLineEdit::Normal, "", &ok);
    if (!ok) return;

    QSqlQuery query(manager->getDatabase());
    query.prepare("INSERT INTO clients (full_name, phone, email, passport) "
                 "VALUES (?, ?, ?, ?)");
    query.addBindValue(name);
    query.addBindValue(phone);
    query.addBindValue(email);
    query.addBindValue(passport);

    if (query.exec()) {
        loadClients();
    } else {
        QMessageBox::warning(this, "Ошибка",
            "Не удалось добавить клиента: " + query.lastError().text());
    }
}

HotelManagerClients::ServicesDialog::ServicesDialog(HotelManager *manager, QWidget *parent)
    : QDialog(parent), manager(manager)
{
    setWindowTitle("Управление услугами");
    resize(500, 300);

    QVBoxLayout *layout = new QVBoxLayout(this);

    servicesTable = new QTableWidget(this);
    servicesTable->setColumnCount(3);
    servicesTable->setHorizontalHeaderLabels(
        QStringList() << "Услуга" << "Цена" << "Описание");
    servicesTable->horizontalHeader()->setStretchLastSection(true);

    loadServices();
    layout->addWidget(servicesTable);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *addButton = new QPushButton("Добавить услугу", this);
    QPushButton *closeButton = new QPushButton("Закрыть", this);

    buttonLayout->addWidget(addButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    layout->addLayout(buttonLayout);

    connect(addButton, &QPushButton::clicked, this, &ServicesDialog::addService);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);
}

void HotelManagerClients::ServicesDialog::loadServices()
{
    QSqlQuery query(manager->getDatabase());
    query.exec("SELECT service_name, price, description FROM services");

    servicesTable->setRowCount(0);
    int row = 0;

    while (query.next()) {
        servicesTable->insertRow(row);
        servicesTable->setItem(row, 0,
            new QTableWidgetItem(query.value(0).toString()));
        servicesTable->setItem(row, 1,
            new QTableWidgetItem(QString::number(query.value(1).toDouble())));
        servicesTable->setItem(row, 2,
            new QTableWidgetItem(query.value(2).toString()));
        row++;
    }
}

void HotelManagerClients::ServicesDialog::addService()
{
    bool ok;
    QString name = QInputDialog::getText(this, "Добавить услугу",
                                       "Название услуги:", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) return;

    double price = QInputDialog::getDouble(this, "Цена",
                                          "Цена услуги:", 0, 0, 100000, 2, &ok);
    if (!ok) return;

    QString description = QInputDialog::getText(this, "Описание",
                                              "Описание услуги:", QLineEdit::Normal, "", &ok);
    if (!ok) return;

    QSqlQuery query(manager->getDatabase());
    query.prepare("INSERT INTO services (service_name, price, description) "
                 "VALUES (?, ?, ?)");
    query.addBindValue(name);
    query.addBindValue(price);
    query.addBindValue(description);

    if (query.exec()) {
        loadServices();
    } else {
        QMessageBox::warning(this, "Ошибка",
            "Не удалось добавить услугу: " + query.lastError().text());
    }
}

void HotelManagerClients::manageClients(HotelManager *manager)
{
    ClientsDialog dialog(manager);
    dialog.exec();
}

void HotelManagerClients::manageServices(HotelManager *manager)
{
    ServicesDialog dialog(manager);
    dialog.exec();
}
