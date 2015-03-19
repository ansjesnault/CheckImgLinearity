#pragma once
#include <QString>
#include <QImage>

class ImagePlugin;

class ColorSwatch
{
public:
	ColorSwatch(ImagePlugin* imgPlg);
	virtual ~ColorSwatch();

public:
	bool	loadSettings(QString iniFile);
	bool	loadImages();
	bool	haveImage();
	bool	haveMask();
	QImage	getMask();
	QImage	getQImage();

private:
	class Private;
	Private* d;
};