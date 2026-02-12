/****************************************************************************
** Meta object code from reading C++ file 'AssemblyTreeWidget.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../src/ui/AssemblyTreeWidget.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AssemblyTreeWidget.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN7opencad2ui18AssemblyTreeWidgetE_t {};
} // unnamed namespace

template <> constexpr inline auto opencad::ui::AssemblyTreeWidget::qt_create_metaobjectdata<qt_meta_tag_ZN7opencad2ui18AssemblyTreeWidgetE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "opencad::ui::AssemblyTreeWidget",
        "componentSelected",
        "",
        "std::shared_ptr<assembly::Component>",
        "component",
        "componentsSelected",
        "std::vector<std::shared_ptr<assembly::Component>>",
        "components",
        "constraintSelected",
        "std::shared_ptr<assembly::AssemblyConstraint>",
        "constraint",
        "visibilityChanged",
        "structChanged",
        "onItemDoubleClicked",
        "QTreeWidgetItem*",
        "item",
        "column",
        "onItemChanged",
        "onCustomContextMenuRequested",
        "QPoint",
        "pos",
        "onSelectionChanged"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'componentSelected'
        QtMocHelpers::SignalData<void(std::shared_ptr<assembly::Component>)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'componentsSelected'
        QtMocHelpers::SignalData<void(std::vector<std::shared_ptr<assembly::Component>>)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 6, 7 },
        }}),
        // Signal 'constraintSelected'
        QtMocHelpers::SignalData<void(std::shared_ptr<assembly::AssemblyConstraint>)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 9, 10 },
        }}),
        // Signal 'visibilityChanged'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'structChanged'
        QtMocHelpers::SignalData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onItemDoubleClicked'
        QtMocHelpers::SlotData<void(QTreeWidgetItem *, int)>(13, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 14, 15 }, { QMetaType::Int, 16 },
        }}),
        // Slot 'onItemChanged'
        QtMocHelpers::SlotData<void(QTreeWidgetItem *, int)>(17, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 14, 15 }, { QMetaType::Int, 16 },
        }}),
        // Slot 'onCustomContextMenuRequested'
        QtMocHelpers::SlotData<void(const QPoint &)>(18, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 19, 20 },
        }}),
        // Slot 'onSelectionChanged'
        QtMocHelpers::SlotData<void()>(21, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AssemblyTreeWidget, qt_meta_tag_ZN7opencad2ui18AssemblyTreeWidgetE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject opencad::ui::AssemblyTreeWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QTreeWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7opencad2ui18AssemblyTreeWidgetE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7opencad2ui18AssemblyTreeWidgetE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7opencad2ui18AssemblyTreeWidgetE_t>.metaTypes,
    nullptr
} };

void opencad::ui::AssemblyTreeWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AssemblyTreeWidget *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->componentSelected((*reinterpret_cast<std::add_pointer_t<std::shared_ptr<assembly::Component>>>(_a[1]))); break;
        case 1: _t->componentsSelected((*reinterpret_cast<std::add_pointer_t<std::vector<std::shared_ptr<assembly::Component>>>>(_a[1]))); break;
        case 2: _t->constraintSelected((*reinterpret_cast<std::add_pointer_t<std::shared_ptr<assembly::AssemblyConstraint>>>(_a[1]))); break;
        case 3: _t->visibilityChanged(); break;
        case 4: _t->structChanged(); break;
        case 5: _t->onItemDoubleClicked((*reinterpret_cast<std::add_pointer_t<QTreeWidgetItem*>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 6: _t->onItemChanged((*reinterpret_cast<std::add_pointer_t<QTreeWidgetItem*>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 7: _t->onCustomContextMenuRequested((*reinterpret_cast<std::add_pointer_t<QPoint>>(_a[1]))); break;
        case 8: _t->onSelectionChanged(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (AssemblyTreeWidget::*)(std::shared_ptr<assembly::Component> )>(_a, &AssemblyTreeWidget::componentSelected, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (AssemblyTreeWidget::*)(std::vector<std::shared_ptr<assembly::Component>> )>(_a, &AssemblyTreeWidget::componentsSelected, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (AssemblyTreeWidget::*)(std::shared_ptr<assembly::AssemblyConstraint> )>(_a, &AssemblyTreeWidget::constraintSelected, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (AssemblyTreeWidget::*)()>(_a, &AssemblyTreeWidget::visibilityChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (AssemblyTreeWidget::*)()>(_a, &AssemblyTreeWidget::structChanged, 4))
            return;
    }
}

const QMetaObject *opencad::ui::AssemblyTreeWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *opencad::ui::AssemblyTreeWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7opencad2ui18AssemblyTreeWidgetE_t>.strings))
        return static_cast<void*>(this);
    return QTreeWidget::qt_metacast(_clname);
}

int opencad::ui::AssemblyTreeWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTreeWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void opencad::ui::AssemblyTreeWidget::componentSelected(std::shared_ptr<assembly::Component> _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void opencad::ui::AssemblyTreeWidget::componentsSelected(std::vector<std::shared_ptr<assembly::Component>> _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void opencad::ui::AssemblyTreeWidget::constraintSelected(std::shared_ptr<assembly::AssemblyConstraint> _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void opencad::ui::AssemblyTreeWidget::visibilityChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void opencad::ui::AssemblyTreeWidget::structChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}
QT_WARNING_POP
