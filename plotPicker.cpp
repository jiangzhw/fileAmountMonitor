#include "specPlotPicker.h"

PlotPicker ::PlotPicker (int xAxis, int yAxis, RubberBand rubberBand, DisplayMode trackerMode, QwtPlotCanvas* canvas):
        QwtPlotPicker(xAxis, yAxis, rubberBand, trackerMode, canvas)  {
}

//对picker追踪的显示自定义,必须是重定义trackerTextF,不是trackerText
QwtText PlotPicker::trackerTextF (const  QPointF & pos) const
{

    QDateTime occurTime = dateTime.addSecs(pos.x());
    QString occurTimeStr= occurTime.toString("yyyy-MM-dd hh:mm");
    QString fileAmountStr=QString::number((int)pos.y());

    QString info=tr("(");
    info.append(occurTimeStr);
    info.append(tr(","));
    info.append(tr("文件数:"));
    info.append(fileAmountStr);
    info.append(tr(")"));
    QwtText text(info);

    QColor bgColor(Qt::black);
    //An alpha setting of 255 is completely opaque, and an alpha of 0 is completely transparent
    bgColor.setAlpha(120);
    text.setBackgroundBrush(QBrush(bgColor));


    return text;
}

void PlotPicker::setDateTime(QDateTime dTime){
    dateTime=dTime;
}
