#include "MainWindow.h"

#include <QApplication>
#include <QTimer>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("avm-retom"));
    app.setOrganizationName(QStringLiteral("AVM-Energo"));
    MainWindow w;
    w.show();
    // `--screenshot <file>`: grab the window to a PNG and quit (headless UI check, no device needed).
    const QStringList args = app.arguments();
    if (const int i = args.indexOf(QStringLiteral("--screenshot")); i >= 0 && i + 1 < args.size())
    {
        QTimer::singleShot(300, &app, [&w, &app, path = args.at(i + 1)] {
            w.grab().save(path);
            app.quit();
        });
    }
    return app.exec();
}
