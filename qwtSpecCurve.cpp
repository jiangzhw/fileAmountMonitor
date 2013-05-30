#include <QPolygonF>
#include <QVector>
#include <qwt_painter.h>
#include <qwt_symbol.h>
#include "qwt_plot_spectrocurve.h"
#include "qwt_color_map.h"
#include "qwt_scale_map.h"

#include "qwtSpecCurve.h"

/*
class QwtSpecCurve::PrivateData
{
public:
    PrivateData():
        colorRange( 0.0, 1000.0 ),
        paintAttributes( QwtSpecCurve::ClipPoints )
    {
        colorMap = new QwtLinearColorMap();
    }

    ~PrivateData()
    {
        delete colorMap;
    }

    QwtColorMap *colorMap;
    QwtInterval colorRange;
    QVector<QRgb> colorTable;
    int paintAttributes;
};
*/
QwtSpecCurve::QwtSpecCurve( const QwtText &title ):
        QwtPlotSeriesItem<QwtPoint3D>( title )
{
    init();
}

/*!
  Constructor
  \param title Title of the curve
*/
QwtSpecCurve::QwtSpecCurve( const QString &title ):
        QwtPlotSeriesItem<QwtPoint3D>( QwtText( title ) )
{
    init();
}

//! Destructor
QwtSpecCurve::~QwtSpecCurve()
{
    delete d_data;
}


void QwtSpecCurve::drawSeries( QPainter *painter,
                               const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                               const QRectF &canvasRect, int from, int to ) const
{
    drawCurve( xMap, yMap, canvasRect, from, to );
    drawSymbols( xMap, yMap, canvasRect, from, to );
}

void QwtSpecCurve::drawLines( QPainter *painter,
                              const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                              const QRectF &canvasRect, int from, int to ) const
{
    int size = to - from + 1;
    if ( size <= 0 )
        return;

    QPolygonF polyline( size );

    QPointF *points = polyline.data();
    for ( int i = from; i <= to; i++ )
    {
        const QwtPoint3D sample = m_series->sample( i );

        points[i - from].rx() = xMap.transform( sample.x() );
        points[i - from].ry() = yMap.transform( sample.y() );
    }

    // add polygon clipping if you want to zoom in deep
    // ...

    QwtPainter::drawPolyline( painter, polyline );
}

void QwtSpecCurve::drawSymbols( QPainter *painter,
                                const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                                const QRectF &canvasRect, int from, int to ) const
{
    //QwtSymbol (Style, const QBrush &, const QPen &, const QSize &)
    QwtSymbol symbol( ... );

    for ( int i = from; i <= to; i++ ) {
        const QwtPoint3D sample = m_series->sample( i );
        symbol.setBrush( color( sample.z() ) );

        const QPointF pos( xMap.transform( sample.x() );
        yMap.transform( sample.y() );

        symbol.drawSymbols( painter, &pos, 1 );
    }

}


//! \return QwtPlotItem::Rtti_PlotCurve
int QwtSpecCurve::rtti() const
{
    return QwtPlotItem::Rtti_PlotSpectroCurve;
}

/*!
  Initialize data with an array of samples.
  \param samples Vector of points
*/
void QwtSpecCurve::setSamples( const QVector<QwtPoint3D> &samples )
{
    delete d_series;
    d_series = new QwtPoint3DSeriesData( samples );
    itemChanged();
}
