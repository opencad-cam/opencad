/****************************************************************************
** Meta object code from reading C++ file 'FeatureTree.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../src/core/FeatureTree.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FeatureTree.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN7opencad4core11FeatureTreeE_t {};
} // unnamed namespace

template <> constexpr inline auto opencad::core::FeatureTree::qt_create_metaobjectdata<qt_meta_tag_ZN7opencad4core11FeatureTreeE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "opencad::core::FeatureTree",
        "featureAdded",
        "",
        "Feature*",
        "feature",
        "featureRemoved",
        "featureModified",
        "treeStructureChanged",
        "regenerationStarted",
        "regenerationCompleted",
        "success"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'featureAdded'
        QtMocHelpers::SignalData<void(Feature *)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'featureRemoved'
        QtMocHelpers::SignalData<void(Feature *)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'featureModified'
        QtMocHelpers::SignalData<void(Feature *)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'treeStructureChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'regenerationStarted'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'regenerationCompleted'
        QtMocHelpers::SignalData<void(bool)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 10 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<FeatureTree, qt_meta_tag_ZN7opencad4core11FeatureTreeE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject opencad::core::FeatureTree::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7opencad4core11FeatureTreeE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7opencad4core11FeatureTreeE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7opencad4core11FeatureTreeE_t>.metaTypes,
    nullptr
} };

void opencad::core::FeatureTree::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<FeatureTree *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->featureAdded((*reinterpret_cast<std::add_pointer_t<Feature*>>(_a[1]))); break;
        case 1: _t->featureRemoved((*reinterpret_cast<std::add_pointer_t<Feature*>>(_a[1]))); break;
        case 2: _t->featureModified((*reinterpret_cast<std::add_pointer_t<Feature*>>(_a[1]))); break;
        case 3: _t->treeStructureChanged(); break;
        case 4: _t->regenerationStarted(); break;
        case 5: _t->regenerationCompleted((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (FeatureTree::*)(Feature * )>(_a, &FeatureTree::featureAdded, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (FeatureTree::*)(Feature * )>(_a, &FeatureTree::featureRemoved, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (FeatureTree::*)(Feature * )>(_a, &FeatureTree::featureModified, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (FeatureTree::*)()>(_a, &FeatureTree::treeStructureChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (FeatureTree::*)()>(_a, &FeatureTree::regenerationStarted, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (FeatureTree::*)(bool )>(_a, &FeatureTree::regenerationCompleted, 5))
            return;
    }
}

const QMetaObject *opencad::core::FeatureTree::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *opencad::core::FeatureTree::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7opencad4core11FeatureTreeE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int opencad::core::FeatureTree::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void opencad::core::FeatureTree::featureAdded(Feature * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void opencad::core::FeatureTree::featureRemoved(Feature * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void opencad::core::FeatureTree::featureModified(Feature * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void opencad::core::FeatureTree::treeStructureChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void opencad::core::FeatureTree::regenerationStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void opencad::core::FeatureTree::regenerationCompleted(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}
QT_WARNING_POP
