#pragma once

#include <QMainWindow>

class ImagePlugin;
class QAction;

class SwatchMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SwatchMainWindow(QWidget *parent = 0);
    virtual ~SwatchMainWindow();

public:
	virtual void keyPressEvent(QKeyEvent * event);

public:
	ImagePlugin*	getImgPlugin();

protected:
	void	createConnexionsMenu	();
	bool	loadColorWatchSettings	(QString iniFile);
	bool	openImage				(QString);
	void	switchImageSDKandReset	(QAction* actFromMenuSDK);
	void	createGraph				();

private:
    class Private;
    Private *d;
};
