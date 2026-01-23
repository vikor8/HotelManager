#ifndef HOTELMANAGER_UI_H
#define HOTELMANAGER_UI_H

#include <QWidget>
#include <QPoint>  // Добавить эту строку

// Вместо forward declaration HotelManager, если он определен в другом файле
class HotelManager;

namespace Ui {
class HotelManagerUI;
}

class HotelManagerUI : public QWidget
{
    Q_OBJECT

public:
    explicit HotelManagerUI(QWidget *parent = nullptr);
    ~HotelManagerUI();

    // Изменить сигнатуру метода - убрать static или сделать правильно
    void showContextMenu(const QPoint &pos);

private:
    Ui::HotelManagerUI *ui;
    HotelManager *manager; // Если нужен доступ к менеджеру
};

#endif // HOTELMANAGER_UI_H
