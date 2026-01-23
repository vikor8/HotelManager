/****************************************************************************
** Meta object code from reading C++ file 'hotelmanager.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../hotelmanager.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'hotelmanager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_HotelManager_t {
    uint offsetsAndSizes[30];
    char stringdata0[13];
    char stringdata1[14];
    char stringdata2[1];
    char stringdata3[15];
    char stringdata4[12];
    char stringdata5[6];
    char stringdata6[16];
    char stringdata7[4];
    char stringdata8[8];
    char stringdata9[11];
    char stringdata10[11];
    char stringdata11[14];
    char stringdata12[14];
    char stringdata13[15];
    char stringdata14[12];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_HotelManager_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_HotelManager_t qt_meta_stringdata_HotelManager = {
    {
        QT_MOC_LITERAL(0, 12),  // "HotelManager"
        QT_MOC_LITERAL(13, 13),  // "onDateChanged"
        QT_MOC_LITERAL(27, 0),  // ""
        QT_MOC_LITERAL(28, 14),  // "onTableClicked"
        QT_MOC_LITERAL(43, 11),  // "QModelIndex"
        QT_MOC_LITERAL(55, 5),  // "index"
        QT_MOC_LITERAL(61, 15),  // "showContextMenu"
        QT_MOC_LITERAL(77, 3),  // "pos"
        QT_MOC_LITERAL(81, 7),  // "addRoom"
        QT_MOC_LITERAL(89, 10),  // "deleteRoom"
        QT_MOC_LITERAL(100, 10),  // "addBooking"
        QT_MOC_LITERAL(111, 13),  // "removeBooking"
        QT_MOC_LITERAL(125, 13),  // "manageClients"
        QT_MOC_LITERAL(139, 14),  // "manageServices"
        QT_MOC_LITERAL(154, 11)   // "viewReports"
    },
    "HotelManager",
    "onDateChanged",
    "",
    "onTableClicked",
    "QModelIndex",
    "index",
    "showContextMenu",
    "pos",
    "addRoom",
    "deleteRoom",
    "addBooking",
    "removeBooking",
    "manageClients",
    "manageServices",
    "viewReports"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_HotelManager[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   74,    2, 0x08,    1 /* Private */,
       3,    1,   75,    2, 0x08,    2 /* Private */,
       6,    1,   78,    2, 0x08,    4 /* Private */,
       8,    0,   81,    2, 0x08,    6 /* Private */,
       9,    0,   82,    2, 0x08,    7 /* Private */,
      10,    0,   83,    2, 0x08,    8 /* Private */,
      11,    0,   84,    2, 0x08,    9 /* Private */,
      12,    0,   85,    2, 0x08,   10 /* Private */,
      13,    0,   86,    2, 0x08,   11 /* Private */,
      14,    0,   87,    2, 0x08,   12 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void, QMetaType::QPoint,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject HotelManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_HotelManager.offsetsAndSizes,
    qt_meta_data_HotelManager,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_HotelManager_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<HotelManager, std::true_type>,
        // method 'onDateChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onTableClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QModelIndex &, std::false_type>,
        // method 'showContextMenu'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QPoint &, std::false_type>,
        // method 'addRoom'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'deleteRoom'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'addBooking'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'removeBooking'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'manageClients'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'manageServices'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'viewReports'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void HotelManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<HotelManager *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onDateChanged(); break;
        case 1: _t->onTableClicked((*reinterpret_cast< std::add_pointer_t<QModelIndex>>(_a[1]))); break;
        case 2: _t->showContextMenu((*reinterpret_cast< std::add_pointer_t<QPoint>>(_a[1]))); break;
        case 3: _t->addRoom(); break;
        case 4: _t->deleteRoom(); break;
        case 5: _t->addBooking(); break;
        case 6: _t->removeBooking(); break;
        case 7: _t->manageClients(); break;
        case 8: _t->manageServices(); break;
        case 9: _t->viewReports(); break;
        default: ;
        }
    }
}

const QMetaObject *HotelManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *HotelManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_HotelManager.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int HotelManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 10;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
