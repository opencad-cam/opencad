/****************************************************************************
** Meta object code from reading C++ file 'SketchView2D.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../src/ui/sketch/SketchView2D.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SketchView2D.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN7opencad2ui12SketchView2DE_t {};
} // unnamed namespace

template <> constexpr inline auto opencad::ui::SketchView2D::qt_create_metaobjectdata<qt_meta_tag_ZN7opencad2ui12SketchView2DE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "opencad::ui::SketchView2D",
        "entityCreated",
        "",
        "sketch::SketchEntity*",
        "entity",
        "entitySelected",
        "toolChanged",
        "SketchToolType",
        "tool",
        "cursorPositionChanged",
        "x",
        "y",
        "sketchExitRequested",
        "profileSelected",
        "profileIndex",
        "ringSelected",
        "outerProfileIndex",
        "innerProfileIndex",
        "multiProfilesConfirmed",
        "std::vector<std::pair<int,int>>",
        "selections",
        "profileHovered",
        "profileSelectionConfirmed",
        "profileSelectionCancelled"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'entityCreated'
        QtMocHelpers::SignalData<void(sketch::SketchEntity *)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'entitySelected'
        QtMocHelpers::SignalData<void(sketch::SketchEntity *)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'toolChanged'
        QtMocHelpers::SignalData<void(SketchToolType)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 7, 8 },
        }}),
        // Signal 'cursorPositionChanged'
        QtMocHelpers::SignalData<void(double, double)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 10 }, { QMetaType::Double, 11 },
        }}),
        // Signal 'sketchExitRequested'
        QtMocHelpers::SignalData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'profileSelected'
        QtMocHelpers::SignalData<void(int)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 14 },
        }}),
        // Signal 'ringSelected'
        QtMocHelpers::SignalData<void(int, int)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 16 }, { QMetaType::Int, 17 },
        }}),
        // Signal 'multiProfilesConfirmed'
        QtMocHelpers::SignalData<void(const std::vector<std::pair<int,int>> &)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 19, 20 },
        }}),
        // Signal 'profileHovered'
        QtMocHelpers::SignalData<void(int)>(21, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 14 },
        }}),
        // Signal 'profileSelectionConfirmed'
        QtMocHelpers::SignalData<void()>(22, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'profileSelectionCancelled'
        QtMocHelpers::SignalData<void()>(23, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<SketchView2D, qt_meta_tag_ZN7opencad2ui12SketchView2DE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject opencad::ui::SketchView2D::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7opencad2ui12SketchView2DE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7opencad2ui12SketchView2DE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7opencad2ui12SketchView2DE_t>.metaTypes,
    nullptr
} };

void opencad::ui::SketchView2D::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SketchView2D *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->entityCreated((*reinterpret_cast<std::add_pointer_t<sketch::SketchEntity*>>(_a[1]))); break;
        case 1: _t->entitySelected((*reinterpret_cast<std::add_pointer_t<sketch::SketchEntity*>>(_a[1]))); break;
        case 2: _t->toolChanged((*reinterpret_cast<std::add_pointer_t<SketchToolType>>(_a[1]))); break;
        case 3: _t->cursorPositionChanged((*reinterpret_cast<std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2]))); break;
        case 4: _t->sketchExitRequested(); break;
        case 5: _t->profileSelected((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 6: _t->ringSelected((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 7: _t->multiProfilesConfirmed((*reinterpret_cast<std::add_pointer_t<std::vector<std::pair<int,int>>>>(_a[1]))); break;
        case 8: _t->profileHovered((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 9: _t->profileSelectionConfirmed(); break;
        case 10: _t->profileSelectionCancelled(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (SketchView2D::*)(sketch::SketchEntity * )>(_a, &SketchView2D::entityCreated, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (SketchView2D::*)(sketch::SketchEntity * )>(_a, &SketchView2D::entitySelected, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (SketchView2D::*)(SketchToolType )>(_a, &SketchView2D::toolChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (SketchView2D::*)(double , double )>(_a, &SketchView2D::cursorPositionChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (SketchView2D::*)()>(_a, &SketchView2D::sketchExitRequested, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (SketchView2D::*)(int )>(_a, &SketchView2D::profileSelected, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (SketchView2D::*)(int , int )>(_a, &SketchView2D::ringSelected, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (SketchView2D::*)(const std::vector<std::pair<int,int>> & )>(_a, &SketchView2D::multiProfilesConfirmed, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (SketchView2D::*)(int )>(_a, &SketchView2D::profileHovered, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (SketchView2D::*)()>(_a, &SketchView2D::profileSelectionConfirmed, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (SketchView2D::*)()>(_a, &SketchView2D::profileSelectionCancelled, 10))
            return;
    }
}

const QMetaObject *opencad::ui::SketchView2D::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *opencad::ui::SketchView2D::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7opencad2ui12SketchView2DE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int opencad::ui::SketchView2D::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void opencad::ui::SketchView2D::entityCreated(sketch::SketchEntity * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void opencad::ui::SketchView2D::entitySelected(sketch::SketchEntity * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void opencad::ui::SketchView2D::toolChanged(SketchToolType _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void opencad::ui::SketchView2D::cursorPositionChanged(double _t1, double _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2);
}

// SIGNAL 4
void opencad::ui::SketchView2D::sketchExitRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void opencad::ui::SketchView2D::profileSelected(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void opencad::ui::SketchView2D::ringSelected(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1, _t2);
}

// SIGNAL 7
void opencad::ui::SketchView2D::multiProfilesConfirmed(const std::vector<std::pair<int,int>> & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1);
}

// SIGNAL 8
void opencad::ui::SketchView2D::profileHovered(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 8, nullptr, _t1);
}

// SIGNAL 9
void opencad::ui::SketchView2D::profileSelectionConfirmed()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void opencad::ui::SketchView2D::profileSelectionCancelled()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}
QT_WARNING_POP
