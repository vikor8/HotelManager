#include "hotelmanager.h"
#include "ui_hotelmanager.h"
#include <QDateEdit>
#include <QHeaderView>
#include <QBrush>
#include <QColor>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QDebug>

HotelManager::HotelManager(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::HotelManager)
{
    ui->setupUi(this);

    // Устанавливаем текущую дату по умолчанию
    startDate = QDate::currentDate();

    // Настройка виджета даты
    ui->dateEdit->setDate(startDate);
    ui->dateEdit->setCalendarPopup(true);

    // Настройка таблицы
    ui->tableWidget->setColumnCount(31); // 1 столбец для номеров + 30 дней
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "Комнаты");

    // Инициализация базы данных
    initDatabase();

    // Заполняем номерами комнат (например, 20 номеров с 101 по 120)
    int roomCount = 20;
    ui->tableWidget->setRowCount(roomCount);
    for (int i = 0; i < roomCount; i++) {
        int roomNumber = 101 + i;
        ui->tableWidget->setItem(i, 0, new QTableWidgetItem(QString("Комната %1").arg(roomNumber)));
    }

    // Загружаем данные из базы данных
    loadOccupancyFromDB();

    // Обновляем заголовки и отображение
    updateTableHeaders();

    // Подключаем сигналы
    connect(ui->dateEdit, &QDateEdit::dateChanged, this, &HotelManager::onDateChanged);
    connect(ui->tableWidget, &QTableWidget::clicked, this, &HotelManager::onTableClicked);

    // Настраиваем контекстное меню для таблицы
    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableWidget, &QTableWidget::customContextMenuRequested,
            this, [this](const QPoint &pos) {
        QMenu menu(this);
        QAction *addAction = menu.addAction("Забронировать номер");
        QAction *removeAction = menu.addAction("Снять бронь");

        QAction *selectedAction = menu.exec(ui->tableWidget->viewport()->mapToGlobal(pos));

        if (selectedAction == addAction) {
            addBooking();
        } else if (selectedAction == removeAction) {
            removeBooking();
        }
    });
    connect(ui->btnToday, &QPushButton::clicked, this, [this]() {
        ui->dateEdit->setDate(QDate::currentDate());
    });

    connect(ui->btnRefresh, &QPushButton::clicked, this, [this]() {
        loadOccupancyFromDB();
        updateTableHeaders();
    });
}

HotelManager::~HotelManager()
{
    // Закрываем базу данных
    if (db.isOpen()) {
        db.close();
    }
    delete ui;
}

void HotelManager::initDatabase()
{
    // Инициализация базы данных SQLite
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("hotel_bookings.db");

    if (!db.open()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть базу данных: " + db.lastError().text());
        return;
    }

    // Создаем таблицу, если она не существует
    QSqlQuery query;
    QString createTableQuery =
        "CREATE TABLE IF NOT EXISTS bookings ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "room_number INTEGER NOT NULL, "
        "booking_date DATE NOT NULL, "
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
        "UNIQUE(room_number, booking_date)"
        ")";

    if (!query.exec(createTableQuery)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось создать таблицу: " + query.lastError().text());
    }
}

void HotelManager::loadOccupancyFromDB()
{
    occupancyCache.clear();

    QSqlQuery query;
    query.prepare("SELECT room_number, booking_date FROM bookings");

    if (query.exec()) {
        while (query.next()) {
            int roomNumber = query.value(0).toInt();
            QDate date = query.value(1).toDate();

            // Сохраняем в кэш
            occupancyCache[qMakePair(roomNumber, date)] = true;
        }
    } else {
        qDebug() << "Ошибка загрузки данных: " << query.lastError().text();
    }
}

void HotelManager::saveOccupancyToDB(int roomNumber, const QDate &date, bool occupied)
{
    if (occupied) {
        // Добавляем бронирование
        QSqlQuery query;
        query.prepare("INSERT OR REPLACE INTO bookings (room_number, booking_date) VALUES (?, ?)");
        query.addBindValue(roomNumber);
        query.addBindValue(date.toString("yyyy-MM-dd"));

        if (!query.exec()) {
            QMessageBox::warning(this, "Ошибка",
                "Не удалось сохранить бронирование: " + query.lastError().text());
            return;
        }

        // Обновляем кэш
        occupancyCache[qMakePair(roomNumber, date)] = true;
    } else {
        // Удаляем бронирование
        QSqlQuery query;
        query.prepare("DELETE FROM bookings WHERE room_number = ? AND booking_date = ?");
        query.addBindValue(roomNumber);
        query.addBindValue(date.toString("yyyy-MM-dd"));

        if (!query.exec()) {
            QMessageBox::warning(this, "Ошибка",
                "Не удалось удалить бронирование: " + query.lastError().text());
            return;
        }

        // Удаляем из кэша
        occupancyCache.remove(qMakePair(roomNumber, date));
    }
}

bool HotelManager::isRoomOccupied(int roomNumber, const QDate &date)
{
    return occupancyCache.contains(qMakePair(roomNumber, date));
}

void HotelManager::onDateChanged()
{
    startDate = ui->dateEdit->date();
    updateTableHeaders();
}

void HotelManager::onTableClicked(const QModelIndex &index)
{
    // Если кликнули не на ячейку с датой, игнорируем
    if (index.column() == 0) return;

    int row = index.row();
    int col = index.column();

    // Получаем номер комнаты
    QString roomText = ui->tableWidget->item(row, 0)->text();
    int roomNumber = roomText.mid(9).toInt(); // "Комната 101" -> 101

    // Получаем дату для этой колонки
    QDate cellDate = startDate.addDays(col - 1);

    // Создаем меню действий
    QMenu menu(this);

    if (isRoomOccupied(roomNumber, cellDate)) {
        menu.addAction("Снять бронь", this, &HotelManager::removeBooking);
    } else {
        menu.addAction("Забронировать", this, &HotelManager::addBooking);
    }

    menu.exec(QCursor::pos());
}

void HotelManager::addBooking()
{
    QModelIndexList selected = ui->tableWidget->selectionModel()->selectedIndexes();
    if (selected.isEmpty()) return;

    QModelIndex index = selected.first();
    int row = index.row();
    int col = index.column();

    if (col == 0) return; // Нельзя забронировать столбец с номерами

    // Получаем номер комнаты
    QString roomText = ui->tableWidget->item(row, 0)->text();
    int roomNumber = roomText.mid(9).toInt();

    // Получаем дату
    QDate cellDate = startDate.addDays(col - 1);

    // Сохраняем в БД
    saveOccupancyToDB(roomNumber, cellDate, true);

    // Обновляем отображение
    updateTableHeaders();

    QMessageBox::information(this, "Успех",
        QString("Комната %1 забронирована на %2").arg(roomNumber).arg(cellDate.toString("dd.MM.yyyy")));
}

void HotelManager::removeBooking()
{
    QModelIndexList selected = ui->tableWidget->selectionModel()->selectedIndexes();
    if (selected.isEmpty()) return;

    QModelIndex index = selected.first();
    int row = index.row();
    int col = index.column();

    if (col == 0) return;

    // Получаем номер комнаты
    QString roomText = ui->tableWidget->item(row, 0)->text();
    int roomNumber = roomText.mid(9).toInt();

    // Получаем дату
    QDate cellDate = startDate.addDays(col - 1);

    // Удаляем из БД
    saveOccupancyToDB(roomNumber, cellDate, false);

    // Обновляем отображение
    updateTableHeaders();

    QMessageBox::information(this, "Успех",
        QString("Бронь комнаты %1 на %2 снята").arg(roomNumber).arg(cellDate.toString("dd.MM.yyyy")));
}

void HotelManager::updateTableHeaders()
{
    // Очищаем все ячейки кроме первой колонки
    for (int row = 0; row < ui->tableWidget->rowCount(); row++) {
        for (int col = 1; col < ui->tableWidget->columnCount(); col++) {
            if (ui->tableWidget->item(row, col)) {
                delete ui->tableWidget->item(row, col);
            }
        }
    }

    // Устанавливаем заголовки для колонок с датами
    QStringList headers;
    headers << "Комнаты";

    for (int i = 0; i < 30; i++) {
        QDate currentDate = startDate.addDays(i);
        headers << currentDate.toString("dd.MM\nyyyy");
    }

    ui->tableWidget->setHorizontalHeaderLabels(headers);

    // Заполняем таблицу данными о занятости
    for (int row = 0; row < ui->tableWidget->rowCount(); row++) {
        // Получаем номер комнаты из первой колонки
        QString roomText = ui->tableWidget->item(row, 0)->text();
        int roomNumber = roomText.mid(9).toInt(); // "Комната 101" -> 101

        for (int col = 1; col < ui->tableWidget->columnCount(); col++) {
            QDate cellDate = startDate.addDays(col - 1);

            QTableWidgetItem *item = new QTableWidgetItem();

            // Проверяем занятость через кэш/БД
            if (isRoomOccupied(roomNumber, cellDate)) {
                item->setBackground(QBrush(QColor(144, 238, 144))); // светло-зеленый
                item->setText("Занят");
                item->setToolTip(QString("Комната %1 занята на %2")
                    .arg(roomNumber)
                    .arg(cellDate.toString("dd.MM.yyyy")));
            } else {
                item->setBackground(QBrush(QColor(255, 255, 255))); // белый
                item->setText("Свободен");
                item->setToolTip(QString("Комната %1 свободна на %2")
                    .arg(roomNumber)
                    .arg(cellDate.toString("dd.MM.yyyy")));
            }

            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable); // Не редактируемая
            ui->tableWidget->setItem(row, col, item);
        }
    }

    // Настройка ширины столбцов
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    for (int i = 1; i < ui->tableWidget->columnCount(); i++) {
        ui->tableWidget->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Fixed);
        ui->tableWidget->horizontalHeader()->resizeSection(i, 80);
    }

    // Делаем первую колонку (номера) нередактируемой
    for (int row = 0; row < ui->tableWidget->rowCount(); row++) {
        if (ui->tableWidget->item(row, 0)) {
            ui->tableWidget->item(row, 0)->setFlags(ui->tableWidget->item(row, 0)->flags() & ~Qt::ItemIsEditable);
        }
    }

    // Устанавливаем высоту строк
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(30);
}
