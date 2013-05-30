#include <QtGui/QApplication>
#include<QTextCodec>
#include<QTranslator>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qApp->addLibraryPath("./Plugins");
    QString libPath=QObject::tr("");
    //添加依赖库
    foreach(QString str,qApp->libraryPaths()){
        libPath.append(str);
        libPath.append("\n");
    }
    //指定字符编码，防止中文乱码
    QTextCodec::setCodecForTr(QTextCodec::codecForLocale());

    QTranslator qtTranslator;
    QTranslator progTranslator;

    /**
     *加载QT程序及本程序的的中文化文件
     */
    qtTranslator.load(":/qm/qt_zh_CN.qm");
    progTranslator.load(":/qm/trans.qm");
    a.installTranslator(&qtTranslator);
    a.installTranslator(&progTranslator);
    MainWindow w;
    w.showMaximized();
    w.show();
    return a.exec();
}
