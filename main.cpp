#include "dialog.h"
#include <QApplication>
#include <QProcess>




int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Dialog w;

    //重设窗口位置
    QDesktopWidget *desktop = QApplication::desktop();

    w.show();

    int e = a.exec();
    if(e == RETCODE_RESTART) {
        QProcess::startDetached(qApp->applicationFilePath(), QStringList());
        return 0;
    }
    return e;
}
