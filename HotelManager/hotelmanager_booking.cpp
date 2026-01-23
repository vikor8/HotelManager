#include "hotelmanager_booking.h"
#include "hotelmanager.h"
#include "ui_hotelmanager.h"
#include "hotelmanager_ui.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QMessageBox>
#include <QDate>
#include <QVariant>

#include <QDebug>
#include <QSqlQuery>

void HotelManagerBooking::addBooking(HotelManager *manager)
{
    Ui::HotelManager *ui = manager->getUI();

    QModelIndexList selected = ui->tableWidget->selectionModel()->selectedIndexes();
    if (selected.isEmpty()) {
        QMessageBox::information(manager, "Бронирование", "Выберите ячейку в таблице");
        return;
    }

    QModelIndex index = selected.first();
    int row = index.row();
    int col = index.column();

    if (col == 0) return;

    QString roomText = ui->tableWidget->item(row, 0)->text();
    int roomNumber = HotelUtils::extractRoomNumber(roomText);
    QDate cellDate = manager->getStartDate().addDays(col - 1);

    // Проверяем информацию о комнате
    QSqlQuery roomQuery(manager->getDatabase());
    roomQuery.prepare("SELECT total_beds FROM rooms WHERE room_number = ?");
    roomQuery.addBindValue(roomNumber);

    int totalBeds = 1;
    if (roomQuery.exec() && roomQuery.next()) {
        totalBeds = roomQuery.value(0).toInt();
    }

    int bedNumber = 1;
    QString gender = "";

    // Если несколько кроватей - показываем диалог выбора
    if (totalBeds > 1) {
        showBedSelectionDialog(manager, roomNumber, cellDate, totalBeds, bedNumber, gender);
    }

    // Сохраняем бронирование
    if (bedNumber > 0) {
        saveOccupancyToDB(manager, roomNumber, bedNumber, gender, cellDate, true);
        manager->updateTableHeaders();

        QString bedInfo = (totalBeds > 1) ?
            QString(" (кровать %1%2)").arg(bedNumber).arg(gender.isEmpty() ? "" : gender) : "";

        QMessageBox::information(manager, "Успех",
            QString("Комната %1%2 забронирована на %3")
                .arg(roomNumber)
                .arg(bedInfo)
                .arg(cellDate.toString("dd.MM.yyyy")));
    }
}

void HotelManagerBooking::removeBooking(HotelManager *manager)
{
    Ui::HotelManager *ui = manager->getUI();

    QModelIndexList selected = ui->tableWidget->selectionModel()->selectedIndexes();
    if (selected.isEmpty()) {
        QMessageBox::information(manager, "Снять бронь", "Выберите занятую ячейку");
        return;
    }

    QModelIndex index = selected.first();
    int row = index.row();
    int col = index.column();

    if (col == 0) return;

    QString roomText = ui->tableWidget->item(row, 0)->text();
    int roomNumber = HotelUtils::extractRoomNumber(roomText);
    QDate cellDate = manager->getStartDate().addDays(col - 1);

    // Получаем информацию о бронировании
    QSqlQuery bookingQuery(manager->getDatabase());
    bookingQuery.prepare("SELECT bed_number, gender FROM bookings WHERE room_number = ? AND booking_date = ?");
    bookingQuery.addBindValue(roomNumber);
    bookingQuery.addBindValue(cellDate.toString("yyyy-MM-dd"));

    int bedNumber = 1;
    QString gender = "";

    if (bookingQuery.exec() && bookingQuery.next()) {
        bedNumber = bookingQuery.value(0).toInt();
        gender = bookingQuery.value(1).toString();
    }

    // Удаляем бронирование
    saveOccupancyToDB(manager, roomNumber, bedNumber, gender, cellDate, false);
    manager->updateTableHeaders();

    QString bedInfo = (bedNumber > 0) ?
        QString(" (кровать %1%2)").arg(bedNumber).arg(gender.isEmpty() ? "" : gender) : "";

    QMessageBox::information(manager, "Успех",
        QString("Бронь комнаты %1%2 на %3 снята")
            .arg(roomNumber)
            .arg(bedInfo)
            .arg(cellDate.toString("dd.MM.yyyy")));
}

void HotelManagerBooking::loadOccupancyFromDB(HotelManager *manager)
{
    QMap<QPair<int, QDate>, bool> &cache = manager->getOccupancyCache();
    cache.clear();

    QSqlQuery query(manager->getDatabase());
    query.prepare("SELECT room_number, bed_number, booking_date FROM bookings");

    if (query.exec()) {
        while (query.next()) {
            int roomNumber = query.value(0).toInt();
            int bedNumber = query.value(1).toInt();
            QDate date = query.value(2).toDate();

            cache[qMakePair(roomNumber * 100 + bedNumber, date)] = true;
        }
    } else {
        qDebug() << "Ошибка загрузки данных:" << query.lastError().text();
    }
}

void HotelManagerBooking::saveOccupancyToDB(HotelManager *manager, int roomNumber, int bedNumber,
                                           const QString &gender, const QDate &date, bool occupied)
{
    QSqlDatabase &db = manager->getDatabase();
    QMap<QPair<int, QDate>, bool> &cache = manager->getOccupancyCache();

    if (occupied) {
        QSqlQuery query(db);
        query.prepare("INSERT OR REPLACE INTO bookings (room_number, bed_number, gender, booking_date) VALUES (?, ?, ?, ?)");
        query.addBindValue(roomNumber);
        query.addBindValue(bedNumber);
        query.addBindValue(gender);
        query.addBindValue(date.toString("yyyy-MM-dd"));

        if (!query.exec()) {
            QMessageBox::warning(manager, "Ошибка",
                "Не удалось сохранить бронирование: " + query.lastError().text());
            return;
        }

        cache[qMakePair(roomNumber * 100 + bedNumber, date)] = true;
    } else {
        QSqlQuery query(db);
        query.prepare("DELETE FROM bookings WHERE room_number = ? AND bed_number = ? AND booking_date = ?");
        query.addBindValue(roomNumber);
        query.addBindValue(bedNumber);
        query.addBindValue(date.toString("yyyy-MM-dd"));

        if (!query.exec()) {
            QMessageBox::warning(manager, "Ошибка",
                "Не удалось удалить бронирование: " + query.lastError().text());
            return;
        }

        cache.remove(qMakePair(roomNumber * 100 + bedNumber, date));
    }
}

bool HotelManagerBooking::isRoomOccupied(int roomNumber, const QDate &date, QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM bookings WHERE room_number = ? AND booking_date = ?");
    query.addBindValue(roomNumber);
    query.addBindValue(date.toString("yyyy-MM-dd"));

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

bool HotelManagerBooking::isBedOccupied(HotelManager *manager, int roomNumber, int bedNumber, const QDate &date)
{
    return manager->getOccupancyCache().contains(qMakePair(roomNumber * 100 + bedNumber, date));
}

// Реализация диалога выбора кровати
HotelManagerBooking::BookingDialog::BookingDialog(HotelManager *manager, int roomNumber,
                                                 const QDate &date, int totalBeds, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Выбор кровати");
    resize(300, 180);

    loadOccupiedBeds(manager, roomNumber, date);
    setupUI(totalBeds);
}

void HotelManagerBooking::BookingDialog::setupUI(int totalBeds)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    // Информация о занятых кроватях
    infoLabel = new QLabel(this);
    if (occupiedBeds.isEmpty()) {
        infoLabel->setText(QString("Все %1 кровати свободны").arg(totalBeds));
    } else {
        infoLabel->setText(QString("Заняты кровати: %1\nСвободно: %2 из %3")
                          .arg(occupiedBeds.join(", "))
                          .arg(totalBeds - occupiedBeds.size())
                          .arg(totalBeds));
    }
    layout->addWidget(infoLabel);

    // Выбор номера кровати
    QHBoxLayout *bedNumLayout = new QHBoxLayout();
    QLabel *bedNumLabel = new QLabel("Номер кровати:", this);
    bedSpinBox = new QSpinBox(this);
    bedSpinBox->setRange(1, totalBeds);
    bedSpinBox->setValue(1);

    // Пропускаем занятые кровати
    for (const QString &occupied : occupiedBeds) {
        int occupiedNum = occupied.left(occupied.length() -
            (occupied.endsWith("М") || occupied.endsWith("Ж") ? 1 : 0)).toInt();
        if (occupiedNum == 1) bedSpinBox->setValue(2);
    }

    bedNumLayout->addWidget(bedNumLabel);
    bedNumLayout->addWidget(bedSpinBox);
    layout->addLayout(bedNumLayout);

    // Выбор пола
    QHBoxLayout *genderLayout = new QHBoxLayout();
    QLabel *genderLabel = new QLabel("Пол:", this);
    genderCombo = new QComboBox(this);
    genderCombo->addItem("Не указывать", "");
    genderCombo->addItem("Мужской (М)", "М");
    genderCombo->addItem("Женский (Ж)", "Ж");
    genderLayout->addWidget(genderLabel);
    genderLayout->addWidget(genderCombo);
    layout->addLayout(genderLayout);

    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("OK", this);
    QPushButton *cancelButton = new QPushButton("Отмена", this);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    connect(okButton, &QPushButton::clicked, this, [this]() {
        selectedBedNumber = bedSpinBox->value();
        selectedGender = genderCombo->currentData().toString();
        accept();
    });

    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void HotelManagerBooking::BookingDialog::loadOccupiedBeds(HotelManager *manager,
                                                         int roomNumber, const QDate &date)
{
    QSqlQuery query(manager->getDatabase());
    query.prepare("SELECT bed_number, gender FROM bookings WHERE room_number = ? AND booking_date = ? ORDER BY bed_number");
    query.addBindValue(roomNumber);
    query.addBindValue(date.toString("yyyy-MM-dd"));

    if (query.exec()) {
        while (query.next()) {
            int occupiedBed = query.value(0).toInt();
            QString occupiedGender = query.value(1).toString();
            occupiedBeds.append(QString("%1%2").arg(occupiedBed)
                .arg(occupiedGender.isEmpty() ? "" : occupiedGender));
        }
    }
}

void HotelManagerBooking::showBedSelectionDialog(HotelManager *manager, int roomNumber,
                                                const QDate &date, int totalBeds,
                                                int &bedNumber, QString &gender)
{
    BookingDialog dialog(manager, roomNumber, date, totalBeds);

    if (dialog.exec() == QDialog::Accepted) {
        bedNumber = dialog.selectedBedNumber;
        gender = dialog.selectedGender;

        // Проверяем, не занята ли выбранная кровать
        if (isBedOccupied(manager, roomNumber, bedNumber, date)) {
            QMessageBox::warning(manager, "Ошибка",
                QString("Кровать %1 уже занята на эту дату!").arg(bedNumber));
            bedNumber = 0; // Отменяем бронирование
        }
    } else {
        bedNumber = 0; // Пользователь отменил
    }
}
