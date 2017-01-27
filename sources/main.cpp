#include <QtWidgets/qapplication.h>
#include <QtGui/qsurfaceformat.h>

#include "maingui.h"

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setVersion(4, 1);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setOption(QSurfaceFormat::DeprecatedFunctions, false);
    QSurfaceFormat::setDefaultFormat(format);

    MainGui gui;
    gui.show();

    gui.resize(1000, 600);
    return app.exec();
}