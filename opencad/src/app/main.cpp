/**
 * @file main.cpp
 * @brief OpenCAD Application Entry Point
 * 
 * OpenCAD - Modular CAD/CAE Platform
 * Version 0.1.0
 */

#include "ui/MainWindow.h"

#include <QApplication>
#include <QSurfaceFormat>
#include <QStyleFactory>

int main(int argc, char* argv[]) {
    // Set OpenGL format before creating application
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSamples(4);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    // Create application
    QApplication app(argc, argv);
    
    // Set application info
    app.setApplicationName("OpenCAD");
    app.setApplicationVersion("0.1.0");
    app.setOrganizationName("OpenCAD Team");
    app.setOrganizationDomain("opencad.org");

    // Set modern style
    app.setStyle(QStyleFactory::create("Fusion"));

    // Apply dark theme
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    app.setPalette(darkPalette);

    // Create and show main window
    opencad::ui::MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
