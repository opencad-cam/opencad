#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  opencad::ui::MainWindow *w = new opencad::ui::MainWindow();
  delete w;
  return 0;
}
