
#pragma once

#include <QMainWindow>

class ImagePlugin;
class QAction;

class SwatchMainWindow : public QMainWindow
{
    Q_OBJECT

public: //constructor/destructor
    explicit SwatchMainWindow(QWidget *parent = 0);
    virtual ~SwatchMainWindow();

public: // overloaded
	virtual void keyPressEvent(QKeyEvent * event);

public: // intern
	ImagePlugin*	getImgPlugin();

protected: // intern
	void	createConnexionsMenu();
	void	exclusiveMenuImageSDKActionCheck(QAction* actFromMenuSDK);

private: // internal data
    class Private;
    Private *d;
};
