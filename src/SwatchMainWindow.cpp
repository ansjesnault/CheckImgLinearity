#include "SwatchMainWindow.h"
#include "qcustomplot.h"
#include "ui_mainwindow.h"
#include "ImagePlugin.h"
#include "ColorSwatch.h"
#include "PreBuildUtil.h"

#include <QDesktopWidget>
#include <QImage>
#include <QMessageBox>

#include <iostream>
#include <memory>

class SwatchMainWindow::Private
{
public:
    Private()
		: mUi(std::unique_ptr<Ui::MainWindow>(new Ui::MainWindow))
        , mTitle("Color Swatch Munsell Neutral Value Scale test kit")
		, mImgPlg(std::unique_ptr<ImagePluginQt>(new ImagePluginQt))
		, mColorSwatch(std::shared_ptr<ColorSwatch>(nullptr))
    {}
    
	std::unique_ptr<Ui::MainWindow> mUi;
    QString							mTitle;

	std::unique_ptr<ImagePlugin>	mImgPlg;

	QString							mLoadedSettingsFilePath;
	std::shared_ptr<ColorSwatch>	mColorSwatch;

	QString							mOpenedImgFilePath;
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

bool SwatchMainWindow::loadColorWatchSettings(QString iniFile)
{
	d->mColorSwatch.reset(new ColorSwatch(d->mImgPlg.get()));

	bool isLoaded = false;
	try							
	{ 
		if( d->mColorSwatch->loadSettings(d->mLoadedSettingsFilePath) )
		{
			if( d->mColorSwatch->loadImages() )
			{
				d->mColorSwatch->fillPatchesPixelsFromMask();
				createGraph();
			}
		}
	}
	catch(std::exception &e) { std::cerr<<"[Failed to load settings] "+std::string(e.what())<<std::endl; }

	if(isLoaded = d->mColorSwatch->haveImage())
		d->mUi->label->setPixmap( QPixmap::fromImage( d->mColorSwatch->getQImage() ).scaled(d->mUi->label->size(),Qt::KeepAspectRatio) );

	d->mUi->statusBar->showMessage( 
		(isLoaded ? 
			tr("Color Swatch Settings loaded [%1 mask] : ").arg( d->mColorSwatch->haveMask() ? "with" : "without" )
			: 
			tr("Color Swatch Settings NOT loaded: "))
		+ d->mLoadedSettingsFilePath );

	std::cout<<"\n"<<*d->mColorSwatch.get()<<"\n"<<std::endl; // verbose

	return isLoaded;
}

//---------------------------------------------------------------------

bool SwatchMainWindow::openImage(QString)
{
	bool isLoaded = d->mImgPlg->loadImage(d->mOpenedImgFilePath);
	if(isLoaded) 
	{
		if(d->mColorSwatch)
		{
			d->mColorSwatch.reset();
			std::cout<<"\nReset ColorWatch data structure\n"<<std::endl; // verbose
		}
		d->mUi->label->setPixmap( QPixmap::fromImage(d->mImgPlg->toQImage()).scaled(d->mUi->label->size(),Qt::KeepAspectRatio) );
	}

	QString msg = (isLoaded ? tr("Image loaded: ") : tr("Image NOT loaded: ")) + d->mOpenedImgFilePath;
	d->mUi->statusBar->showMessage(msg);
	std::cout<<msg.toStdString()<<std::endl;
	return isLoaded;
}

//---------------------------------------------------------------------

void SwatchMainWindow::createConnexionsMenu()
{
	// Open an Image
	connect(d->mUi->action_Open, &QAction::triggered, [this]()
		{
			d->mOpenedImgFilePath = QFileDialog::getOpenFileName(this, 
					tr("Open Image"), 
					QApplication::applicationDirPath(),//+"/rsc", 
					d->mImgPlg->getImageFilterExtensions() 
				);
			d->mLoadedSettingsFilePath = QString();
			openImage(d->mOpenedImgFilePath);
		}
	);

	// Load ini file for the ColorSwatch test kit
	connect(d->mUi->action_LoadColorSwatchSettings, &QAction::triggered, [this]()
		{
			d->mLoadedSettingsFilePath = QFileDialog::getOpenFileName(this, tr("Open test kit settings"), 
					QApplication::applicationDirPath()+"/rsc", 
					tr("Ini Files (*.ini)")
				);
			d->mOpenedImgFilePath = QString();
			loadColorWatchSettings(d->mLoadedSettingsFilePath);
		}
	);

	// swtich to Qt SDK image plugin
	connect(d->mUi->action_Qt, &QAction::triggered, [this]()
		{
			d->mImgPlg.reset( new ImagePluginQt );
			switchImageSDKandReset(d->mUi->action_Qt);
		}
	);
	
	// switch to OpenImageIO SDK image plugin
	connect(d->mUi->actionOpen_ImageIO, &QAction::triggered, [this]()
		{
			d->mImgPlg.reset( new ImagePluginOIIO );
			switchImageSDKandReset(d->mUi->actionOpen_ImageIO);
		}
	);

}

//---------------------------------------------------------------------

void SwatchMainWindow::switchImageSDKandReset(QAction* actFromMenuSDK)
{
	QString msg = tr("Use ImageSDK : ")+actFromMenuSDK->iconText();
	d->mUi->statusBar->showMessage(msg, 5000); // 5s
	std::cout<<msg.toStdString()<<std::endl; // verbose
	for(QAction* act : d->mUi->menuImageSDK->actions())
	{
		act->setCheckable(true);
		act->objectName() == actFromMenuSDK->objectName() ? act->setChecked(true) : act->setChecked(false);
	}

	d->mUi->label->setPixmap(QPixmap());
	if(!d->mLoadedSettingsFilePath.isEmpty())
	{
		std::cout<<"Reload ColorWatch data structure"<<std::endl; // verbose
		loadColorWatchSettings(d->mLoadedSettingsFilePath);
	}
	else if(!d->mOpenedImgFilePath.isEmpty())
	{
		std::cout<<"Reload opened image"<<std::endl; // verbose
		openImage(d->mOpenedImgFilePath);
	}
	else
		std::cout<<"No ColorWatch and/or no image to reload"<<std::endl; // verbose
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

void SwatchMainWindow::createGraph()
{

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







































