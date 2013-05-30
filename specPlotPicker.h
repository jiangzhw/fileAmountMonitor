#ifndef SPECPLOTPICKER_H
#define SPECPLOTPICKER_H
#include <qwt_plot_picker.h>
#include <qwt_text.h>
#include <QDateTime>

//自定义picker，绘制数据
class PlotPicker : public QwtPlotPicker
{
public:
    PlotPicker (int xAxis, int yAxis, RubberBand rubberBand, DisplayMode trackerMode, QwtPlotCanvas* canvas);
    void setDateTime(QDateTime dTime);
private:
    //对picker追踪的显示重定义,必须是重定义trackerTextF,不是trackerText
    QwtText trackerTextF (const QPointF  & pos) const;
    QDateTime dateTime;
};
#endif // SPECPLOTPICKER_H
