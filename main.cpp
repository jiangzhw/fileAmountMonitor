#include <QtGui/QApplication>
#include<QTextCodec>
#include<QTranslator>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qApp->addLibraryPath("./Plugins");
    QString libPath=QObject::tr("");
    //���������
    foreach(QString str,qApp->libraryPaths()){
        libPath.append(str);
        libPath.append("\n");
    }
    //ָ���ַ����룬��ֹ��������
    QTextCodec::setCodecForTr(QTextCodec::codecForLocale());

    QTranslator qtTranslator;
    QTranslator progTranslator;

    /**
     *����QT���򼰱�����ĵ����Ļ��ļ�
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
