#ifndef QWTSPECCURVE_H
#define QWTSPECCURVE_H
#include <qwt_plot_seriesitem.h>
#include <qwt_point_3d.h>
#include <qwt_scale_map.h>
#include <QRectF>
#include "qwt_global.h"
#include "qwt_series_data.h"

class QwtSpecCurve: public QwtPlotSeriesItem<QwtPoint3D>
{
public:
    enum PaintAttribute
    {
        ClipPoints = 1
    };

    explicit QwtSpecCurve( const QString &title = QString::null );
    explicit QwtSpecCurve( const QwtText &title );

    virtual ~QwtSpecCurve();

    virtual int rtti() const;

    void setSamples( const QVector<QwtPoint3D> & );

    virtual void drawSeries( QPainter *,
                             const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                             const QRectF &canvasRect, int from, int to ) const;

protected:
    virtual void drawSymbols( QPainter *,
                              const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                              const QRectF &canvasRect, int from, int to ) const;

    virtual void drawLines( QPainter *,
                            const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                            const QRectF &canvasRect, int from, int to ) const;
private:
   // void init();
  //  class PrivateData;
  //  PrivateData *d_data;
};
#endif // QWTSPECCURVE_H
