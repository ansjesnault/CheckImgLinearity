#include "ImagePlugin.h"

#include <QImage>
#include <QStringlist>
#include <QByteArray>
#include <QImageReader>

#include <memory>

//---------------------------------------------------------------------
//---------------------------------------------------------------------

class ImagePluginQt::Private
{
public:
	Private()
		: mQimg(std::make_shared<QImage>())
		, mQimgMask(std::unique_ptr<QImage>(new QImage))
	{}
public:
	std::shared_ptr<QImage> mQimg;
	std::unique_ptr<QImage> mQimgMask;
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

bool ImagePluginQt::loadImageMask(QString filename)
{
	d->mQimgMask.reset(new QImage);
	return d->mQimgMask->load(filename);
}

bool ImagePluginQt::isImageMaskLoaded()
{
	return !d->mQimgMask->isNull();
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