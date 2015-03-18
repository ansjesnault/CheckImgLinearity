#include "SwatchMainWindow.h"
#include "qcustomplot.h"
#include "ui_mainwindow.h"
#include "ImagePlugin.h"

#include <QDesktopWidget>
#include <QMessageBox>

#include <iostream>
#include <memory>

class SwatchMainWindow::Private
{
public:
    Private()
		: mUi(std::unique_ptr<Ui::MainWindow>(new Ui::MainWindow))
        , mTitle("Swatch Munsell Neutral Value Scale test kit")
		, mImgPlg(std::unique_ptr<ImagePluginQt>(new ImagePluginQt))
    {}
    
	std::unique_ptr<Ui::MainWindow> mUi;
    QString							mTitle;
	std::unique_ptr<ImagePlugin>	mImgPlg;
	QString							mMaskImg;	//path
	QString							mImg;		//path
};

//---------------------------------------------------------------------

SwatchMainWindow::SwatchMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , d(new Private)
{
    d->mUi->setupUi(this);
	QRect rec = QApplication::desktop()->screenGeometry();
	setMaximumSize(rec.width(), rec.height());
	setMinimumSize(800, 600);
    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    setWindowTitle(d->mTitle);
    statusBar()->clearMessage();
    d->mUi->customPlot->replot();
	createConnexionsMenu();
}

SwatchMainWindow::~SwatchMainWindow()
{
    delete d;
}

//---------------------------------------------------------------------

ImagePlugin* SwatchMainWindow::getImgPlugin() 
{
	return d->mImgPlg.get();
}

//---------------------------------------------------------------------

void SwatchMainWindow::createConnexionsMenu()
{
	connect(d->mUi->actionLoad_Mask, &QAction::triggered, [this]()
		{
			d->mMaskImg = QFileDialog::getOpenFileName(this, 
					tr("Open Image Mask"), 
					QApplication::applicationDirPath()+"/rsc", 
					getImgPlugin()->getImageFilterExtensions() 
				);
			bool status = d->mImgPlg->loadImageMask(d->mMaskImg);
			d->mUi->statusBar->showMessage( (status ? tr("Mask loaded: ") : tr("Mask NOT loaded: ")) + d->mMaskImg);
		}
	);

	connect(d->mUi->action_Open, &QAction::triggered, [this]()
		{
			d->mImg = QFileDialog::getOpenFileName(this, 
					tr("Open Image"), 
					QApplication::applicationDirPath()+"/rsc", 
					getImgPlugin()->getImageFilterExtensions() 
				);
			bool status = d->mImgPlg->loadImage(d->mImg);
			if(status) d->mUi->label->setPixmap( QPixmap::fromImage(d->mImgPlg->toQImage()) );
			d->mUi->statusBar->showMessage( (status ? tr("Image loaded: ") : tr("Image NOT loaded: ")) + d->mImg);
		}
	);

	connect(d->mUi->action_Qt, &QAction::triggered, [this]()
		{
			d->mImgPlg.reset( new ImagePluginQt );
			d->mUi->statusBar->showMessage(tr("Reset ImageSDK to : ")+d->mUi->action_Qt->iconText(), 5000); // 5s
			exclusiveMenuImageSDKActionCheck(d->mUi->action_Qt);
		}
	);

}

//---------------------------------------------------------------------

void SwatchMainWindow::exclusiveMenuImageSDKActionCheck(QAction* actFromMenuSDK)
{
	for(QAction* act : d->mUi->menuImageSDK->actions())
		act->objectName() == actFromMenuSDK->objectName() ? act->setChecked(true) : act->setChecked(false);
}

//---------------------------------------------------------------------

void SwatchMainWindow::keyPressEvent(QKeyEvent * event)
{
	switch(event->key())
	{
	case Qt::Key::Key_Escape : this->close(); break;
	default:
		break;
	}
	QMainWindow::keyPressEvent(event);
}

//---------------------------------------------------------------------

/*
void MainWindow::setupSimpleDemo(QCustomPlot *customPlot)
{
  demoName = "Simple Demo";
  
  // add two new graphs and set their look:
  customPlot->addGraph();
  customPlot->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
  customPlot->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20))); // first graph will be filled with translucent blue
  customPlot->addGraph();
  customPlot->graph(1)->setPen(QPen(Qt::red)); // line color red for second graph
  // generate some points of data (y0 for first, y1 for second graph):
  QVector<double> x(250), y0(250), y1(250);
  for (int i=0; i<250; ++i)
  {
    x[i] = i;
    y0[i] = qExp(-i/150.0)*qCos(i/10.0); // exponentially decaying cosine
    y1[i] = qExp(-i/150.0);              // exponential envelope
  }
  // configure right and top axis to show ticks but no labels:
  // (see QCPAxisRect::setupFullAxesBox for a quicker method to do this)
  customPlot->xAxis2->setVisible(true);
  customPlot->xAxis2->setTickLabels(false);
  customPlot->yAxis2->setVisible(true);
  customPlot->yAxis2->setTickLabels(false);
  // make left and bottom axes always transfer their ranges to right and top axes:
  connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
  connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));
  // pass data points to graphs:
  customPlot->graph(0)->setData(x, y0);
  customPlot->graph(1)->setData(x, y1);
  // let the ranges scale themselves so graph 0 fits perfectly in the visible area:
  customPlot->graph(0)->rescaleAxes();
  // same thing for graph 1, but only enlarge ranges (in case graph 1 is smaller than graph 0):
  customPlot->graph(1)->rescaleAxes(true);
  // Note: we could have also just called customPlot->rescaleAxes(); instead
  // Allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking:
  customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
}
*/
/*
void MainWindow::screenShot()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  QPixmap pm = QPixmap::grabWindow(qApp->desktop()->winId(), this->x()+2, this->y()+2, this->frameGeometry().width()-4, this->frameGeometry().height()-4);
#else
  QPixmap pm = qApp->primaryScreen()->grabWindow(qApp->desktop()->winId(), this->x()+2, this->y()+2, this->frameGeometry().width()-4, this->frameGeometry().height()-4);
#endif
  QString fileName = "plot.png";
  fileName.replace(" ", "");
  pm.save("./screenshots/"+fileName);
  qApp->quit();
}
*/







































