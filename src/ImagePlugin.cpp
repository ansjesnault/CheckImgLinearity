#include "ImagePlugin.h"

#include <QImage>
#include <QStringlist>
#include <QByteArray>
#include <QImageReader>

#include <memory>

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
	return *d->mQimg.get();
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

QString ImagePluginOIIO::getImageFilterExtensions()
{
	return QString();
}

//---------------------------------------------------------------------

bool ImagePluginOIIO::loadImage(QString filename)
{
	return false;
}

//---------------------------------------------------------------------

QImage ImagePluginOIIO::toQImage()
{
	return QImage();
}