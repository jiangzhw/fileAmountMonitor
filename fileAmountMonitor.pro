# -------------------------------------------------
# Project created by QtCreator 2011-10-27T15:46:36
# -------------------------------------------------
TARGET = fileAmountMonitor
TEMPLATE = app
CODECFORTR = GBK
INCLUDEPATH += "D:\qt\4.6-msvc2005\include\Qwt"
SOURCES += main.cpp \
    mainwindow.cpp \
    plotPicker.cpp
HEADERS += mainwindow.h \
    pixmaps.h \
    FractionCurve.h \
    specPlotPicker.h
FORMS += mainwindow.ui

# 下边两句要同时加
LIBS += -lqwt
DEFINES += QT_DLL \
    QWT_DLL
RESOURCES += res.qrc
RC_FILE += description.rc
TRANSLATIONS += trans.ts
