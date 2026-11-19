#include "MainWindow.h"

#include <QApplication>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("avm-retom"));
    app.setOrganizationName(QStringLiteral("AVM-Energo"));
    MainWindow w;
    w.show();
    return app.exec();
}
