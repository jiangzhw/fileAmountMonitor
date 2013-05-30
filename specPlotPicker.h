#ifndef SPECPLOTPICKER_H
#define SPECPLOTPICKER_H
#include <qwt_plot_picker.h>
#include <qwt_text.h>
#include <QDateTime>

//�Զ���picker����������
class PlotPicker : public QwtPlotPicker
{
public:
    PlotPicker (int xAxis, int yAxis, RubberBand rubberBand, DisplayMode trackerMode, QwtPlotCanvas* canvas);
    void setDateTime(QDateTime dTime);
private:
    //��picker׷�ٵ���ʾ�ض���,�������ض���trackerTextF,����trackerText
    QwtText trackerTextF (const QPointF  & pos) const;
    QDateTime dateTime;
};
#endif // SPECPLOTPICKER_H
