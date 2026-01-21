/********************************************************************************
** Form generated from reading UI file 'hotelmanager.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_HOTELMANAGER_H
#define UI_HOTELMANAGER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_HotelManager
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QDateEdit *dateEdit;
    QSpacerItem *horizontalSpacer;
    QTableWidget *tableWidget;

    void setupUi(QMainWindow *HotelManager)
    {
        if (HotelManager->objectName().isEmpty())
            HotelManager->setObjectName("HotelManager");
        HotelManager->resize(1200, 600);
        centralwidget = new QWidget(HotelManager);
        centralwidget->setObjectName("centralwidget");
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName("verticalLayout");
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        label = new QLabel(centralwidget);
        label->setObjectName("label");

        horizontalLayout->addWidget(label);

        dateEdit = new QDateEdit(centralwidget);
        dateEdit->setObjectName("dateEdit");

        horizontalLayout->addWidget(dateEdit);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        verticalLayout->addLayout(horizontalLayout);

        tableWidget = new QTableWidget(centralwidget);
        tableWidget->setObjectName("tableWidget");
        tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        tableWidget->setSelectionBehavior(QAbstractItemView::SelectItems);

        verticalLayout->addWidget(tableWidget);

        HotelManager->setCentralWidget(centralwidget);

        retranslateUi(HotelManager);

        QMetaObject::connectSlotsByName(HotelManager);
    } // setupUi

    void retranslateUi(QMainWindow *HotelManager)
    {
        HotelManager->setWindowTitle(QCoreApplication::translate("HotelManager", "Hotel Manager", nullptr));
        label->setText(QCoreApplication::translate("HotelManager", "\320\235\320\260\321\207\320\260\320\273\321\214\320\275\320\260\321\217 \320\264\320\260\321\202\320\260:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class HotelManager: public Ui_HotelManager {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_HOTELMANAGER_H
