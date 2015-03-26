#include "ImagePlugin.h"
#include "PreBuildUtil.h"

#include <QImage>
#include <QColor>
#include <QStringlist>
#include <QByteArray>
#include <QImageReader>

#include <memory>
#include <iostream>

//---------------------------------------------------------------------
//---------   ImagePluginQt  ----------------------------------------
//---------------------------------------------------------------------

class ImagePluginQt::Private
{
public:
	Private() : mQimg(std::make_shared<QImage>())
	{}
public:
	std::shared_ptr<QImage> mQimg;
};

//---------------------------------------------------------------------

ImagePluginQt::ImagePluginQt() 
	: ImagePlugin()
	, d(new Private)
{

}

ImagePluginQt::~ImagePluginQt() 
{
	delete d;
}

//---------------------------------------------------------------------

QString ImagePluginQt::getImageFilterExtensions()
{
    QStringList imageExts;
    for(const QByteArray &format : QImageReader::supportedImageFormats())
        imageExts.append(QString("*.%1").arg(format.data()));

    return !imageExts.empty() ? QString("Image (%1)").arg(imageExts.join(" ")) : QString();
}

//---------------------------------------------------------------------

bool ImagePluginQt::loadImage(QString filename)
{
	d->mQimg.reset(new QImage);
	return d->mQimg->load(filename);
}

//---------------------------------------------------------------------

QImage ImagePluginQt::toQImage()
{
	return d->mQimg ? *d->mQimg.get() : QImage();
}

//---------------------------------------------------------------------

QSize ImagePluginQt::size()
{
	return d->mQimg ? d->mQimg->size() : QSize();
}

//---------------------------------------------------------------------

bool ImagePluginQt::averagesChannels(pixelsCoords pixCoords, float &r, float &g, float &b, float &a)
{
	bool err = false;
	int nbPixels = 0;

	/*// This method use bytes [0-255] channels averages computation
	int somRed = 0, somGreen = 0, somBlue = 0, somAlpha = 0;
	for(auto pixCoord : pixCoords)
	{
		if(d->mQimg->valid(pixCoord.first, pixCoord.second))
		{
			QRgb pix  = d->mQimg->pixel(pixCoord.first, pixCoord.second);
			somRed	 += qRed(pix);
			somGreen += qGreen(pix);
			somBlue	 += qBlue(pix);
			somAlpha += qAlpha(pix);
			nbPixels ++;
		}
		else
			err = true;
	}
	// Convert pixel bytes [0-255] to float precision
	r = (somRed/nbPixels)/255.0f;
	g = (somGreen/nbPixels)/255.0f;
	b = (somBlue/nbPixels)/255.0f;
	a = (somAlpha/nbPixels)/255.0f;
	*/

	// this method use float [0-1] channel averages computation 
	for(auto pixCoord : pixCoords)
	{
		if(d->mQimg->valid(pixCoord.first, pixCoord.second))
		{
			QColor color(d->mQimg->pixel(pixCoord.first, pixCoord.second));
			r += color.redF();
			g += color.greenF();
			b += color.blueF();
			a += color.alphaF();
			nbPixels ++;
		}
		else
			err = true;
	}
	float scale = 1.0f / float(nbPixels);
	r = r*scale;
	g = g*scale;
	b = b*scale;
	a = a*scale;

	if(err)
		std::cout<<"["<<FILE_LINE_FUNC_STR<<"] ERROR occured. Some invalid pixel was detected. Averages will be affected."<<std::endl;
	else
		return true;
	return false;
}




//---------------------------------------------------------------------
//---------   ImagePluginOIIO  ----------------------------------------
//---------------------------------------------------------------------

class ImagePluginOIIO::Private
{
public:
	Private()
	{}
public:
};

//---------------------------------------------------------------------

ImagePluginOIIO::ImagePluginOIIO() 
	: ImagePlugin()
	, d(new Private)
{

}

ImagePluginOIIO::~ImagePluginOIIO() 
{
	delete d;
}

//---------------------------------------------------------------------