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
#include <QInputDialog>
#include <QMenuBar>
#include <QStatusBar>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QCursor>
#include <QKeySequence>
#include <QModelIndex>
#include <QModelIndexList>
#include <QItemSelectionModel>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QComboBox>

HotelManager::HotelManager(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::HotelManager)
{
    ui->setupUi(this);

    // Инициализируем меню
    initMenuBar();

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

    // Загружаем комнаты из БД
    loadRoomsFromDB();

    // Загружаем данные о бронированиях из базы данных
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

        menu.addSeparator();

        QAction *infoAction = menu.addAction("Информация о номере");
        QAction *editAction = menu.addAction("Изменить номер");
        QAction *deleteAction = menu.addAction("Удалить номер");

        QAction *selectedAction = menu.exec(ui->tableWidget->viewport()->mapToGlobal(pos));

        if (selectedAction == addAction) {
            addBooking();
        } else if (selectedAction == removeAction) {
            removeBooking();
        } else if (selectedAction == infoAction) {
            QModelIndexList selected = ui->tableWidget->selectionModel()->selectedIndexes();
            if (!selected.isEmpty()) {
                int row = selected.first().row();
                QString roomText = ui->tableWidget->item(row, 0)->text();
                int roomNumber = roomText.mid(9).toInt(); // "Комната 101" -> 101

                // Получаем информацию о номере из БД
                QSqlQuery query;
                query.prepare("SELECT room_type, capacity, price_per_night, description FROM rooms WHERE room_number = ?");
                query.addBindValue(roomNumber);

                if (query.exec() && query.next()) {
                    QString info = QString("Номер: %1\n"
                                          "Тип: %2\n"
                                          "Вместимость: %3 чел.\n"
                                          "Цена за ночь: %4 руб.\n"
                                          "Описание: %5")
                                  .arg(roomNumber)
                                  .arg(query.value(0).toString())
                                  .arg(query.value(1).toInt())
                                  .arg(query.value(2).toDouble())
                                  .arg(query.value(3).toString());

                    QMessageBox::information(this, "Информация о номере", info);
                }
            }
        } else if (selectedAction == editAction) {
            QModelIndexList selected = ui->tableWidget->selectionModel()->selectedIndexes();
            if (!selected.isEmpty()) {
                int row = selected.first().row();
                QString roomText = ui->tableWidget->item(row, 0)->text();
                int roomNumber = roomText.mid(9).toInt();

                // Здесь можно добавить функционал редактирования номера
                QMessageBox::information(this, "Редактирование",
                    QString("Редактирование комнаты %1").arg(roomNumber));
            }
        } else if (selectedAction == deleteAction) {
            deleteRoom();
        }
    });

    // Подключаем кнопки
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

void HotelManager::initMenuBar()
{
    // Создаем менюбар
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    // Меню "Файл"
    QMenu *fileMenu = menuBar->addMenu("&Файл");

    QAction *addRoomAction = new QAction("&Добавить комнату", this);
    addRoomAction->setShortcut(QKeySequence("Ctrl+N"));
    connect(addRoomAction, &QAction::triggered, this, &HotelManager::addRoom);
    fileMenu->addAction(addRoomAction);

    QAction *deleteRoomAction = new QAction("&Удалить комнату", this);
    deleteRoomAction->setShortcut(QKeySequence("Ctrl+D"));
    connect(deleteRoomAction, &QAction::triggered, this, &HotelManager::deleteRoom);
    fileMenu->addAction(deleteRoomAction);

    fileMenu->addSeparator();

    QAction *exitAction = new QAction("&Выход", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(exitAction);

    // Меню "Клиенты"
    QMenu *clientsMenu = menuBar->addMenu("&Клиенты");

    QAction *addClientAction = new QAction("&Добавить клиента", this);
    addClientAction->setShortcut(QKeySequence("Ctrl+C"));
    connect(addClientAction, &QAction::triggered, this, &HotelManager::manageClients);
    clientsMenu->addAction(addClientAction);

    QAction *viewClientsAction = new QAction("&Список клиентов", this);
    viewClientsAction->setShortcut(QKeySequence("Ctrl+L"));
    connect(viewClientsAction, &QAction::triggered, this, &HotelManager::manageClients);
    clientsMenu->addAction(viewClientsAction);

    // Меню "Бронирование"
    QMenu *bookingMenu = menuBar->addMenu("&Бронирование");

    QAction *newBookingAction = new QAction("&Новое бронирование", this);
    newBookingAction->setShortcut(QKeySequence("Ctrl+B"));
    connect(newBookingAction, &QAction::triggered, this, &HotelManager::addBooking);
    bookingMenu->addAction(newBookingAction);

    QAction *viewBookingsAction = new QAction("&Все бронирования", this);
    viewBookingsAction->setShortcut(QKeySequence("Ctrl+Shift+B"));
    connect(viewBookingsAction, &QAction::triggered, this, []() {
        QMessageBox::information(nullptr, "Бронирования", "Список всех бронирований будет здесь");
    });
    bookingMenu->addAction(viewBookingsAction);

    // Меню "Доп услуги"
    QMenu *servicesMenu = menuBar->addMenu("&Доп услуги");

    QAction *manageServicesAction = new QAction("&Управление услугами", this);
    connect(manageServicesAction, &QAction::triggered, this, &HotelManager::manageServices);
    servicesMenu->addAction(manageServicesAction);

    QAction *assignServiceAction = new QAction("&Назначить услугу", this);
    connect(assignServiceAction, &QAction::triggered, this, []() {
        QMessageBox::information(nullptr, "Услуги", "Назначение дополнительных услуг");
    });
    servicesMenu->addAction(assignServiceAction);

    // Меню "Обзор"
    QMenu *reportsMenu = menuBar->addMenu("&Обзор");

    QAction *viewReportsAction = new QAction("&Отчеты", this);
    connect(viewReportsAction, &QAction::triggered, this, &HotelManager::viewReports);
    reportsMenu->addAction(viewReportsAction);

    QAction *statisticsAction = new QAction("&Статистика", this);
    connect(statisticsAction, &QAction::triggered, this, []() {
        QMessageBox::information(nullptr, "Статистика", "Статистика загруженности отеля");
    });
    reportsMenu->addAction(statisticsAction);

    QAction *calendarAction = new QAction("&Календарь", this);
    calendarAction->setShortcut(QKeySequence("Ctrl+K"));
    connect(calendarAction, &QAction::triggered, this, []() {
        QMessageBox::information(nullptr, "Календарь", "Просмотр в календарном виде");
    });
    reportsMenu->addAction(calendarAction);
}

bool HotelManager::isValidRoomName(const QString &name)
{
    // Регулярное выражение: кириллица, латиница, цифры, пробелы и разрешенные символы
    // Добавлены точка и запятая для более гибкого описания
    QRegularExpression regex("^[а-яА-ЯёЁa-zA-Z0-9\\s\\(\\)\\/\\-\\.\\,]+$");
    QRegularExpressionMatch match = regex.match(name);
    return match.hasMatch() && !name.trimmed().isEmpty();
}

QString HotelManager::getRoomNameFromUser(const QString &title, const QString &label, const QString &defaultValue)
{
    QDialog dialog(this);
    dialog.setWindowTitle(title);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLabel *labelWidget = new QLabel(label, &dialog);
    layout->addWidget(labelWidget);

    QLineEdit *lineEdit = new QLineEdit(&dialog);
    lineEdit->setText(defaultValue);

    // Устанавливаем валидатор
    QRegularExpressionValidator *validator = new QRegularExpressionValidator(
        QRegularExpression("^[а-яА-ЯёЁa-zA-Z0-9\\s\\(\\)\\/\\-]+$"), &dialog);
    lineEdit->setValidator(validator);

    layout->addWidget(lineEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("OK", &dialog);
    QPushButton *cancelButton = new QPushButton("Отмена", &dialog);

    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    QString result;

    connect(okButton, &QPushButton::clicked, &dialog, [&dialog, lineEdit, &result]() {
        if (!lineEdit->text().trimmed().isEmpty()) {
            result = lineEdit->text().trimmed();
            dialog.accept();
        } else {
            QMessageBox::warning(&dialog, "Ошибка", "Название комнаты не может быть пустым!");
        }
    });

    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        return result;
    }

    return QString();
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

    // Создаем таблицу бронирований, если она не существует
    QSqlQuery query;
    QString createBookingsTable =
        "CREATE TABLE IF NOT EXISTS bookings ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "room_number INTEGER NOT NULL, "
        "booking_date DATE NOT NULL, "
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
        "UNIQUE(room_number, booking_date)"
        ")";

    if (!query.exec(createBookingsTable)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось создать таблицу бронирований: " + query.lastError().text());
    }

    // Создаем таблицу комнат, если она не существует
    QString createRoomsTable =
        "CREATE TABLE IF NOT EXISTS rooms ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "room_number INTEGER UNIQUE NOT NULL, "
        "room_type TEXT DEFAULT 'Стандарт', "
        "capacity INTEGER DEFAULT 2, "
        "price_per_night REAL DEFAULT 3000.0, "
        "description TEXT, "
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ")";

    if (!query.exec(createRoomsTable)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось создать таблицу комнат: " + query.lastError().text());
    }

    // Создаем таблицу клиентов, если она не существует
    QString createClientsTable =
        "CREATE TABLE IF NOT EXISTS clients ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "full_name TEXT NOT NULL, "
        "phone TEXT, "
        "email TEXT, "
        "passport TEXT, "
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ")";

    if (!query.exec(createClientsTable)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось создать таблицу клиентов: " + query.lastError().text());
    }

    // Создаем таблицу услуг, если она не существует
    QString createServicesTable =
        "CREATE TABLE IF NOT EXISTS services ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "service_name TEXT NOT NULL, "
        "price REAL DEFAULT 0.0, "
        "description TEXT"
        ")";

    if (!query.exec(createServicesTable)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось создать таблицу услуг: " + query.lastError().text());
    }

    statusBar()->showMessage("База данных подключена", 3000);
}

void HotelManager::loadRoomsFromDB()
{
    QSqlQuery query("SELECT room_number, room_type FROM rooms ORDER BY room_number");

    int row = 0;
    ui->tableWidget->setRowCount(0);

    while (query.next()) {
        int roomNumber = query.value(0).toInt();
        QString roomType = query.value(1).toString();

        ui->tableWidget->insertRow(row);
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(
            QString("Комната %1 (%2)").arg(roomNumber).arg(roomType)));
        row++;
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
    // Извлекаем номер из строки типа "Комната 101 (Стандарт)"
    int roomNumber = 0;
    QRegularExpression re("Комната\\s+(\\d+)");
    QRegularExpressionMatch match = re.match(roomText);
    if (match.hasMatch()) {
        roomNumber = match.captured(1).toInt();
    }

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

void HotelManager::addRoom()
{
    bool ok;

    // Получаем номер комнаты
    int roomNumber = QInputDialog::getInt(this, "Добавить комнату",
                                         "Введите номер комнаты:",
                                         101, 1, 999, 1, &ok);

    if (!ok) return;

    // Проверяем, существует ли уже комната с таким номером
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT COUNT(*) FROM rooms WHERE room_number = ?");
    checkQuery.addBindValue(roomNumber);

    if (checkQuery.exec() && checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        QMessageBox::warning(this, "Ошибка",
            QString("Комната с номером %1 уже существует!").arg(roomNumber));
        return;
    }

    // Получаем тип комнаты
    QStringList roomTypes = QStringList() << "Стандарт" << "Люкс" << "Семейный" << "Бизнес" << "Апартаменты";
    QString roomType = QInputDialog::getItem(this, "Тип комнаты",
                                            "Выберите тип комнаты:",
                                            roomTypes, 0, false, &ok);
    if (!ok) return;

    // Получаем вместимость
    int capacity = QInputDialog::getInt(this, "Вместимость",
                                       "Введите вместимость (человек):",
                                       2, 1, 10, 1, &ok);
    if (!ok) return;

    // Получаем цену
    double price = QInputDialog::getDouble(this, "Цена",
                                          "Введите цену за ночь:",
                                          3000.0, 500.0, 50000.0, 2, &ok);
    if (!ok) return;

    // Создаем кастомный диалог для ввода описания
    QDialog dialog(this);
    dialog.setWindowTitle("Описание комнаты");

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLabel *label = new QLabel("Введите описание комнаты (можно использовать кириллицу, латиницу, цифры, (), /, -, пробелы):", &dialog);
    layout->addWidget(label);

    QLineEdit *lineEdit = new QLineEdit(&dialog);

    // Устанавливаем валидатор с правильным регулярным выражением
    QRegularExpressionValidator *validator = new QRegularExpressionValidator(
        QRegularExpression("^[а-яА-ЯёЁa-zA-Z0-9\\s\\(\\)\\/\\-\\.\\,]+$"), &dialog);
    lineEdit->setValidator(validator);

    // Разрешаем длинный текст
    lineEdit->setMaxLength(500);

    layout->addWidget(lineEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("OK", &dialog);
    QPushButton *cancelButton = new QPushButton("Отмена", &dialog);

    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    QString description;

    connect(okButton, &QPushButton::clicked, &dialog, [&dialog, lineEdit, &description]() {
        QString text = lineEdit->text().trimmed();
        if (!text.isEmpty()) {
            description = text;
            dialog.accept();
        } else {
            QMessageBox::warning(&dialog, "Ошибка", "Описание комнаты не может быть пустым!");
        }
    });

    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return; // Пользователь отменил
    }

    // Добавляем комнату в базу данных
    QSqlQuery query;
    query.prepare("INSERT INTO rooms (room_number, room_type, capacity, price_per_night, description) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(roomNumber);
    query.addBindValue(roomType);
    query.addBindValue(capacity);
    query.addBindValue(price);
    query.addBindValue(description);

    if (query.exec()) {
        // Обновляем таблицу
        loadRoomsFromDB();
        updateTableHeaders();

        QMessageBox::information(this, "Успех",
            QString("Комната %1 (%2) успешно добавлена!\nОписание: %3")
                .arg(roomNumber)
                .arg(roomType)
                .arg(description));
        statusBar()->showMessage(QString("Добавлена комната %1").arg(roomNumber), 3000);
    } else {
        QMessageBox::critical(this, "Ошибка",
            "Не удалось добавить комнату: " + query.lastError().text());
    }
}
void HotelManager::deleteRoom()
{
    // Получаем список всех комнат для выбора
    QSqlQuery query("SELECT room_number, room_type FROM rooms ORDER BY room_number");

    QStringList rooms;
    QMap<QString, int> roomMap; // Для сопоставления строки с номером комнаты

    while (query.next()) {
        int roomNumber = query.value(0).toInt();
        QString roomType = query.value(1).toString();
        QString roomString = QString("%1 (%2)").arg(roomNumber).arg(roomType);
        rooms.append(roomString);
        roomMap[roomString] = roomNumber;
    }

    if (rooms.isEmpty()) {
        QMessageBox::information(this, "Удаление комнаты", "Нет комнат для удаления");
        return;
    }

    bool ok;
    QString selectedRoom = QInputDialog::getItem(this, "Удалить комнату",
                                                "Выберите комнату для удаления:",
                                                rooms, 0, false, &ok);

    if (!ok || selectedRoom.isEmpty()) return;

    int roomNumber = roomMap[selectedRoom];

    // Проверяем, есть ли активные бронирования у этой комнаты
    QSqlQuery checkBookingsQuery;
    checkBookingsQuery.prepare("SELECT COUNT(*) FROM bookings WHERE room_number = ? AND booking_date >= ?");
    checkBookingsQuery.addBindValue(roomNumber);
    checkBookingsQuery.addBindValue(QDate::currentDate().toString("yyyy-MM-dd"));

    if (checkBookingsQuery.exec() && checkBookingsQuery.next()) {
        int activeBookings = checkBookingsQuery.value(0).toInt();

        if (activeBookings > 0) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this, "Подтверждение удаления",
                QString("У комнаты %1 есть активные бронирования (%2 шт.).\n"
                       "При удалении комнаты все бронирования будут также удалены.\n"
                       "Продолжить?").arg(roomNumber).arg(activeBookings),
                QMessageBox::Yes | QMessageBox::No
            );

            if (reply != QMessageBox::Yes) {
                return;
            }
        }
    }

    // Удаляем все бронирования этой комнаты
    QSqlQuery deleteBookingsQuery;
    deleteBookingsQuery.prepare("DELETE FROM bookings WHERE room_number = ?");
    deleteBookingsQuery.addBindValue(roomNumber);

    if (!deleteBookingsQuery.exec()) {
        QMessageBox::warning(this, "Ошибка",
            "Не удалось удалить бронирования комнаты: " + deleteBookingsQuery.lastError().text());
        return;
    }

    // Удаляем комнату
    QSqlQuery deleteRoomQuery;
    deleteRoomQuery.prepare("DELETE FROM rooms WHERE room_number = ?");
    deleteRoomQuery.addBindValue(roomNumber);

    if (deleteRoomQuery.exec()) {
        // Очищаем кэш для этой комнаты
        QList<QPair<int, QDate>> keysToRemove;
        for (auto it = occupancyCache.begin(); it != occupancyCache.end(); ++it) {
            if (it.key().first == roomNumber) {
                keysToRemove.append(it.key());
            }
        }

        for (const auto &key : keysToRemove) {
            occupancyCache.remove(key);
        }

        // Обновляем таблицу
        loadRoomsFromDB();
        updateTableHeaders();

        QMessageBox::information(this, "Успех",
            QString("Комната %1 успешно удалена!").arg(roomNumber));
        statusBar()->showMessage(QString("Удалена комната %1").arg(roomNumber), 3000);
    } else {
        QMessageBox::critical(this, "Ошибка",
            "Не удалось удалить комнату: " + deleteRoomQuery.lastError().text());
    }
}

// ... остальные методы (manageClients, manageServices, viewReports, addBooking, removeBooking, updateTableHeaders)
// остаются такими же, как в предыдущем ответе ...

void HotelManager::manageClients()
{
    // Создаем диалог для управления клиентами
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Управление клиентами");
    dialog->resize(600, 400);

    QVBoxLayout *layout = new QVBoxLayout(dialog);

    // Таблица клиентов
    QTableWidget *clientsTable = new QTableWidget(dialog);
    clientsTable->setColumnCount(5);
    clientsTable->setHorizontalHeaderLabels(QStringList() << "ID" << "ФИО" << "Телефон" << "Email" << "Паспорт");

    // Загружаем клиентов из БД
    QSqlQuery query("SELECT id, full_name, phone, email, passport FROM clients ORDER BY full_name");
    int row = 0;
    while (query.next()) {
        clientsTable->insertRow(row);
        for (int col = 0; col < 5; col++) {
            clientsTable->setItem(row, col, new QTableWidgetItem(query.value(col).toString()));
        }
        row++;
    }

    layout->addWidget(clientsTable);

    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    QPushButton *addButton = new QPushButton("Добавить клиента", dialog);
    connect(addButton, &QPushButton::clicked, dialog, [dialog, clientsTable]() {
        bool ok;
        QString name = QInputDialog::getText(dialog, "Добавить клиента",
                                           "Введите ФИО:", QLineEdit::Normal, "", &ok);
        if (!ok || name.isEmpty()) return;

        QString phone = QInputDialog::getText(dialog, "Телефон",
                                            "Введите телефон:", QLineEdit::Normal, "", &ok);
        if (!ok) return;

        QString email = QInputDialog::getText(dialog, "Email",
                                            "Введите email:", QLineEdit::Normal, "", &ok);
        if (!ok) return;

        QString passport = QInputDialog::getText(dialog, "Паспорт",
                                               "Введите паспортные данные:", QLineEdit::Normal, "", &ok);
        if (!ok) return;

        QSqlQuery query;
        query.prepare("INSERT INTO clients (full_name, phone, email, passport) "
                     "VALUES (?, ?, ?, ?)");
        query.addBindValue(name);
        query.addBindValue(phone);
        query.addBindValue(email);
        query.addBindValue(passport);

        if (query.exec()) {
            // Обновляем таблицу
            int row = clientsTable->rowCount();
            clientsTable->insertRow(row);
            clientsTable->setItem(row, 0, new QTableWidgetItem(query.lastInsertId().toString()));
            clientsTable->setItem(row, 1, new QTableWidgetItem(name));
            clientsTable->setItem(row, 2, new QTableWidgetItem(phone));
            clientsTable->setItem(row, 3, new QTableWidgetItem(email));
            clientsTable->setItem(row, 4, new QTableWidgetItem(passport));
        }
    });

    QPushButton *closeButton = new QPushButton("Закрыть", dialog);
    connect(closeButton, &QPushButton::clicked, dialog, &QDialog::close);

    buttonLayout->addWidget(addButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);

    layout->addLayout(buttonLayout);
    dialog->setLayout(layout);
    dialog->exec();
}

void HotelManager::manageServices()
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Управление услугами");
    dialog->resize(500, 300);

    QVBoxLayout *layout = new QVBoxLayout(dialog);

    // Таблица услуг
    QTableWidget *servicesTable = new QTableWidget(dialog);
    servicesTable->setColumnCount(3);
    servicesTable->setHorizontalHeaderLabels(QStringList() << "Услуга" << "Цена" << "Описание");

    QSqlQuery query("SELECT service_name, price, description FROM services");
    int row = 0;
    while (query.next()) {
        servicesTable->insertRow(row);
        for (int col = 0; col < 3; col++) {
            servicesTable->setItem(row, col, new QTableWidgetItem(query.value(col).toString()));
        }
        row++;
    }

    layout->addWidget(servicesTable);

    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    QPushButton *addButton = new QPushButton("Добавить услугу", dialog);
    connect(addButton, &QPushButton::clicked, dialog, [dialog, servicesTable]() {
        bool ok;
        QString name = QInputDialog::getText(dialog, "Добавить услугу",
                                           "Название услуги:", QLineEdit::Normal, "", &ok);
        if (!ok || name.isEmpty()) return;

        double price = QInputDialog::getDouble(dialog, "Цена",
                                              "Цена услуги:", 0, 0, 100000, 2, &ok);
        if (!ok) return;

        QString description = QInputDialog::getText(dialog, "Описание",
                                                  "Описание услуги:", QLineEdit::Normal, "", &ok);
        if (!ok) return;

        QSqlQuery query;
        query.prepare("INSERT INTO services (service_name, price, description) "
                     "VALUES (?, ?, ?)");
        query.addBindValue(name);
        query.addBindValue(price);
        query.addBindValue(description);

        if (query.exec()) {
            int row = servicesTable->rowCount();
            servicesTable->insertRow(row);
            servicesTable->setItem(row, 0, new QTableWidgetItem(name));
            servicesTable->setItem(row, 1, new QTableWidgetItem(QString::number(price)));
            servicesTable->setItem(row, 2, new QTableWidgetItem(description));
        }
    });

    QPushButton *closeButton = new QPushButton("Закрыть", dialog);
    connect(closeButton, &QPushButton::clicked, dialog, &QDialog::close);

    buttonLayout->addWidget(addButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);

    layout->addLayout(buttonLayout);
    dialog->setLayout(layout);
    dialog->exec();
}

void HotelManager::viewReports()
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Отчеты и обзор");
    dialog->resize(400, 300);

    QVBoxLayout *layout = new QVBoxLayout(dialog);

    QTextEdit *reportText = new QTextEdit(dialog);
    reportText->setReadOnly(true);

    // Генерируем отчет
    QString report;
    report += "<h2>Отчет по отелю</h2>";
    report += "<h3>Статистика на " + QDate::currentDate().toString("dd.MM.yyyy") + "</h3>";

    // Общее количество комнат
    QSqlQuery roomQuery("SELECT COUNT(*) FROM rooms");
    if (roomQuery.next()) {
        report += "<p><b>Всего комнат:</b> " + roomQuery.value(0).toString() + "</p>";
    }

    // Занятость на сегодня
    QSqlQuery occupancyQuery;
    occupancyQuery.prepare("SELECT COUNT(DISTINCT room_number) FROM bookings WHERE booking_date = ?");
    occupancyQuery.addBindValue(QDate::currentDate().toString("yyyy-MM-dd"));

    if (occupancyQuery.exec() && occupancyQuery.next()) {
        int occupiedToday = occupancyQuery.value(0).toInt();

        // Повторно выполняем запрос для получения общего количества комнат
        roomQuery.exec("SELECT COUNT(*) FROM rooms");
        int totalRooms = 0;
        if (roomQuery.next()) {
            totalRooms = roomQuery.value(0).toInt();
        }

        double occupancyRate = totalRooms > 0 ? (occupiedToday * 100.0 / totalRooms) : 0;

        report += "<p><b>Занято сегодня:</b> " + QString::number(occupiedToday) + " из " +
                 QString::number(totalRooms) + " (" + QString::number(occupancyRate, 'f', 1) + "%)</p>";
    }

    // Предстоящие бронирования
    report += "<h3>Предстоящие бронирования (7 дней)</h3>";

    QSqlQuery upcomingQuery;
    upcomingQuery.prepare("SELECT room_number, booking_date FROM bookings "
                         "WHERE booking_date BETWEEN ? AND ? "
                         "ORDER BY booking_date, room_number");
    QDate today = QDate::currentDate();
    upcomingQuery.addBindValue(today.toString("yyyy-MM-dd"));
    upcomingQuery.addBindValue(today.addDays(7).toString("yyyy-MM-dd"));

    if (upcomingQuery.exec()) {
        QMap<QDate, QStringList> bookingsByDate;
        while (upcomingQuery.next()) {
            QDate date = upcomingQuery.value(1).toDate();
            int room = upcomingQuery.value(0).toInt();
            bookingsByDate[date].append(QString::number(room));
        }

        if (bookingsByDate.isEmpty()) {
            report += "<p>Нет предстоящих бронирований на ближайшие 7 дней</p>";
        } else {
            for (auto it = bookingsByDate.begin(); it != bookingsByDate.end(); ++it) {
                report += "<p><b>" + it.key().toString("dd.MM") + ":</b> " +
                         it.value().join(", ") + "</p>";
            }
        }
    }

    reportText->setHtml(report);
    layout->addWidget(reportText);

    QPushButton *closeButton = new QPushButton("Закрыть", dialog);
    connect(closeButton, &QPushButton::clicked, dialog, &QDialog::close);

    layout->addWidget(closeButton);
    dialog->setLayout(layout);
    dialog->exec();
}

void HotelManager::addBooking()
{
    QModelIndexList selected = ui->tableWidget->selectionModel()->selectedIndexes();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "Бронирование",
            "Выберите ячейку в таблице для бронирования");
        return;
    }

    QModelIndex index = selected.first();
    int row = index.row();
    int col = index.column();

    if (col == 0) return; // Нельзя забронировать столбец с номерами

    // Получаем номер комнаты
    QString roomText = ui->tableWidget->item(row, 0)->text();
    int roomNumber = 0;
    QRegularExpression re("Комната\\s+(\\d+)");
    QRegularExpressionMatch match = re.match(roomText);
    if (match.hasMatch()) {
        roomNumber = match.captured(1).toInt();
    }

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
    if (selected.isEmpty()) {
        QMessageBox::information(this, "Снять бронь",
            "Выберите занятую ячейку для снятия брони");
        return;
    }

    QModelIndex index = selected.first();
    int row = index.row();
    int col = index.column();

    if (col == 0) return;

    // Получаем номер комнаты
    QString roomText = ui->tableWidget->item(row, 0)->text();
    int roomNumber = 0;
    QRegularExpression re("Комната\\s+(\\d+)");
    QRegularExpressionMatch match = re.match(roomText);
    if (match.hasMatch()) {
        roomNumber = match.captured(1).toInt();
    }

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
        int roomNumber = 0;
        QRegularExpression re("Комната\\s+(\\d+)");
        QRegularExpressionMatch match = re.match(roomText);
        if (match.hasMatch()) {
            roomNumber = match.captured(1).toInt();
        }

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
