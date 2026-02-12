/****************************************************************************
** Meta object code from reading C++ file 'ToolSettingsPanel.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../src/ui/ToolSettingsPanel.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ToolSettingsPanel.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN7opencad2ui17ToolSettingsPanelE_t {};
} // unnamed namespace

template <> constexpr inline auto opencad::ui::ToolSettingsPanel::qt_create_metaobjectdata<qt_meta_tag_ZN7opencad2ui17ToolSettingsPanelE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "opencad::ui::ToolSettingsPanel",
        "polygonSidesChanged",
        "",
        "sides",
        "slotWidthChanged",
        "width",
        "autoConstraintEnabledChanged",
        "enabled",
        "autoConstraintToleranceChanged",
        "degrees",
        "polygonInscribedChanged",
        "inscribed",
        "settingsChanged",
        "applyClicked",
        "onPolygonSidesChanged",
        "value",
        "onSlotWidthChanged",
        "onAutoConstraintToggled",
        "checked",
        "onToleranceChanged",
        "onPolygonInscribedToggled",
        "onSettingsChanged"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'polygonSidesChanged'
        QtMocHelpers::SignalData<void(int)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Signal 'slotWidthChanged'
        QtMocHelpers::SignalData<void(double)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 5 },
        }}),
        // Signal 'autoConstraintEnabledChanged'
        QtMocHelpers::SignalData<void(bool)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 7 },
        }}),
        // Signal 'autoConstraintToleranceChanged'
        QtMocHelpers::SignalData<void(double)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 9 },
        }}),
        // Signal 'polygonInscribedChanged'
        QtMocHelpers::SignalData<void(bool)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 11 },
        }}),
        // Signal 'settingsChanged'
        QtMocHelpers::SignalData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'applyClicked'
        QtMocHelpers::SignalData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onPolygonSidesChanged'
        QtMocHelpers::SlotData<void(int)>(14, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 15 },
        }}),
        // Slot 'onSlotWidthChanged'
        QtMocHelpers::SlotData<void(double)>(16, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 15 },
        }}),
        // Slot 'onAutoConstraintToggled'
        QtMocHelpers::SlotData<void(bool)>(17, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 18 },
        }}),
        // Slot 'onToleranceChanged'
        QtMocHelpers::SlotData<void(double)>(19, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 15 },
        }}),
        // Slot 'onPolygonInscribedToggled'
        QtMocHelpers::SlotData<void(bool)>(20, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 18 },
        }}),
        // Slot 'onSettingsChanged'
        QtMocHelpers::SlotData<void()>(21, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ToolSettingsPanel, qt_meta_tag_ZN7opencad2ui17ToolSettingsPanelE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject opencad::ui::ToolSettingsPanel::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7opencad2ui17ToolSettingsPanelE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7opencad2ui17ToolSettingsPanelE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7opencad2ui17ToolSettingsPanelE_t>.metaTypes,
    nullptr
} };

void opencad::ui::ToolSettingsPanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ToolSettingsPanel *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->polygonSidesChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->slotWidthChanged((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 2: _t->autoConstraintEnabledChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 3: _t->autoConstraintToleranceChanged((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 4: _t->polygonInscribedChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 5: _t->settingsChanged(); break;
        case 6: _t->applyClicked(); break;
        case 7: _t->onPolygonSidesChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 8: _t->onSlotWidthChanged((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 9: _t->onAutoConstraintToggled((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 10: _t->onToleranceChanged((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 11: _t->onPolygonInscribedToggled((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 12: _t->onSettingsChanged(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ToolSettingsPanel::*)(int )>(_a, &ToolSettingsPanel::polygonSidesChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ToolSettingsPanel::*)(double )>(_a, &ToolSettingsPanel::slotWidthChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (ToolSettingsPanel::*)(bool )>(_a, &ToolSettingsPanel::autoConstraintEnabledChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (ToolSettingsPanel::*)(double )>(_a, &ToolSettingsPanel::autoConstraintToleranceChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (ToolSettingsPanel::*)(bool )>(_a, &ToolSettingsPanel::polygonInscribedChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (ToolSettingsPanel::*)()>(_a, &ToolSettingsPanel::settingsChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (ToolSettingsPanel::*)()>(_a, &ToolSettingsPanel::applyClicked, 6))
            return;
    }
}

const QMetaObject *opencad::ui::ToolSettingsPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *opencad::ui::ToolSettingsPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7opencad2ui17ToolSettingsPanelE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int opencad::ui::ToolSettingsPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void opencad::ui::ToolSettingsPanel::polygonSidesChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void opencad::ui::ToolSettingsPanel::slotWidthChanged(double _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void opencad::ui::ToolSettingsPanel::autoConstraintEnabledChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void opencad::ui::ToolSettingsPanel::autoConstraintToleranceChanged(double _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void opencad::ui::ToolSettingsPanel::polygonInscribedChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void opencad::ui::ToolSettingsPanel::settingsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void opencad::ui::ToolSettingsPanel::applyClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}
QT_WARNING_POP
