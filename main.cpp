#include "RealTime3DBuildGUI.h"

#include <QApplication>
#include <QCoreApplication>
#include <QSplashScreen>
#include <QDesktopWidget>
#include <QThread>
#include "Version.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("RealTime3DBuild");
    QCoreApplication::setApplicationName("RealTime3DBuild");
    QCoreApplication::setApplicationVersion(GetPointLabVersion());

    QPixmap pixmap(QString::fromUtf8(":/Images/startimg.png"));
//    int bestWidth = 700.0 / 3840.0*QApplication::desktop()->screenGeometry().width();
//    QPixmap startImg = pixmap.scaledToWidth(bestWidth, Qt::SmoothTransformation);

    QSplashScreen* splash = new QSplashScreen(pixmap, Qt::WindowStaysOnTopHint);
    splash->show();

    QApplication::processEvents();
//    QThread::sleep(0.5);

    RealTime3DBuildGUI w;
    w.show();
    splash->close();
    return a.exec();
}
