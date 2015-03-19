
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
	void	createConnexionsMenu();
	void	exclusiveMenuImageSDKActionCheck(QAction* actFromMenuSDK);

private:
    class Private;
    Private *d;
};
