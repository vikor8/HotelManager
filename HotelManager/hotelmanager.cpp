#include "hotelmanager.h"
#include "ui_hotelmanager.h"
#include <QDateEdit>
#include <QHeaderView>
#include <QBrush>
#include <QColor>

HotelManager::HotelManager(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::HotelManager)
{
    ui->setupUi(this);

    // Инициализация
    startDate = QDate::currentDate();

    // Настройка виджета даты
    ui->dateEdit->setDate(startDate);
    ui->dateEdit->setCalendarPopup(true);

    // Настройка таблицы
    ui->tableWidget->setColumnCount(31); // 1 столбец для номеров + 30 дней
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "Комнаты");

    // Заполняем номерами комнат
    int roomCount = 20; // 20 номеров для примера
    ui->tableWidget->setRowCount(roomCount);
    for (int i = 0; i < roomCount; i++) {
        ui->tableWidget->setItem(i, 0, new QTableWidgetItem(QString("Комната %1").arg(i + 101)));
    }

    // Заполняем тестовыми данными
    fillSampleData();

    // Обновляем заголовки и отображение
    updateTableHeaders();

    // Подключаем сигнал изменения даты
    connect(ui->dateEdit, &QDateEdit::dateChanged, this, &HotelManager::onDateChanged);
}

HotelManager::~HotelManager()
{
    delete ui;
}

void HotelManager::onDateChanged()
{
    startDate = ui->dateEdit->date();
    updateTableHeaders();
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
        headers << currentDate.toString("dd.MM");
    }

    ui->tableWidget->setHorizontalHeaderLabels(headers);

    // Заполняем таблицу данными о занятости
    for (int row = 0; row < ui->tableWidget->rowCount(); row++) {
        int roomNumber = row + 101; // 101, 102, 103...

        for (int col = 1; col < ui->tableWidget->columnCount(); col++) {
            QDate cellDate = startDate.addDays(col - 1);

            QTableWidgetItem *item = new QTableWidgetItem();

            // Проверяем, занят ли номер на эту дату
            if (roomOccupancy.contains(roomNumber) &&
                roomOccupancy[roomNumber].contains(cellDate)) {
                item->setBackground(QBrush(QColor(144, 238, 144))); // светло-зеленый
                item->setText("Занят");
            } else {
                item->setBackground(QBrush(QColor(255, 255, 255))); // белый
                item->setText("Свободен");
            }

            item->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(row, col, item);
        }
    }

    // Настройка ширины столбцов
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    for (int i = 1; i < ui->tableWidget->columnCount(); i++) {
        ui->tableWidget->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Fixed);
        ui->tableWidget->horizontalHeader()->resizeSection(i, 70);
    }

    // Делаем первую колонку (номера) нередактируемой
    for (int row = 0; row < ui->tableWidget->rowCount(); row++) {
        if (ui->tableWidget->item(row, 0)) {
            ui->tableWidget->item(row, 0)->setFlags(ui->tableWidget->item(row, 0)->flags() & ~Qt::ItemIsEditable);
        }
    }
}

void HotelManager::fillSampleData()
{
    // Очищаем данные
    roomOccupancy.clear();

    // Заполняем тестовыми данными
    // Комната 101 занята с 1 по 5 число текущего месяца
    for (int i = 0; i < 5; i++) {
        QDate occupiedDate = QDate::currentDate().addDays(i);
        roomOccupancy[101].insert(occupiedDate);
    }

    // Комната 102 занята с 3 по 10 число
    for (int i = 2; i < 10; i++) {
        QDate occupiedDate = QDate::currentDate().addDays(i);
        roomOccupancy[102].insert(occupiedDate);
    }

    // Комната 105 занята с 15 по 20 число
    for (int i = 14; i < 20; i++) {
        QDate occupiedDate = QDate::currentDate().addDays(i);
        roomOccupancy[105].insert(occupiedDate);
    }

    // Комната 110 занята на все нечетные дни
    for (int i = 0; i < 30; i += 2) {
        QDate occupiedDate = QDate::currentDate().addDays(i);
        roomOccupancy[110].insert(occupiedDate);
    }

    // Комната 115 занята с 25 по 29 число
    for (int i = 24; i < 29; i++) {
        QDate occupiedDate = QDate::currentDate().addDays(i);
        roomOccupancy[115].insert(occupiedDate);
    }

    // Комната 120 занята весь месяц
    for (int i = 0; i < 30; i++) {
        QDate occupiedDate = QDate::currentDate().addDays(i);
        roomOccupancy[120].insert(occupiedDate);
    }
}

