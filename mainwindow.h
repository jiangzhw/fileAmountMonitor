#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QFile>
#include <QDateTime>
#include <QSystemTrayIcon>


#include <qwt_text.h>
#include <qwt_symbol.h>
#include <qwt_plot.h>
#include <qwt_plot_dict.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_item.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_canvas.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_widget.h>
#include <qwt_legend.h>
#include <qwt_legend_item.h>
#include <qwt_legend_itemmanager.h>
#include <qwt_picker_machine.h>

#include "specPlotPicker.h"


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();




public slots:

    //绘制数据
    void plotCurve();

    void moved(const QPoint &);
    void selected(const QPolygon  &);
    void showItem(QwtPlotItem *, bool on);

    void showInfo(QString text = QString::null);
    void print();
    void exportDocument();
    void showHelp();
    //系统托盘被点击对应的槽函数
    void onSystemTrayIconClicked(QSystemTrayIcon::ActivationReason);

protected:
    void changeEvent(QEvent *e);
    void readData();
    bool inRedCurve(const QPoint &  pos );
    bool inGreenCurve(const QPoint &  pos );
    bool inBlueCurve(const QPoint &  pos );




private:
    Ui::MainWindow *ui;
  //  QwtPlotZoomer *zoomer;

    PlotPicker *plotPicker;
    QwtPlotCurve *redPlotCurve;
    QwtPlotCurve *greenPlotCurve;
    QwtPlotCurve *bluePlotCurve;
    QwtPlotPanner *PQ;
    QDateTime curTime;

    //由于是过去的时间，假定一个最早的日期六年内的分钟数
    double xMinScale;
    int yMaxScale;

    QVector< double > redXData;
    QVector< double > redYData;
    QVector< double > greenXData;
    QVector< double > greenYData;
    QVector< double > blueXData;
    QVector< double > blueYData;

    QFile *localFile;
    QString m_filename;

};

#endif // MAINWINDOW_H
