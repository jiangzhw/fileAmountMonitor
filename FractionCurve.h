#ifndef FRACTIONCURVE_H
#define FRACTIONCURVE_H
#include <QSize>
#include <qwt_legend.h>
#include <qwt_plot_curve.h>
#include <qwt_legend_item.h>

//让图例的线段更明显些
class FractionCurve : public QwtPlotCurve

{

public:

    void updateLegend(QwtLegend *legend) const;

    FractionCurve(const QString);

};



FractionCurve::FractionCurve(const QString s) : QwtPlotCurve(s)

{

    ;

}


//让图例的线段更明显些
void FractionCurve::updateLegend(QwtLegend *legend) const

{

    QwtPlotCurve::updateLegend(legend);

    QwtLegendItem *lgdItem =dynamic_cast<QwtLegendItem*>(legend->find( this ));

    if ( lgdItem ){
       QSize size(10, 10);
         lgdItem->setIdentifierSize(size);
    }

     //   lgdItem->setIdentfierWidth(24);

}
#endif // FRACTIONCURVE_H
