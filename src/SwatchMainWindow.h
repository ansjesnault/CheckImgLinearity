#pragma once

#include <QMainWindow>
#include <QPair>
#include <QVector>

class ImagePlugin;
class QAction;

class SwatchMainWindow : public QMainWindow
{
    Q_OBJECT
public:
	typedef QVector<double> GraphData1D;
	typedef QPair<GraphData1D,GraphData1D> GraphData2D;


public:
    explicit SwatchMainWindow(QWidget *parent = 0);
    virtual ~SwatchMainWindow();

public:
	virtual void keyPressEvent	(QKeyEvent	  * event);
	virtual void resizeEvent	(QResizeEvent * event);

public:
	ImagePlugin*	getImgPlugin();

protected:
	void	createConnexionsMenu	();
	bool	loadColorWatchSettings	(QString iniFile);
	bool	openImage				(QString);
	void	switchImageSDKandReset	(QAction* actFromMenuSDK);
	void	createGraph				(GraphData2D graphRef, 
									 GraphData2D graphR,
									 GraphData2D graphG,
									 GraphData2D graphB,
									 GraphData2D graphA );

private:
    class Private;
    Private *d;
};
