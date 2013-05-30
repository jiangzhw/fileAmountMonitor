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



//自定义时间轴
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

//缩放器
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
    trayIcon->setToolTip(tr("文件创建、修改、访问时间统计"));
    trayIcon->setIcon(systemIcon);
    trayIcon->show();
    QMenu *trayMenu=new QMenu(this);
    QAction *quitAction=new QAction(tr("退出"),this);
    QAction *showAction = new QAction(tr("显示主窗口"), this);
    QAction *hideAction = new QAction(tr("隐藏主窗口"), this);
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
    btnPrint->setText(tr("打印"));
    btnPrint->setIcon(QIcon(print_xpm));
    btnPrint->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->addWidget(btnPrint);
    connect(btnPrint, SIGNAL(clicked()), SLOT(print()));
#endif

    QToolButton *btnExport = new QToolButton(toolBar);
    btnExport->setText(tr("导出"));
    btnExport->setIcon(QIcon(print_xpm));
    btnExport->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->addWidget(btnExport);
    connect(btnExport, SIGNAL(clicked()), SLOT(exportDocument()));

    QToolButton *btnHelp = new QToolButton(toolBar);
    btnHelp->setText(tr("帮助"));
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



    ui->qwtPlot->setTitle(tr("文件创建、修改、访问时间统计"));
    ui->qwtPlot->setAutoReplot(true);

    //鼠标滚轮放大缩小：
    QwtPlotMagnifier *PM = new QwtPlotMagnifier( ui->qwtPlot->canvas() );
    PM->setAxisEnabled(QwtPlot::yLeft,true);
    PM->setAxisEnabled( QwtPlot::yRight, false);
    PM->setAxisEnabled(QwtPlot::xBottom,true);
    PM->setAxisEnabled( QwtPlot::xTop, false);

    //按下鼠标中键(滚轮)拖动曲线
    PQ= new QwtPlotPanner( ui->qwtPlot->canvas() );
    PQ->setAxisEnabled(QwtPlot::yLeft,true);
    PQ->setAxisEnabled( QwtPlot::yRight, false);
    PQ->setAxisEnabled(QwtPlot::xBottom,true);
    PQ->setAxisEnabled( QwtPlot::xTop, false);
    PQ->setMouseButton(Qt::MidButton);
    PQ->setEnabled(true);


    //绘制图形的开始时间，全局变量，在plotPicker中要用到
    curTime=QDateTime(QDateTime::currentDateTime());
    plotPicker= new PlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,QwtPlotPicker::CrossRubberBand, QwtPicker::ActiveOnly, ui->qwtPlot->canvas());
    plotPicker->setDateTime(curTime);

    plotPicker->setStateMachine(new QwtPickerDragPointMachine());
    plotPicker->setRubberBandPen(QColor(Qt::black));
    plotPicker->setRubberBand(QwtPicker::CrossRubberBand);
    plotPicker->setTrackerPen(QColor(Qt::white));

    connect(plotPicker, SIGNAL(moved(const QPoint &)), SLOT(moved(const QPoint &)));
    connect(plotPicker, SIGNAL(selected(const QPolygon  &)), SLOT(selected(const QPolygon &)));


    //六年内的分钟数
    xMinScale=-525600;
    // xMinScale=2190;
    yMaxScale=500;


    ui->qwtPlot->setAxisTitle(QwtPlot::xBottom,tr("时间 -->"));
    ui->qwtPlot->setAxisScale(QwtPlot::xBottom,xMinScale,1);
    ui->qwtPlot->setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw(curTime));

    ui->qwtPlot->setAxisTitle(QwtPlot::yLeft, tr("文件数量 -->"));
    ui->qwtPlot->setAxisScale(QwtPlot::yLeft, 0, yMaxScale,1);


    /// ui->qwtPlot->setCanvasBackground(QColor(Qt::gray));
    ui->qwtPlot->plotLayout()->setAlignCanvasToScales(true);

    //增加网格
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableX(true);
    grid->enableY(true);
    grid->enableXMin(false);
    grid->enableYMin(false);
    grid->setMajPen(QPen(Qt::black, 0, Qt::DotLine));
    grid->attach(ui->qwtPlot);

    redPlotCurve= new FractionCurve(tr("创建的文件数量"));
    //数字表示线粗细
    redPlotCurve->setPen(QPen(Qt::red,2));

    //对原始数据的点通过setSymbol突出表示，brush相当于一个圆面内部，pen相当于圆周着色
    redPlotCurve->setLegendAttribute(QwtPlotCurve::LegendShowLine);
    redPlotCurve->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, Qt::red,QPen(Qt::black), QSize(6, 6) ) );
    //实现曲线平滑,去除锯齿
    redPlotCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    redPlotCurve->attach(ui->qwtPlot);

    greenPlotCurve= new FractionCurve(tr("修改的文件数量"));
    greenPlotCurve->setPen(QPen(Qt::green,2));
    greenPlotCurve->setLegendAttribute(QwtPlotCurve::LegendShowLine);
    greenPlotCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    greenPlotCurve->attach(ui->qwtPlot);
    greenPlotCurve->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, Qt::green, QPen(Qt::black), QSize(6, 6) ) );


    bluePlotCurve= new FractionCurve(tr("最后访问的文件数量"));
    bluePlotCurve->setPen(QPen(Qt::blue,2));
    bluePlotCurve->setLegendAttribute(QwtPlotCurve::LegendShowLine);
    bluePlotCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    bluePlotCurve->attach(ui->qwtPlot);
    bluePlotCurve->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, Qt::blue,QPen(Qt::black), QSize(6, 6) ) );

    //图示
    QwtLegend *legend = new QwtLegend;
    //可复选
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

            text = tr("光标定位: 选择区内一点并按下左键");
        else
            text = tr("缩放: 滑动鼠标滚轮");

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

    info.append(tr("时间："));
    info.append(occutTimeStr);
    info.append(tr("  "));
    info.append(tr("文件数(趋势)："));
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
//显示选中的点的时间、数量
//如果只想显示在曲线上的值，则拿选中的值与qvector进行比较即可
//不太合适
void MainWindow::selected(const QPolygon &  pos )
{
    QString info;
    QString typeStr=tr("");
    bool findPoint=false;
    if(pos.size()>=1){
        if(this->inRedCurve(pos.point(0))){
            typeStr.append(tr("创建文件数 "));
            findPoint=true;
        }
        if(this->inGreenCurve(pos.point(0))){
            typeStr.append(tr("修改文件数 "));
            findPoint=true;
        }
        if(this->inBlueCurve(pos.point(0))){
            typeStr.append(tr("最后访问文件数 "));
            findPoint=true;
        }
        if(findPoint){
            typeStr.append(tr("："));
            int seconds=pos.point(0).x();
            int yValue=pos.point(0).y();
            QDateTime occurTime = curTime.addSecs(ui->qwtPlot->invTransform(QwtPlot::xBottom,seconds));
            int fileAmount=ui->qwtPlot->invTransform(QwtPlot::yLeft,yValue);
            QString occutTimeStr= occurTime.toString("yyyy-MM-dd hh:mm");
            info.append(tr("时间："));
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

//读取扫描文件结果，并写入 QVector< double > &
void MainWindow::readData(){
    /*默认文件扫描结果放在fileAmountAlteration.txt中
    *格式例子：
     *2011-10-27 21:39,800,735,600
     *分别是时间，创建文件数，修改文件数，最后访问文件数
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
        QMessageBox::warning(this, tr("检测结果文件打开错误!"),
                             tr("不能读文件 %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
    }
    //按日期排序
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
        //增加X轴,由于是过去的时间，是向左延伸
        if (redXData.last() <= xMinScale)
        {
            xMinScale *= 2;
            ui->qwtPlot->setAxisScale(QwtPlot::xBottom,xMinScale,0);
        }
    }
    //增加Y轴
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

//只显示选择的曲线
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

    printer.setCreator(tr("文件时间信息统计"));
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
    filter += tr("PDF 文件 (*.pdf)");
#ifndef QWT_NO_SVG
    filter += tr("SVG 文件 (*.svg)");
#endif
    filter += tr("Postscript 文件 (*.ps)");

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
            this, tr("导出文件"), fileName,
            filter.join(";;"), NULL, QFileDialog::DontConfirmOverwrite);
#endif

    if ( !fileName.isEmpty() )
    {
        QwtPlotRenderer renderer;
#if 0
        // 保持导出的文件与所见到的一致
        renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground, false);
        renderer.setLayoutFlag(QwtPlotRenderer::KeepMargins, true);
        renderer.setLayoutFlag(QwtPlotRenderer::KeepFrames, true);
#endif
        renderer.renderDocument(ui->qwtPlot, fileName, QSizeF(300, 200), 85);
    }
}


void MainWindow::showHelp(){
    QString helpInfo=tr("");
    helpInfo.append(tr("1.选择或取消可复选的图例能选择显示的曲线\n"));
    helpInfo.append(tr("2.按下鼠标左键，选中曲线中的一点，可查看该点的时间与文件数量信息\n"));
    helpInfo.append(tr("3.滑动鼠标滚轮，可对图形进行缩小或放大\n"));
    helpInfo.append(tr("4.按下鼠标中键(滚轮)，可对图形进行拖动\n"));
    helpInfo.append(tr("5.曲线中只有用圆点突出的数据才出自原始的检测数据"));
    QMessageBox::information (NULL,QObject::tr("帮助"),helpInfo);

}


void MainWindow::onSystemTrayIconClicked(QSystemTrayIcon::ActivationReason reason){
    if(reason==QSystemTrayIcon::Trigger){
        if(isHidden())
            show();
        else
            hide();
    }
}
