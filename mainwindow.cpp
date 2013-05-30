#include <QDateTime>
#include <QToolBar>
#include <QToolButton>
#include <QHBoxLayout>
#include <QPrinter>
#include <QPrintDialog>
#include <QFileDialog>
#include <QImageWriter>
#include <QPen>
#include <QAction>
#include <QWhatsThis>
#include <QMessageBox>
#include <QSystemTrayIcon>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "pixmaps.h"
#include "FractionCurve.h"



//�Զ���ʱ����
class TimeScaleDraw: public QwtScaleDraw
{
public:
    TimeScaleDraw(const QDateTime &base):
            baseTime(base)
    {

    }

    virtual QwtText label(double v) const
    {
        QDateTime occurTime = baseTime.addSecs((int)v);
        return occurTime.toString("yyyy-MM-dd hh:mm");
    }

private:
    QDateTime baseTime;
};

//������
class Zoomer: public QwtPlotZoomer
{
public:
    Zoomer(int xAxis, int yAxis, QwtPlotCanvas *canvas):
            QwtPlotZoomer(xAxis, yAxis, canvas)
    {
        setTrackerMode(QwtPicker::AlwaysOff);
        setRubberBand(QwtPicker::NoRubberBand);
        setMousePattern(QwtEventPattern::MouseSelect2,
                        Qt::RightButton, Qt::ControlModifier);
        setMousePattern(QwtEventPattern::MouseSelect3,
                        Qt::RightButton);
    }
};


MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QFile cssFile(":/css/css.qss");
    cssFile.open(QFile::ReadOnly);
    QString styleSheet=tr(cssFile.readAll());
    qApp->setStyleSheet(styleSheet);

    QIcon systemIcon = QIcon(":/images/curve.png");
    QSystemTrayIcon *trayIcon=new QSystemTrayIcon(this);
    trayIcon->setToolTip(tr("�ļ��������޸ġ�����ʱ��ͳ��"));
    trayIcon->setIcon(systemIcon);
    trayIcon->show();
    QMenu *trayMenu=new QMenu(this);
    QAction *quitAction=new QAction(tr("�˳�"),this);
    QAction *showAction = new QAction(tr("��ʾ������"), this);
    QAction *hideAction = new QAction(tr("����������"), this);
    trayMenu->addAction(showAction);
    trayMenu->addAction(hideAction);
    trayMenu->addAction(quitAction);
    trayIcon->setContextMenu(trayMenu);
    connect(trayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,
            SLOT(onSystemTrayIconClicked(QSystemTrayIcon::ActivationReason)));
    connect(quitAction,SIGNAL(triggered()),this,SLOT(close()));
    connect(showAction,SIGNAL(triggered()),this,SLOT(show()));
    connect(hideAction,SIGNAL(triggered()),this,SLOT(hide()));


    QToolBar *toolBar = new QToolBar(this);


#ifndef QT_NO_PRINTER
    QToolButton *btnPrint = new QToolButton(toolBar);
    btnPrint->setVisible(false);
    btnPrint->setText(tr("��ӡ"));
    btnPrint->setIcon(QIcon(print_xpm));
    btnPrint->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->addWidget(btnPrint);
    connect(btnPrint, SIGNAL(clicked()), SLOT(print()));
#endif

    QToolButton *btnExport = new QToolButton(toolBar);
    btnExport->setText(tr("����"));
    btnExport->setIcon(QIcon(print_xpm));
    btnExport->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->addWidget(btnExport);
    connect(btnExport, SIGNAL(clicked()), SLOT(exportDocument()));

    QToolButton *btnHelp = new QToolButton(toolBar);
    btnHelp->setText(tr("����"));
    QIcon icon = QIcon(":/images/help.png");
    btnHelp->setIcon(QIcon(icon));
    btnHelp->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->addWidget(btnHelp);
    setIconSize(QSize(22, 22));

    connect(btnHelp, SIGNAL(clicked()), SLOT(showHelp()));

    QWidget *hBox = new QWidget(toolBar);
    QHBoxLayout *layout = new QHBoxLayout(hBox);
    layout->setSpacing(0);
    layout->addWidget(new QWidget(hBox), 10); // spacer
    (void)toolBar->addWidget(hBox);
    addToolBar(toolBar);



    ui->qwtPlot->setTitle(tr("�ļ��������޸ġ�����ʱ��ͳ��"));
    ui->qwtPlot->setAutoReplot(true);

    //�����ַŴ���С��
    QwtPlotMagnifier *PM = new QwtPlotMagnifier( ui->qwtPlot->canvas() );
    PM->setAxisEnabled(QwtPlot::yLeft,true);
    PM->setAxisEnabled( QwtPlot::yRight, false);
    PM->setAxisEnabled(QwtPlot::xBottom,true);
    PM->setAxisEnabled( QwtPlot::xTop, false);

    //��������м�(����)�϶�����
    PQ= new QwtPlotPanner( ui->qwtPlot->canvas() );
    PQ->setAxisEnabled(QwtPlot::yLeft,true);
    PQ->setAxisEnabled( QwtPlot::yRight, false);
    PQ->setAxisEnabled(QwtPlot::xBottom,true);
    PQ->setAxisEnabled( QwtPlot::xTop, false);
    PQ->setMouseButton(Qt::MidButton);
    PQ->setEnabled(true);


    //����ͼ�εĿ�ʼʱ�䣬ȫ�ֱ�������plotPicker��Ҫ�õ�
    curTime=QDateTime(QDateTime::currentDateTime());
    plotPicker= new PlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,QwtPlotPicker::CrossRubberBand, QwtPicker::ActiveOnly, ui->qwtPlot->canvas());
    plotPicker->setDateTime(curTime);

    plotPicker->setStateMachine(new QwtPickerDragPointMachine());
    plotPicker->setRubberBandPen(QColor(Qt::black));
    plotPicker->setRubberBand(QwtPicker::CrossRubberBand);
    plotPicker->setTrackerPen(QColor(Qt::white));

    connect(plotPicker, SIGNAL(moved(const QPoint &)), SLOT(moved(const QPoint &)));
    connect(plotPicker, SIGNAL(selected(const QPolygon  &)), SLOT(selected(const QPolygon &)));


    //�����ڵķ�����
    xMinScale=-525600;
    // xMinScale=2190;
    yMaxScale=500;


    ui->qwtPlot->setAxisTitle(QwtPlot::xBottom,tr("ʱ�� -->"));
    ui->qwtPlot->setAxisScale(QwtPlot::xBottom,xMinScale,1);
    ui->qwtPlot->setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw(curTime));

    ui->qwtPlot->setAxisTitle(QwtPlot::yLeft, tr("�ļ����� -->"));
    ui->qwtPlot->setAxisScale(QwtPlot::yLeft, 0, yMaxScale,1);


    /// ui->qwtPlot->setCanvasBackground(QColor(Qt::gray));
    ui->qwtPlot->plotLayout()->setAlignCanvasToScales(true);

    //��������
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableX(true);
    grid->enableY(true);
    grid->enableXMin(false);
    grid->enableYMin(false);
    grid->setMajPen(QPen(Qt::black, 0, Qt::DotLine));
    grid->attach(ui->qwtPlot);

    redPlotCurve= new FractionCurve(tr("�������ļ�����"));
    //���ֱ�ʾ�ߴ�ϸ
    redPlotCurve->setPen(QPen(Qt::red,2));

    //��ԭʼ���ݵĵ�ͨ��setSymbolͻ����ʾ��brush�൱��һ��Բ���ڲ���pen�൱��Բ����ɫ
    redPlotCurve->setLegendAttribute(QwtPlotCurve::LegendShowLine);
    redPlotCurve->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, Qt::red,QPen(Qt::black), QSize(6, 6) ) );
    //ʵ������ƽ��,ȥ�����
    redPlotCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    redPlotCurve->attach(ui->qwtPlot);

    greenPlotCurve= new FractionCurve(tr("�޸ĵ��ļ�����"));
    greenPlotCurve->setPen(QPen(Qt::green,2));
    greenPlotCurve->setLegendAttribute(QwtPlotCurve::LegendShowLine);
    greenPlotCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    greenPlotCurve->attach(ui->qwtPlot);
    greenPlotCurve->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, Qt::green, QPen(Qt::black), QSize(6, 6) ) );


    bluePlotCurve= new FractionCurve(tr("�����ʵ��ļ�����"));
    bluePlotCurve->setPen(QPen(Qt::blue,2));
    bluePlotCurve->setLegendAttribute(QwtPlotCurve::LegendShowLine);
    bluePlotCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    bluePlotCurve->attach(ui->qwtPlot);
    bluePlotCurve->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, Qt::blue,QPen(Qt::black), QSize(6, 6) ) );

    //ͼʾ
    QwtLegend *legend = new QwtLegend;
    //�ɸ�ѡ
    legend->setItemMode(QwtLegend::CheckableItem);
    ui->qwtPlot->insertLegend(legend,QwtPlot::RightLegend);

    QwtPlotItemList items =   ui->qwtPlot->itemList(QwtPlotItem::Rtti_PlotCurve);
    for ( int i = 0; i < items.size(); i++ )
    {
        QwtLegendItem *legendItem = (QwtLegendItem *)legend->find(items[i]);
        if ( legendItem ){
            legendItem->setChecked(true);
            items[i]->setVisible(true);
        }

    }
    connect(ui->qwtPlot, SIGNAL(legendChecked(QwtPlotItem *, bool)), SLOT(showItem(QwtPlotItem *, bool)));

    this->plotCurve();
    ui->qwtPlot->replot();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showInfo(QString text)
{
    if ( text == QString::null )
    {
        if ( plotPicker->rubberBand() )

            text = tr("��궨λ: ѡ������һ�㲢�������");
        else
            text = tr("����: ����������");

    }

#ifndef QT_NO_STATUSBAR
    statusBar()->showMessage(text);
#endif
}

void MainWindow::moved(const QPoint &pos)
{
    QString info;
    QDateTime occurTime = curTime.addSecs(ui->qwtPlot->invTransform(QwtPlot::xBottom,pos.x()));
    int fileAmount=ui->qwtPlot->invTransform(QwtPlot::yLeft,pos.y());
    QString occutTimeStr= occurTime.toString("yyyy-MM-dd hh:mm");

    info.append(tr("ʱ�䣺"));
    info.append(occutTimeStr);
    info.append(tr("  "));
    info.append(tr("�ļ���(����)��"));
    info.append(QString::number(fileAmount));
    showInfo(info);
}

bool MainWindow::inRedCurve(const QPoint &pos){
    int seconds=pos.x();
    int yValue=pos.y();
    bool in=false;
    double occurTime = ui->qwtPlot->invTransform(QwtPlot::xBottom,seconds);
    double fileAmount=ui->qwtPlot->invTransform(QwtPlot::yLeft,yValue);
    int index=redXData.indexOf(occurTime);
    if(index ==-1){
        in=false;
    }else{
        if(redYData.at(index)==fileAmount){
            in=true;
        }
    }
    return in;
}

bool MainWindow::inGreenCurve(const QPoint &pos){
    int seconds=pos.x();
    int yValue=pos.y();
    bool in=false;
    double occurTime = ui->qwtPlot->invTransform(QwtPlot::xBottom,seconds);
    double fileAmount=ui->qwtPlot->invTransform(QwtPlot::yLeft,yValue);
    int index=greenXData.indexOf(occurTime);
    if(index ==-1){
        in=false;
    }else{
        if(greenYData.at(index)==fileAmount){
            in=true;
        }
    }
    return in;
}
bool MainWindow::inBlueCurve(const QPoint &pos){
    int seconds=pos.x();
    int yValue=pos.y();
    bool in=false;
    double occurTime = ui->qwtPlot->invTransform(QwtPlot::xBottom,seconds);
    double fileAmount=ui->qwtPlot->invTransform(QwtPlot::yLeft,yValue);
    int index=blueXData.indexOf(occurTime);
    if(index ==-1){
        in=false;
    }else{
        if(blueYData.at(index)==fileAmount){
            in=true;
        }
    }
    return in;
}
//��ʾѡ�еĵ��ʱ�䡢����
//���ֻ����ʾ�������ϵ�ֵ������ѡ�е�ֵ��qvector���бȽϼ���
//��̫����
void MainWindow::selected(const QPolygon &  pos )
{
    QString info;
    QString typeStr=tr("");
    bool findPoint=false;
    if(pos.size()>=1){
        if(this->inRedCurve(pos.point(0))){
            typeStr.append(tr("�����ļ��� "));
            findPoint=true;
        }
        if(this->inGreenCurve(pos.point(0))){
            typeStr.append(tr("�޸��ļ��� "));
            findPoint=true;
        }
        if(this->inBlueCurve(pos.point(0))){
            typeStr.append(tr("�������ļ��� "));
            findPoint=true;
        }
        if(findPoint){
            typeStr.append(tr("��"));
            int seconds=pos.point(0).x();
            int yValue=pos.point(0).y();
            QDateTime occurTime = curTime.addSecs(ui->qwtPlot->invTransform(QwtPlot::xBottom,seconds));
            int fileAmount=ui->qwtPlot->invTransform(QwtPlot::yLeft,yValue);
            QString occutTimeStr= occurTime.toString("yyyy-MM-dd hh:mm");
            info.append(tr("ʱ�䣺"));
            info.append(occutTimeStr);
            info.append(tr("  "));
            info.append(typeStr);
            info.append(QString::number(fileAmount));
            showInfo(info);
        }


    }
    else{
        showInfo();
        return ;
    }
}

//��ȡɨ���ļ��������д�� QVector< double > &
void MainWindow::readData(){
    /*Ĭ���ļ�ɨ��������fileAmountAlteration.txt��
    *��ʽ���ӣ�
     *2011-10-27 21:39,800,735,600
     *�ֱ���ʱ�䣬�����ļ������޸��ļ������������ļ���
      */
    QString fileName=tr("fileAmountAlteration.txt");
    QFile file(fileName);
    QStringList tempQSL;

    QString tempStr=tr("");
    if (file.open(QFile::ReadOnly))
    {
        QTextStream floStream(&file);
        while ( floStream.atEnd()==0 )
        {
            tempStr=floStream.readLine();
            tempStr.trimmed();
            tempQSL<<tempStr;

        }
        file.close();
    }
    else{
        QMessageBox::warning(this, tr("������ļ��򿪴���!"),
                             tr("���ܶ��ļ� %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
    }
    //����������
    tempQSL.sort();

    if(tempQSL.size()==0){
        return;
    }
    redXData.clear();
    redYData.clear();
    greenXData.clear();
    greenYData.clear();
    blueXData.clear();
    blueYData.clear();

    QStringList oneRecord;
    double secondsTo=0;
    double fileAmount=0;
    QString tempDateStr=tr("");

    foreach(QString str,tempQSL){
        oneRecord=str.split(",");
        if(oneRecord.size()>=4){
            tempDateStr=oneRecord.at(0);
            QDateTime occurTime=QDateTime::fromString(tempDateStr,"yyyy-MM-dd hh:mm");
            secondsTo=curTime.secsTo(occurTime);
            redXData.append(secondsTo);
            fileAmount=QString(oneRecord.at(1)).toDouble();
            redYData.append(fileAmount);

            greenXData.append(secondsTo);
            fileAmount=QString(oneRecord.at(2)).toDouble();
            greenYData.append(fileAmount);

            blueXData.append(secondsTo);
            fileAmount=QString(oneRecord.at(3)).toDouble();
            blueYData.append(fileAmount);
        }
    }
}

void MainWindow::plotCurve(){

    this->readData();
    redPlotCurve->setSamples(redXData,redYData);
    greenPlotCurve->setSamples(greenXData,greenYData);
    bluePlotCurve->setSamples(blueXData,blueYData);
    if (!redXData.empty())
    {
        //����X��,�����ǹ�ȥ��ʱ�䣬����������
        if (redXData.last() <= xMinScale)
        {
            xMinScale *= 2;
            ui->qwtPlot->setAxisScale(QwtPlot::xBottom,xMinScale,0);
        }
    }
    //����Y��
    if(!redYData.empty()){
        if(redYData.last() >= yMaxScale ||  greenYData.last()>=yMaxScale || blueYData.last()>=yMaxScale){
            yMaxScale *=2;
            ui->qwtPlot->setAxisScale(QwtPlot::yLeft, 0,yMaxScale);
        }
    }

    ui->qwtPlot->replot();

}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

//ֻ��ʾѡ�������
void MainWindow::showItem(QwtPlotItem *item, bool on){
    item->setVisible(on);
}


void MainWindow::print()
{
    QPrinter printer(QPrinter::HighResolution);

    QString docName = ui->qwtPlot->title().text();
    if ( !docName.isEmpty() )
    {
        docName.replace (QRegExp (QString::fromLatin1 ("\n")), tr (" -- "));
        printer.setDocName (docName);
    }

    printer.setCreator(tr("�ļ�ʱ����Ϣͳ��"));
    printer.setOrientation(QPrinter::Landscape);

    QPrintDialog dialog(&printer);
    if ( dialog.exec() )
    {
        QwtPlotRenderer renderer;

        if ( printer.colorMode() == QPrinter::GrayScale )
        {
            renderer.setDiscardFlag(QwtPlotRenderer::DiscardCanvasBackground);
            renderer.setLayoutFlag(QwtPlotRenderer::FrameWithScales);
        }

        renderer.renderTo(ui->qwtPlot, printer);
    }
}



void MainWindow::exportDocument()
{

    QString fileName = "fileAmountResult";

#ifndef QT_NO_FILEDIALOG
    const QList<QByteArray> imageFormats =
            QImageWriter::supportedImageFormats();

    QStringList filter;
    filter += tr("PDF �ļ� (*.pdf)");
#ifndef QWT_NO_SVG
    filter += tr("SVG �ļ� (*.svg)");
#endif
    filter += tr("Postscript �ļ� (*.ps)");

    if ( imageFormats.size() > 0 )
    {
        QString imageFilter("Images (");
        for ( int i = 0; i < imageFormats.size(); i++ )
        {
            if ( i > 0 )
                imageFilter += " ";
            imageFilter += "*.";
            imageFilter += imageFormats[i];
        }
        imageFilter += ")";

        filter += imageFilter;
    }

    fileName = QFileDialog::getSaveFileName(
            this, tr("�����ļ�"), fileName,
            filter.join(";;"), NULL, QFileDialog::DontConfirmOverwrite);
#endif

    if ( !fileName.isEmpty() )
    {
        QwtPlotRenderer renderer;
#if 0
        // ���ֵ������ļ�����������һ��
        renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground, false);
        renderer.setLayoutFlag(QwtPlotRenderer::KeepMargins, true);
        renderer.setLayoutFlag(QwtPlotRenderer::KeepFrames, true);
#endif
        renderer.renderDocument(ui->qwtPlot, fileName, QSizeF(300, 200), 85);
    }
}


void MainWindow::showHelp(){
    QString helpInfo=tr("");
    helpInfo.append(tr("1.ѡ���ȡ���ɸ�ѡ��ͼ����ѡ����ʾ������\n"));
    helpInfo.append(tr("2.������������ѡ�������е�һ�㣬�ɲ鿴�õ��ʱ�����ļ�������Ϣ\n"));
    helpInfo.append(tr("3.���������֣��ɶ�ͼ�ν�����С��Ŵ�\n"));
    helpInfo.append(tr("4.��������м�(����)���ɶ�ͼ�ν����϶�\n"));
    helpInfo.append(tr("5.������ֻ����Բ��ͻ�������ݲų���ԭʼ�ļ������"));
    QMessageBox::information (NULL,QObject::tr("����"),helpInfo);

}


void MainWindow::onSystemTrayIconClicked(QSystemTrayIcon::ActivationReason reason){
    if(reason==QSystemTrayIcon::Trigger){
        if(isHidden())
            show();
        else
            hide();
    }
}
