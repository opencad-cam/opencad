/**
 * @file main.cpp
 * @brief OpenCAD Application Entry Point
 *
 * OpenCAD - Modular CAD/CAE Platform
 * Version 0.1.0
 */

#include "ui/MainWindow.h"

#include <QApplication>
#include <QStyleFactory>
#include <QSurfaceFormat>

#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <iostream>

void messageOutput(QtMsgType type, const QMessageLogContext &context,
                   const QString &msg) {
  static QFile logFile("opencad_log.txt");
  if (!logFile.isOpen()) {
    logFile.open(QIODevice::WriteOnly | QIODevice::Append);
  }

  QTextStream out(&logFile);
  out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");

  switch (type) {
  case QtDebugMsg:
    out << "DEBUG: ";
    break;
  case QtInfoMsg:
    out << "INFO: ";
    break;
  case QtWarningMsg:
    out << "WARNING: ";
    break;
  case QtCriticalMsg:
    out << "CRITICAL: ";
    break;
  case QtFatalMsg:
    out << "FATAL: ";
    break;
  }

  out << msg << Qt::endl;
  out.flush();

  // Also print to stderr for good measure if we can see it
  std::cerr << msg.toStdString() << std::endl;
}

int main(int argc, char *argv[]) {
  qInstallMessageHandler(messageOutput);

  Q_INIT_RESOURCE(resources);

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
