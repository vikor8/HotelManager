#include "hotelmanager.h"
#include "ui_hotelmanager.h"

#include "hotelmanager_ui.h"
#include "hotelmanager_database.h"
#include "hotelmanager_booking.h"
#include "hotelmanager_rooms.h"
#include "hotelmanager_reports.h"
#include "hotelmanager_clients.h"

#include <QDateEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QRegularExpression>

HotelManager::HotelManager(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::HotelManager)
{
    ui->setupUi(this);

    // Инициализация через отдельные модули
    HotelManagerUI::initUI(this);
    HotelManagerDatabase::initDatabase(this);
    HotelManagerRooms::loadRoomsFromDB(this);
    HotelManagerBooking::loadOccupancyFromDB(this);

    // Подключение сигналов
    connect(ui->dateEdit, &QDateEdit::dateChanged, this, &HotelManager::onDateChanged);
    connect(ui->tableWidget, &QTableWidget::clicked, this, &HotelManager::onTableClicked);

    // Контекстное меню
    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableWidget, &QTableWidget::customContextMenuRequested,
            this, &HotelManager::showContextMenu);

    // Кнопки
    connect(ui->btnToday, &QPushButton::clicked, this, [this]() {
        ui->dateEdit->setDate(QDate::currentDate());
    });

    connect(ui->btnRefresh, &QPushButton::clicked, this, [this]() {
        HotelManagerBooking::loadOccupancyFromDB(this);
        updateTableHeaders();
    });
}

HotelManager::~HotelManager()
{
    if (db.isOpen()) {
        db.close();
    }
    delete ui;
}

void HotelManager::onDateChanged()
{
    startDate = ui->dateEdit->date();
    updateTableHeaders();
}

void HotelManager::onTableClicked(const QModelIndex &index)
{
    Q_UNUSED(index);
}

void HotelManager::showContextMenu(const QPoint &pos)
{
    HotelManagerUI::showContextMenu(this, pos);
}

void HotelManager::updateTableHeaders()
{
    HotelManagerUI::updateTableHeaders(this);
}

// Реализация утилит
namespace HotelUtils {
    int extractRoomNumber(const QString &roomText)
    {
        QRegularExpression re("Комната\\s+(\\d+)");
        QRegularExpressionMatch match = re.match(roomText);
        if (match.hasMatch()) {
            return match.captured(1).toInt();
        }
        return 0;
    }

    QString formatDateDisplay(const QDate &date)
    {
        return date.toString("dd.MM\nyyyy");
    }
}
