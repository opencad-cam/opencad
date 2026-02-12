/****************************************************************************
** Meta object code from reading C++ file 'Viewport3D.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../src/ui/viewport/Viewport3D.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Viewport3D.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN7opencad2ui10Viewport3DE_t {};
} // unnamed namespace

template <> constexpr inline auto opencad::ui::Viewport3D::qt_create_metaobjectdata<qt_meta_tag_ZN7opencad2ui10Viewport3DE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "opencad::ui::Viewport3D",
        "faceSelected",
        "",
        "edgeSelected",
        "selectionCleared",
        "geometrySelected",
        "type",
        "shapeSelected",
        "TopoDS_Shape",
        "shape",
        "opencascade::handle<AIS_InteractiveObject>",
        "object",
        "componentDragStarted",
        "componentDragged",
        "gp_Vec",
        "delta",
        "componentDragEnded",
        "gp_Pnt",
        "dropPoint"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'faceSelected'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'edgeSelected'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'selectionCleared'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'geometrySelected'
        QtMocHelpers::SignalData<void(const QString &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 },
        }}),
        // Signal 'shapeSelected'
        QtMocHelpers::SignalData<void(const TopoDS_Shape &, opencascade::handle<AIS_InteractiveObject>)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 8, 9 }, { 0x80000000 | 10, 11 },
        }}),
        // Signal 'componentDragStarted'
        QtMocHelpers::SignalData<void(opencascade::handle<AIS_InteractiveObject>)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 10, 11 },
        }}),
        // Signal 'componentDragged'
        QtMocHelpers::SignalData<void(opencascade::handle<AIS_InteractiveObject>, gp_Vec)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 10, 11 }, { 0x80000000 | 14, 15 },
        }}),
        // Signal 'componentDragEnded'
        QtMocHelpers::SignalData<void(opencascade::handle<AIS_InteractiveObject>, gp_Pnt)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 10, 11 }, { 0x80000000 | 17, 18 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<Viewport3D, qt_meta_tag_ZN7opencad2ui10Viewport3DE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject opencad::ui::Viewport3D::staticMetaObject = { {
    QMetaObject::SuperData::link<QOpenGLWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7opencad2ui10Viewport3DE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7opencad2ui10Viewport3DE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7opencad2ui10Viewport3DE_t>.metaTypes,
    nullptr
} };

void opencad::ui::Viewport3D::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<Viewport3D *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->faceSelected(); break;
        case 1: _t->edgeSelected(); break;
        case 2: _t->selectionCleared(); break;
        case 3: _t->geometrySelected((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->shapeSelected((*reinterpret_cast<std::add_pointer_t<TopoDS_Shape>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<opencascade::handle<AIS_InteractiveObject>>>(_a[2]))); break;
        case 5: _t->componentDragStarted((*reinterpret_cast<std::add_pointer_t<opencascade::handle<AIS_InteractiveObject>>>(_a[1]))); break;
        case 6: _t->componentDragged((*reinterpret_cast<std::add_pointer_t<opencascade::handle<AIS_InteractiveObject>>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<gp_Vec>>(_a[2]))); break;
        case 7: _t->componentDragEnded((*reinterpret_cast<std::add_pointer_t<opencascade::handle<AIS_InteractiveObject>>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<gp_Pnt>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (Viewport3D::*)()>(_a, &Viewport3D::faceSelected, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (Viewport3D::*)()>(_a, &Viewport3D::edgeSelected, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (Viewport3D::*)()>(_a, &Viewport3D::selectionCleared, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (Viewport3D::*)(const QString & )>(_a, &Viewport3D::geometrySelected, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (Viewport3D::*)(const TopoDS_Shape & , opencascade::handle<AIS_InteractiveObject> )>(_a, &Viewport3D::shapeSelected, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (Viewport3D::*)(opencascade::handle<AIS_InteractiveObject> )>(_a, &Viewport3D::componentDragStarted, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (Viewport3D::*)(opencascade::handle<AIS_InteractiveObject> , gp_Vec )>(_a, &Viewport3D::componentDragged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (Viewport3D::*)(opencascade::handle<AIS_InteractiveObject> , gp_Pnt )>(_a, &Viewport3D::componentDragEnded, 7))
            return;
    }
}

const QMetaObject *opencad::ui::Viewport3D::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *opencad::ui::Viewport3D::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7opencad2ui10Viewport3DE_t>.strings))
        return static_cast<void*>(this);
    return QOpenGLWidget::qt_metacast(_clname);
}

int opencad::ui::Viewport3D::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QOpenGLWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void opencad::ui::Viewport3D::faceSelected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void opencad::ui::Viewport3D::edgeSelected()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void opencad::ui::Viewport3D::selectionCleared()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void opencad::ui::Viewport3D::geometrySelected(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void opencad::ui::Viewport3D::shapeSelected(const TopoDS_Shape & _t1, opencascade::handle<AIS_InteractiveObject> _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2);
}

// SIGNAL 5
void opencad::ui::Viewport3D::componentDragStarted(opencascade::handle<AIS_InteractiveObject> _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void opencad::ui::Viewport3D::componentDragged(opencascade::handle<AIS_InteractiveObject> _t1, gp_Vec _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1, _t2);
}

// SIGNAL 7
void opencad::ui::Viewport3D::componentDragEnded(opencascade::handle<AIS_InteractiveObject> _t1, gp_Pnt _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1, _t2);
}
QT_WARNING_POP
