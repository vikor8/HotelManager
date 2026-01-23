#include "hotelmanager_ui.h"
#include "hotelmanager.h"
#include "ui_hotelmanager.h"
#include "hotelmanager_booking.h"
#include "hotelmanager_rooms.h"
#include "hotelmanager_reports.h"
#include "hotelmanager_clients.h"

#include <QDateEdit>
#include <QHeaderView>
#include <QBrush>
#include <QColor>
#include <QMessageBox>
#include <QMenuBar>
#include <QKeySequence>
#include <QMenu>
#include <QAction>
#include <QStatusBar>

void HotelManagerUI::initUI(HotelManager *manager)
{
    Ui::HotelManager *ui = manager->getUI();

    // Устанавливаем текущую дату
    manager->setStartDate(QDate::currentDate());

    // Настройка виджетов
    setupDateWidget(manager);
    setupTableWidget(manager);

    // Инициализация меню
    initMenuBar(manager);
}

void HotelManagerUI::setupDateWidget(HotelManager *manager)
{
    Ui::HotelManager *ui = manager->getUI();
    ui->dateEdit->setDate(manager->getStartDate());
    ui->dateEdit->setCalendarPopup(true);
}

void HotelManagerUI::setupTableWidget(HotelManager *manager)
{
    Ui::HotelManager *ui = manager->getUI();
    ui->tableWidget->setColumnCount(31);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "Комнаты");
}

void HotelManagerUI::initMenuBar(HotelManager *manager)
{
    QMenuBar *menuBar = new QMenuBar(manager);
    manager->setMenuBar(menuBar);

    createFileMenu(manager);
    createClientsMenu(manager);
    createBookingMenu(manager);
    createServicesMenu(manager);
    createReportsMenu(manager);
}

void HotelManagerUI::createFileMenu(HotelManager *manager)
{
    QMenuBar *menuBar = manager->menuBar();
    QMenu *fileMenu = menuBar->addMenu("&Файл");

    QAction *addRoomAction = new QAction("&Добавить комнату", manager);
    addRoomAction->setShortcut(QKeySequence("Ctrl+N"));
    QObject::connect(addRoomAction, &QAction::triggered, manager, [manager]() {
        HotelManagerRooms::addRoom(manager);
    });
    fileMenu->addAction(addRoomAction);

    QAction *deleteRoomAction = new QAction("&Удалить комнату", manager);
    deleteRoomAction->setShortcut(QKeySequence("Ctrl+D"));
    QObject::connect(deleteRoomAction, &QAction::triggered, manager, [manager]() {
        HotelManagerRooms::deleteRoom(manager);
    });
    fileMenu->addAction(deleteRoomAction);

    fileMenu->addSeparator();

    QAction *exitAction = new QAction("&Выход", manager);
    exitAction->setShortcut(QKeySequence::Quit);
    QObject::connect(exitAction, &QAction::triggered, manager, &QWidget::close);
    fileMenu->addAction(exitAction);
}

void HotelManagerUI::createClientsMenu(HotelManager *manager)
{
    QMenuBar *menuBar = manager->menuBar();
    QMenu *clientsMenu = menuBar->addMenu("&Клиенты");

    QAction *addClientAction = new QAction("&Добавить клиента", manager);
    addClientAction->setShortcut(QKeySequence("Ctrl+C"));
    QObject::connect(addClientAction, &QAction::triggered, manager, [manager]() {
        HotelManagerClients::manageClients(manager);
    });
    clientsMenu->addAction(addClientAction);

    QAction *viewClientsAction = new QAction("&Список клиентов", manager);
    viewClientsAction->setShortcut(QKeySequence("Ctrl+L"));
    QObject::connect(viewClientsAction, &QAction::triggered, manager, [manager]() {
        HotelManagerClients::manageClients(manager);
    });
    clientsMenu->addAction(viewClientsAction);
}

void HotelManagerUI::createBookingMenu(HotelManager *manager)
{
    QMenuBar *menuBar = manager->menuBar();
    QMenu *bookingMenu = menuBar->addMenu("&Бронирование");

    QAction *newBookingAction = new QAction("&Новое бронирование", manager);
    newBookingAction->setShortcut(QKeySequence("Ctrl+B"));
    QObject::connect(newBookingAction, &QAction::triggered, manager, [manager]() {
        HotelManagerBooking::addBooking(manager);
    });
    bookingMenu->addAction(newBookingAction);
}

void HotelManagerUI::createServicesMenu(HotelManager *manager)
{
    QMenuBar *menuBar = manager->menuBar();
    QMenu *servicesMenu = menuBar->addMenu("&Доп услуги");

    QAction *manageServicesAction = new QAction("&Управление услугами", manager);
    QObject::connect(manageServicesAction, &QAction::triggered, manager, [manager]() {
        HotelManagerClients::manageServices(manager);
    });
    servicesMenu->addAction(manageServicesAction);
}

void HotelManagerUI::createReportsMenu(HotelManager *manager)
{
    QMenuBar *menuBar = manager->menuBar();
    QMenu *reportsMenu = menuBar->addMenu("&Обзор");

    QAction *viewReportsAction = new QAction("&Отчеты", manager);
    QObject::connect(viewReportsAction, &QAction::triggered, manager, [manager]() {
        HotelManagerReports::viewReports(manager);
    });
    reportsMenu->addAction(viewReportsAction);
}

void HotelManagerUI::showContextMenu(HotelManager *manager, const QPoint &pos)
{
    Ui::HotelManager *ui = manager->getUI();
    QMenu menu(manager);

    QModelIndexList selected = ui->tableWidget->selectionModel()->selectedIndexes();
    if (selected.isEmpty()) {
        menu.addAction("Нет выбранной ячейки");
    } else {
        QModelIndex index = selected.first();
        int row = index.row();
        int col = index.column();

        if (col > 0) {
            QString roomText = ui->tableWidget->item(row, 0)->text();
            int roomNumber = HotelUtils::extractRoomNumber(roomText);
            QDate cellDate = manager->getStartDate().addDays(col - 1);

            if (HotelManagerBooking::isRoomOccupied(roomNumber, cellDate, manager->getDatabase())) {
                menu.addAction("Снять бронь", manager, [manager]() {
                    HotelManagerBooking::removeBooking(manager);
                });
            } else {
                menu.addAction("Забронировать", manager, [manager]() {
                    HotelManagerBooking::addBooking(manager);
                });
            }
        }
    }

    menu.addSeparator();
    menu.addAction("Информация о комнате", manager, [manager]() {
        HotelManagerRooms::showRoomInfo(manager);
    });

    menu.exec(ui->tableWidget->viewport()->mapToGlobal(pos));
}

void HotelManagerUI::updateTableHeaders(HotelManager *manager)
{
    Ui::HotelManager *ui = manager->getUI();

    // Очищаем ячейки
    for (int row = 0; row < ui->tableWidget->rowCount(); row++) {
        for (int col = 1; col < ui->tableWidget->columnCount(); col++) {
            if (ui->tableWidget->item(row, col)) {
                delete ui->tableWidget->item(row, col);
            }
        }
    }

    // Устанавливаем заголовки
    QStringList headers;
    headers << "Комнаты";

    for (int i = 0; i < 30; i++) {
        QDate currentDate = manager->getStartDate().addDays(i);
        headers << HotelUtils::formatDateDisplay(currentDate);
    }

    ui->tableWidget->setHorizontalHeaderLabels(headers);

    // Заполняем таблицу
    for (int row = 0; row < ui->tableWidget->rowCount(); row++) {
        QString roomText = ui->tableWidget->item(row, 0)->text();
        int roomNumber = HotelUtils::extractRoomNumber(roomText);

        for (int col = 1; col < ui->tableWidget->columnCount(); col++) {
            QDate cellDate = manager->getStartDate().addDays(col - 1);
            QTableWidgetItem *item = new QTableWidgetItem();

            if (HotelManagerBooking::isRoomOccupied(roomNumber, cellDate, manager->getDatabase())) {
                item->setBackground(QBrush(QColor(144, 238, 144)));
                item->setText("Занят");
            } else {
                item->setBackground(QBrush(QColor(255, 255, 255)));
                item->setText("Свободен");
            }

            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            ui->tableWidget->setItem(row, col, item);
        }
    }

    // Настройка ширины столбцов
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    for (int i = 1; i < ui->tableWidget->columnCount(); i++) {
        ui->tableWidget->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Fixed);
        ui->tableWidget->horizontalHeader()->resizeSection(i, 80);
    }

    ui->tableWidget->verticalHeader()->setDefaultSectionSize(30);
}
