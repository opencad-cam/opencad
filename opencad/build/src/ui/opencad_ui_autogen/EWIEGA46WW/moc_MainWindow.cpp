/****************************************************************************
** Meta object code from reading C++ file 'MainWindow.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../src/ui/MainWindow.h"
#include <QtNetwork/QSslError>
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MainWindow.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN7opencad2ui10MainWindowE_t {};
} // unnamed namespace

template <> constexpr inline auto opencad::ui::MainWindow::qt_create_metaobjectdata<qt_meta_tag_ZN7opencad2ui10MainWindowE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "opencad::ui::MainWindow",
        "onNewFile",
        "",
        "onOpenFile",
        "onSaveFile",
        "onSaveAs",
        "onExportSTL",
        "onExportSTEP",
        "onExit",
        "onUndo",
        "onRedo",
        "onDelete",
        "onViewFit",
        "onViewFront",
        "onViewBack",
        "onViewTop",
        "onViewBottom",
        "onViewLeft",
        "onViewRight",
        "onViewIsometric",
        "onCreateBox",
        "onCreateCylinder",
        "onCreateSphere",
        "onCreateCone",
        "onBooleanFuse",
        "onBooleanCut",
        "onBooleanCommon",
        "onNewSketch",
        "onSketchOnFace",
        "onEditSketch",
        "onFinishSketch",
        "onFaceSelected",
        "onSketchLine",
        "onSketchRectangle",
        "onSketchCircle",
        "onSketchArc",
        "onSketchPoint",
        "onSketchSpline",
        "onSketchEllipse",
        "onSketchProject",
        "onConstraintHorizontal",
        "onConstraintVertical",
        "onConstraintCoincident",
        "onConstraintDistance",
        "onConstraintRadius",
        "onConstraintAngle",
        "onConstraintParallel",
        "onConstraintPerpendicular",
        "onReferencePlane",
        "onExtrude",
        "onCut",
        "onRevolve",
        "onFillet",
        "onChamfer",
        "onShell",
        "onSweep",
        "onLoft",
        "onLoftSurface",
        "onPattern",
        "onMirror",
        "onScale",
        "onHoleWizard",
        "onDraft",
        "onRib",
        "onThicken",
        "onOffsetSurface",
        "onSplit",
        "onDome",
        "onGear",
        "onSketchSlot",
        "onSketchPolygon",
        "onConvertEntities",
        "onIntersectionCurve",
        "onSketchMirror",
        "onSketchTrim",
        "onSketchExtend",
        "onSketchLinearPattern",
        "onSketchCircularPattern",
        "onConstraintTangent",
        "onConstraintEqual",
        "onConstraintFix",
        "onConstraintConcentric",
        "onProfileSelected",
        "profileIndex",
        "onRingSelected",
        "outerProfileIndex",
        "innerProfileIndex",
        "onMultiProfilesConfirmed",
        "std::vector<std::pair<int,int>>",
        "selections",
        "onProfileSelectionCancelled",
        "onSelectShape",
        "onSelectFace",
        "onSelectEdge",
        "onSelectVertex",
        "onAbout",
        "onToolApply",
        "onFeatureSelected",
        "QListWidgetItem*",
        "item",
        "onFeatureContextMenu",
        "QPoint",
        "pos",
        "onToggleFeatureSuppression",
        "onFeatureReordered",
        "onAssemblyTreeSelection",
        "std::shared_ptr<assembly::Component>",
        "component"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'onNewFile'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onOpenFile'
        QtMocHelpers::SlotData<void()>(3, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSaveFile'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSaveAs'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onExportSTL'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onExportSTEP'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onExit'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onUndo'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onRedo'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onDelete'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onViewFit'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onViewFront'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onViewBack'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onViewTop'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onViewBottom'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onViewLeft'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onViewRight'
        QtMocHelpers::SlotData<void()>(18, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onViewIsometric'
        QtMocHelpers::SlotData<void()>(19, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onCreateBox'
        QtMocHelpers::SlotData<void()>(20, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onCreateCylinder'
        QtMocHelpers::SlotData<void()>(21, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onCreateSphere'
        QtMocHelpers::SlotData<void()>(22, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onCreateCone'
        QtMocHelpers::SlotData<void()>(23, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onBooleanFuse'
        QtMocHelpers::SlotData<void()>(24, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onBooleanCut'
        QtMocHelpers::SlotData<void()>(25, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onBooleanCommon'
        QtMocHelpers::SlotData<void()>(26, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onNewSketch'
        QtMocHelpers::SlotData<void()>(27, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSketchOnFace'
        QtMocHelpers::SlotData<void()>(28, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onEditSketch'
        QtMocHelpers::SlotData<void()>(29, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onFinishSketch'
        QtMocHelpers::SlotData<void()>(30, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onFaceSelected'
        QtMocHelpers::SlotData<void()>(31, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSketchLine'
        QtMocHelpers::SlotData<void()>(32, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSketchRectangle'
        QtMocHelpers::SlotData<void()>(33, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSketchCircle'
        QtMocHelpers::SlotData<void()>(34, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSketchArc'
        QtMocHelpers::SlotData<void()>(35, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSketchPoint'
        QtMocHelpers::SlotData<void()>(36, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSketchSpline'
        QtMocHelpers::SlotData<void()>(37, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSketchEllipse'
        QtMocHelpers::SlotData<void()>(38, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSketchProject'
        QtMocHelpers::SlotData<void()>(39, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onConstraintHorizontal'
        QtMocHelpers::SlotData<void()>(40, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onConstraintVertical'
        QtMocHelpers::SlotData<void()>(41, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onConstraintCoincident'
        QtMocHelpers::SlotData<void()>(42, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onConstraintDistance'
        QtMocHelpers::SlotData<void()>(43, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onConstraintRadius'
        QtMocHelpers::SlotData<void()>(44, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onConstraintAngle'
        QtMocHelpers::SlotData<void()>(45, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onConstraintParallel'
        QtMocHelpers::SlotData<void()>(46, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onConstraintPerpendicular'
        QtMocHelpers::SlotData<void()>(47, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onReferencePlane'
        QtMocHelpers::SlotData<void()>(48, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onExtrude'
        QtMocHelpers::SlotData<void()>(49, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onCut'
        QtMocHelpers::SlotData<void()>(50, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onRevolve'
        QtMocHelpers::SlotData<void()>(51, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onFillet'
        QtMocHelpers::SlotData<void()>(52, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onChamfer'
        QtMocHelpers::SlotData<void()>(53, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onShell'
        QtMocHelpers::SlotData<void()>(54, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSweep'
        QtMocHelpers::SlotData<void()>(55, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onLoft'
        QtMocHelpers::SlotData<void()>(56, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onLoftSurface'
        QtMocHelpers::SlotData<void()>(57, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onPattern'
        QtMocHelpers::SlotData<void()>(58, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onMirror'
        QtMocHelpers::SlotData<void()>(59, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onScale'
        QtMocHelpers::SlotData<void()>(60, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onHoleWizard'
        QtMocHelpers::SlotData<void()>(61, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onDraft'
        QtMocHelpers::SlotData<void()>(62, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onRib'
        QtMocHelpers::SlotData<void()>(63, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onThicken'
        QtMocHelpers::SlotData<void()>(64, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onOffsetSurface'
        QtMocHelpers::SlotData<void()>(65, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSplit'
        QtMocHelpers::SlotData<void()>(66, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onDome'
        QtMocHelpers::SlotData<void()>(67, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onGear'
        QtMocHelpers::SlotData<void()>(68, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSketchSlot'
        QtMocHelpers::SlotData<void()>(69, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSketchPolygon'
        QtMocHelpers::SlotData<void()>(70, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onConvertEntities'
        QtMocHelpers::SlotData<void()>(71, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onIntersectionCurve'
        QtMocHelpers::SlotData<void()>(72, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSketchMirror'
        QtMocHelpers::SlotData<void()>(73, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSketchTrim'
        QtMocHelpers::SlotData<void()>(74, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSketchExtend'
        QtMocHelpers::SlotData<void()>(75, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSketchLinearPattern'
        QtMocHelpers::SlotData<void()>(76, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSketchCircularPattern'
        QtMocHelpers::SlotData<void()>(77, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onConstraintTangent'
        QtMocHelpers::SlotData<void()>(78, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onConstraintEqual'
        QtMocHelpers::SlotData<void()>(79, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onConstraintFix'
        QtMocHelpers::SlotData<void()>(80, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onConstraintConcentric'
        QtMocHelpers::SlotData<void()>(81, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onProfileSelected'
        QtMocHelpers::SlotData<void(int)>(82, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 83 },
        }}),
        // Slot 'onRingSelected'
        QtMocHelpers::SlotData<void(int, int)>(84, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 85 }, { QMetaType::Int, 86 },
        }}),
        // Slot 'onMultiProfilesConfirmed'
        QtMocHelpers::SlotData<void(const std::vector<std::pair<int,int>> &)>(87, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 88, 89 },
        }}),
        // Slot 'onProfileSelectionCancelled'
        QtMocHelpers::SlotData<void()>(90, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSelectShape'
        QtMocHelpers::SlotData<void()>(91, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSelectFace'
        QtMocHelpers::SlotData<void()>(92, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSelectEdge'
        QtMocHelpers::SlotData<void()>(93, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSelectVertex'
        QtMocHelpers::SlotData<void()>(94, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onAbout'
        QtMocHelpers::SlotData<void()>(95, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onToolApply'
        QtMocHelpers::SlotData<void()>(96, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onFeatureSelected'
        QtMocHelpers::SlotData<void(QListWidgetItem *)>(97, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 98, 99 },
        }}),
        // Slot 'onFeatureContextMenu'
        QtMocHelpers::SlotData<void(const QPoint &)>(100, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 101, 102 },
        }}),
        // Slot 'onToggleFeatureSuppression'
        QtMocHelpers::SlotData<void()>(103, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onFeatureReordered'
        QtMocHelpers::SlotData<void()>(104, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onAssemblyTreeSelection'
        QtMocHelpers::SlotData<void(std::shared_ptr<assembly::Component>)>(105, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 106, 107 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MainWindow, qt_meta_tag_ZN7opencad2ui10MainWindowE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject opencad::ui::MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7opencad2ui10MainWindowE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7opencad2ui10MainWindowE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7opencad2ui10MainWindowE_t>.metaTypes,
    nullptr
} };

void opencad::ui::MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MainWindow *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->onNewFile(); break;
        case 1: _t->onOpenFile(); break;
        case 2: _t->onSaveFile(); break;
        case 3: _t->onSaveAs(); break;
        case 4: _t->onExportSTL(); break;
        case 5: _t->onExportSTEP(); break;
        case 6: _t->onExit(); break;
        case 7: _t->onUndo(); break;
        case 8: _t->onRedo(); break;
        case 9: _t->onDelete(); break;
        case 10: _t->onViewFit(); break;
        case 11: _t->onViewFront(); break;
        case 12: _t->onViewBack(); break;
        case 13: _t->onViewTop(); break;
        case 14: _t->onViewBottom(); break;
        case 15: _t->onViewLeft(); break;
        case 16: _t->onViewRight(); break;
        case 17: _t->onViewIsometric(); break;
        case 18: _t->onCreateBox(); break;
        case 19: _t->onCreateCylinder(); break;
        case 20: _t->onCreateSphere(); break;
        case 21: _t->onCreateCone(); break;
        case 22: _t->onBooleanFuse(); break;
        case 23: _t->onBooleanCut(); break;
        case 24: _t->onBooleanCommon(); break;
        case 25: _t->onNewSketch(); break;
        case 26: _t->onSketchOnFace(); break;
        case 27: _t->onEditSketch(); break;
        case 28: _t->onFinishSketch(); break;
        case 29: _t->onFaceSelected(); break;
        case 30: _t->onSketchLine(); break;
        case 31: _t->onSketchRectangle(); break;
        case 32: _t->onSketchCircle(); break;
        case 33: _t->onSketchArc(); break;
        case 34: _t->onSketchPoint(); break;
        case 35: _t->onSketchSpline(); break;
        case 36: _t->onSketchEllipse(); break;
        case 37: _t->onSketchProject(); break;
        case 38: _t->onConstraintHorizontal(); break;
        case 39: _t->onConstraintVertical(); break;
        case 40: _t->onConstraintCoincident(); break;
        case 41: _t->onConstraintDistance(); break;
        case 42: _t->onConstraintRadius(); break;
        case 43: _t->onConstraintAngle(); break;
        case 44: _t->onConstraintParallel(); break;
        case 45: _t->onConstraintPerpendicular(); break;
        case 46: _t->onReferencePlane(); break;
        case 47: _t->onExtrude(); break;
        case 48: _t->onCut(); break;
        case 49: _t->onRevolve(); break;
        case 50: _t->onFillet(); break;
        case 51: _t->onChamfer(); break;
        case 52: _t->onShell(); break;
        case 53: _t->onSweep(); break;
        case 54: _t->onLoft(); break;
        case 55: _t->onLoftSurface(); break;
        case 56: _t->onPattern(); break;
        case 57: _t->onMirror(); break;
        case 58: _t->onScale(); break;
        case 59: _t->onHoleWizard(); break;
        case 60: _t->onDraft(); break;
        case 61: _t->onRib(); break;
        case 62: _t->onThicken(); break;
        case 63: _t->onOffsetSurface(); break;
        case 64: _t->onSplit(); break;
        case 65: _t->onDome(); break;
        case 66: _t->onGear(); break;
        case 67: _t->onSketchSlot(); break;
        case 68: _t->onSketchPolygon(); break;
        case 69: _t->onConvertEntities(); break;
        case 70: _t->onIntersectionCurve(); break;
        case 71: _t->onSketchMirror(); break;
        case 72: _t->onSketchTrim(); break;
        case 73: _t->onSketchExtend(); break;
        case 74: _t->onSketchLinearPattern(); break;
        case 75: _t->onSketchCircularPattern(); break;
        case 76: _t->onConstraintTangent(); break;
        case 77: _t->onConstraintEqual(); break;
        case 78: _t->onConstraintFix(); break;
        case 79: _t->onConstraintConcentric(); break;
        case 80: _t->onProfileSelected((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 81: _t->onRingSelected((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 82: _t->onMultiProfilesConfirmed((*reinterpret_cast<std::add_pointer_t<std::vector<std::pair<int,int>>>>(_a[1]))); break;
        case 83: _t->onProfileSelectionCancelled(); break;
        case 84: _t->onSelectShape(); break;
        case 85: _t->onSelectFace(); break;
        case 86: _t->onSelectEdge(); break;
        case 87: _t->onSelectVertex(); break;
        case 88: _t->onAbout(); break;
        case 89: _t->onToolApply(); break;
        case 90: _t->onFeatureSelected((*reinterpret_cast<std::add_pointer_t<QListWidgetItem*>>(_a[1]))); break;
        case 91: _t->onFeatureContextMenu((*reinterpret_cast<std::add_pointer_t<QPoint>>(_a[1]))); break;
        case 92: _t->onToggleFeatureSuppression(); break;
        case 93: _t->onFeatureReordered(); break;
        case 94: _t->onAssemblyTreeSelection((*reinterpret_cast<std::add_pointer_t<std::shared_ptr<assembly::Component>>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *opencad::ui::MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *opencad::ui::MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7opencad2ui10MainWindowE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int opencad::ui::MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 95)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 95;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 95)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 95;
    }
    return _id;
}
QT_WARNING_POP
