#ifndef HOTELMANAGER_UI_H
#define HOTELMANAGER_UI_H

class HotelManager;

class HotelManagerUI
{
public:
    static void initUI(HotelManager *manager);
    static void initMenuBar(HotelManager *manager);
    static void showContextMenu(HotelManager *manager, const QPoint &pos);
    static void updateTableHeaders(HotelManager *manager);

private:
    static void setupTableWidget(HotelManager *manager);
    static void setupDateWidget(HotelManager *manager);
    static void createFileMenu(HotelManager *manager);
    static void createClientsMenu(HotelManager *manager);
    static void createBookingMenu(HotelManager *manager);
    static void createServicesMenu(HotelManager *manager);
    static void createReportsMenu(HotelManager *manager);
};

#endif // HOTELMANAGER_UI_H
