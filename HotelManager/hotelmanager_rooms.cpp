#include "hotelmanager_rooms.h"
#include "hotelmanager.h"
#include "ui_hotelmanager.h"
#include "hotelmanager_ui.h"

#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QTableWidgetItem>
#include <QIntValidator>

void HotelManagerRooms::addRoom(HotelManager *manager)
{
    AddRoomDialog dialog;

    if (dialog.execWithValidation()) {
        // Проверяем существование комнаты
        if (checkRoomExists(dialog.roomNumber, manager->getDatabase())) {
            QMessageBox::warning(manager, "Ошибка",
                QString("Комната с номером %1 уже существует!").arg(dialog.roomNumber));
            return;
        }

        // Добавляем комнату в БД
        QSqlQuery query(manager->getDatabase());
        query.prepare("INSERT INTO rooms (room_number, room_type, total_beds, capacity, price_per_night, description) "
                     "VALUES (?, ?, ?, ?, ?, ?)");
        query.addBindValue(dialog.roomNumber);
        query.addBindValue(dialog.roomType);
        query.addBindValue(dialog.totalBeds);
        query.addBindValue(dialog.capacity);
        query.addBindValue(dialog.price);
        query.addBindValue(dialog.description);

        if (query.exec()) {
            loadRoomsFromDB(manager);
            manager->updateTableHeaders();

            QMessageBox::information(manager, "Успех",
                QString("Комната %1 (%2) успешно добавлена!\n"
                       "Количество кроватей: %3")
                    .arg(dialog.roomNumber)
                    .arg(dialog.roomType)
                    .arg(dialog.totalBeds));

            manager->statusBar()->showMessage(
                QString("Добавлена комната %1").arg(dialog.roomNumber), 3000);
        } else {
            QMessageBox::critical(manager, "Ошибка",
                "Не удалось добавить комнату: " + query.lastError().text());
        }
    }
}

void HotelManagerRooms::deleteRoom(HotelManager *manager)
{
    // Получаем список всех комнат
    QSqlQuery query(manager->getDatabase());
    query.exec("SELECT room_number, room_type, total_beds FROM rooms ORDER BY room_number");

    QStringList rooms;
    QMap<QString, int> roomMap;

    while (query.next()) {
        int roomNumber = query.value(0).toInt();
        QString roomType = query.value(1).toString();
        int totalBeds = query.value(2).toInt();

        rooms.append(formatRoomDisplayName(roomNumber, roomType, totalBeds));
        roomMap[rooms.last()] = roomNumber;
    }

    if (rooms.isEmpty()) {
        QMessageBox::information(manager, "Удаление комнаты", "Нет комнат для удаления");
        return;
    }

    bool ok;
    QString selectedRoom = QInputDialog::getItem(manager, "Удалить комнату",
                                                "Выберите комнату для удаления:",
                                                rooms, 0, false, &ok);

    if (!ok || selectedRoom.isEmpty()) return;

    int roomNumber = roomMap[selectedRoom];

    // Проверяем активные бронирования
    QSqlQuery checkQuery(manager->getDatabase());
    checkQuery.prepare("SELECT COUNT(*) FROM bookings WHERE room_number = ? AND booking_date >= ?");
    checkQuery.addBindValue(roomNumber);
    checkQuery.addBindValue(QDate::currentDate().toString("yyyy-MM-dd"));

    int activeBookings = 0;
    if (checkQuery.exec() && checkQuery.next()) {
        activeBookings = checkQuery.value(0).toInt();
    }

    if (activeBookings > 0) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            manager, "Подтверждение удаления",
            QString("У комнаты %1 есть активные бронирования (%2 шт.).\n"
                   "Все бронирования будут удалены. Продолжить?")
                   .arg(selectedRoom).arg(activeBookings),
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply != QMessageBox::Yes) {
            return;
        }
    }

    // Удаляем бронирования комнаты
    QSqlQuery deleteBookingsQuery(manager->getDatabase());
    deleteBookingsQuery.prepare("DELETE FROM bookings WHERE room_number = ?");
    deleteBookingsQuery.addBindValue(roomNumber);

    if (!deleteBookingsQuery.exec()) {
        QMessageBox::warning(manager, "Ошибка", "Не удалось удалить бронирования комнаты");
        return;
    }

    // Удаляем комнату
    QSqlQuery deleteRoomQuery(manager->getDatabase());
    deleteRoomQuery.prepare("DELETE FROM rooms WHERE room_number = ?");
    deleteRoomQuery.addBindValue(roomNumber);

    if (deleteRoomQuery.exec()) {
        // Очищаем кэш
        QMap<QPair<int, QDate>, bool> &cache = manager->getOccupancyCache();
        QList<QPair<int, QDate>> keysToRemove;

        for (auto it = cache.begin(); it != cache.end(); ++it) {
            if (it.key().first / 100 == roomNumber) {
                keysToRemove.append(it.key());
            }
        }

        for (const auto &key : keysToRemove) {
            cache.remove(key);
        }

        loadRoomsFromDB(manager);
        manager->updateTableHeaders();

        QMessageBox::information(manager, "Успех",
            QString("Комната %1 успешно удалена!").arg(selectedRoom));

        manager->statusBar()->showMessage(
            QString("Удалена комната %1").arg(selectedRoom), 3000);
    } else {
        QMessageBox::critical(manager, "Ошибка",
            "Не удалось удалить комнату: " + deleteRoomQuery.lastError().text());
    }
}

void HotelManagerRooms::loadRoomsFromDB(HotelManager *manager)
{
    Ui::HotelManager *ui = manager->getUI();

    QSqlQuery query(manager->getDatabase());
    query.exec("SELECT room_number, room_type, total_beds FROM rooms ORDER BY room_number");

    int row = 0;
    ui->tableWidget->setRowCount(0);

    while (query.next()) {
        int roomNumber = query.value(0).toInt();
        QString roomType = query.value(1).toString();
        int totalBeds = query.value(2).toInt();

        QString displayName = formatRoomDisplayName(roomNumber, roomType, totalBeds);

        ui->tableWidget->insertRow(row);
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(displayName));
        row++;
    }
}

void HotelManagerRooms::showRoomInfo(HotelManager *manager)
{
    Ui::HotelManager *ui = manager->getUI();

    QModelIndexList selected = ui->tableWidget->selectionModel()->selectedIndexes();
    if (!selected.isEmpty()) {
        int row = selected.first().row();
        QString roomText = ui->tableWidget->item(row, 0)->text();
        int roomNumber = HotelUtils::extractRoomNumber(roomText);

        QSqlQuery query(manager->getDatabase());
        query.prepare("SELECT room_type, total_beds, capacity, price_per_night, description FROM rooms WHERE room_number = ?");
        query.addBindValue(roomNumber);

        if (query.exec() && query.next()) {
            QString info = QString("Номер: %1\n"
                                  "Тип: %2\n"
                                  "Кроватей: %3\n"
                                  "Вместимость: %4 чел.\n"
                                  "Цена за ночь: %5 руб.\n"
                                  "Описание: %6")
                          .arg(roomNumber)
                          .arg(query.value(0).toString())
                          .arg(query.value(1).toInt())
                          .arg(query.value(2).toInt())
                          .arg(query.value(3).toDouble())
                          .arg(query.value(4).toString());

            QMessageBox::information(manager, "Информация о номере", info);
        }
    }
}

bool HotelManagerRooms::checkRoomExists(int roomNumber, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM rooms WHERE room_number = ?");
    query.addBindValue(roomNumber);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

QString HotelManagerRooms::formatRoomDisplayName(int roomNumber, const QString &roomType, int totalBeds)
{
    QString displayName = QString("Комната %1").arg(roomNumber);

    if (totalBeds > 1) {
        displayName += QString(" (%1 кр.)").arg(totalBeds);
    }

    if (roomType != "Стандарт") {
        if (displayName.contains("(")) {
            displayName += QString(" %1").arg(roomType);
        } else {
            displayName += QString(" (%1)").arg(roomType);
        }
    }

    return displayName;
}

// Реализация диалога добавления комнаты
HotelManagerRooms::AddRoomDialog::AddRoomDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Добавить комнату");
    resize(400, 300);
    setupUI();
}

void HotelManagerRooms::AddRoomDialog::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    // Номер комнаты
    QHBoxLayout *roomNumLayout = new QHBoxLayout();
    roomNumLayout->addWidget(new QLabel("Номер комнаты:", this));
    roomNumberEdit = new QLineEdit(this);
    roomNumberEdit->setValidator(new QIntValidator(1, 999, this));
    roomNumLayout->addWidget(roomNumberEdit);
    layout->addLayout(roomNumLayout);

    // Тип комнаты
    QHBoxLayout *typeLayout = new QHBoxLayout();
    typeLayout->addWidget(new QLabel("Тип комнаты:", this));
    roomTypeCombo = new QComboBox(this);
    roomTypeCombo->addItems(QStringList() << "Стандарт" << "Люкс" << "Семейный" << "Бизнес" << "Апартаменты");
    typeLayout->addWidget(roomTypeCombo);
    layout->addLayout(typeLayout);

    // Количество кроватей
    QHBoxLayout *bedsLayout = new QHBoxLayout();
    bedsLayout->addWidget(new QLabel("Количество кроватей:", this));
    totalBedsSpin = new QSpinBox(this);
    totalBedsSpin->setRange(1, 10);
    totalBedsSpin->setValue(1);
    bedsLayout->addWidget(totalBedsSpin);
    layout->addLayout(bedsLayout);

    // Вместимость
    QHBoxLayout *capacityLayout = new QHBoxLayout();
    capacityLayout->addWidget(new QLabel("Вместимость (чел.):", this));
    capacitySpin = new QSpinBox(this);
    capacitySpin->setRange(1, 20);
    capacitySpin->setValue(2);
    capacityLayout->addWidget(capacitySpin);
    layout->addLayout(capacityLayout);

    // Цена
    QHBoxLayout *priceLayout = new QHBoxLayout();
    priceLayout->addWidget(new QLabel("Цена за ночь:", this));
    priceSpin = new QDoubleSpinBox(this);
    priceSpin->setRange(500.0, 50000.0);
    priceSpin->setValue(3000.0);
    priceSpin->setDecimals(2);
    priceSpin->setSuffix(" руб.");
    priceLayout->addWidget(priceSpin);
    layout->addLayout(priceLayout);

    // Описание
    layout->addWidget(new QLabel("Описание:", this));
    descriptionEdit = new QTextEdit(this);
    descriptionEdit->setMaximumHeight(100);
    layout->addWidget(descriptionEdit);

    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("OK", this);
    QPushButton *cancelButton = new QPushButton("Отмена", this);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    connect(okButton, &QPushButton::clicked, this, [this]() {
        if (validateInput()) {
            accept();
        }
    });

    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

bool HotelManagerRooms::AddRoomDialog::validateInput()
{
    if (roomNumberEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите номер комнаты");
        return false;
    }

    roomNumber = roomNumberEdit->text().toInt();
    roomType = roomTypeCombo->currentText();
    totalBeds = totalBedsSpin->value();
    capacity = capacitySpin->value();
    price = priceSpin->value();
    description = descriptionEdit->toPlainText().trimmed();

    if (capacity < totalBeds) {
        QMessageBox::warning(this, "Ошибка",
            "Вместимость не может быть меньше количества кроватей");
        return false;
    }

    return true;
}

bool HotelManagerRooms::AddRoomDialog::execWithValidation()
{
    if (exec() == QDialog::Accepted) {
        return true;
    }
    return false;
}
