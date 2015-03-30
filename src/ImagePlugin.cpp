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
	// format: 'Image (*.png *.jpg *.bmp)')
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

	bool useFloat = true;
	if(!useFloat)
	{
		// This method use bytes [0-255] channels averages computation
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
	}
	else
	{
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
	}

	if(err)
		std::cout<<"["<<FILE_LINE_FUNC_STR<<"] ERROR occured. Some invalid pixel was detected. Averages will be affected."<<std::endl;
	else
		return true;

	return false;
}




//---------------------------------------------------------------------
//---------   ImagePluginOIIO  ----------------------------------------
//---------------------------------------------------------------------

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagecache.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/platform.h>

OIIO_NAMESPACE_USING;

class ImagePluginOIIO::Private
{
public:
	Private()
	{}

public:
	std::unique_ptr<ImageInput> mImgIn;
	std::unique_ptr<ImageBuf>	mImgBuf;

	std::string mCurrentFileName;
	std::shared_ptr<QImage> mQimg;
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

QString ImagePluginOIIO::getImageFilterExtensions()	// needed format: 'Image (*.png *.jpg *.bmp)')
{
	// oiio format: <foramt>:<extension>,<extension>,<...>;<format><...>
	// example: tiff:tif;jpeg:jpg,jpeg;openexr:exr
	std::string extension_list;
	getattribute("extension_list",extension_list);
	
	QString imgFilterExt("Image (");
	QStringList formatsList = QString(extension_list.c_str()).split(";");
	for(QString format : formatsList)
	{
		QStringList namesList	= format.split(":");
		QStringList extList		= namesList.last().split(",");
		for(QString ext : extList)
			imgFilterExt.append(QString("*.%1 ").arg(ext));
	}
	
	//replace last space by ')' to close the needed format string to return
	imgFilterExt.replace(imgFilterExt.length(), 1, ")");

	return imgFilterExt;
}

//---------------------------------------------------------------------

bool ImagePluginOIIO::loadImage(QString filename)
{
	d->mCurrentFileName = filename.toStdString();
	d->mImgIn.reset(	ImageInput::open(d->mCurrentFileName)	);
	d->mImgBuf.reset(	new ImageBuf(d->mCurrentFileName)		);
	d->mQimg.reset();
	return d->mImgIn.get() ? true : false;
}

//---------------------------------------------------------------------

QImage ImagePluginOIIO::toQImage()
{
	if(d->mImgIn == nullptr)
	{
		std::cerr<<"["<<FILE_LINE_FUNC_STR<<"]image not loaded...abort."<<std::endl;
		return QImage();
	}

	// shortcut necessary to avoid reloading same images each time we need it (Qt GUI resize event)
	if(d->mQimg != nullptr)
		return *d->mQimg.get();

	d->mQimg.reset(new QImage(d->mImgIn->spec().width, d->mImgIn->spec().height, QImage::Format_RGB32) );

	ImageSpec spec = d->mImgIn->spec();
	int ncTot	= spec.nchannels;
	int nc3		= std::min(3, ncTot);			// we only need RGB channel data for Format_RGB32
	std::vector<float> scanline( spec.width * ncTot);
	for( int row = 0; row < spec.height; row++ )
	{
		d->mImgIn->read_scanline( row+spec.y, 0+spec.z, TypeDesc::FLOAT, &scanline[0] );
		for( int col = 0; col < spec.width; col++ )
		{
			float rgb[3] = { 0, 0, 0 };
			for(int i=0; i<nc3; ++i)
				rgb[i] = scanline[col*ncTot+i];

			QColor color;
			color.setRedF	(rgb[0]);
			color.setGreenF	(rgb[1]);
			color.setBlueF	(rgb[2]);

			d->mQimg->setPixel(col, row, color.rgb());
		}
	}

	return *d->mQimg.get();
}

//---------------------------------------------------------------------

QSize ImagePluginOIIO::size()
{
	if(d->mImgBuf == nullptr)
	{
		std::cerr<<"["<<FILE_LINE_FUNC_STR<<"]image not loaded...abort."<<std::endl;
		return QSize();
	}
	return QSize(d->mImgBuf->spec().width, d->mImgBuf->spec().height);
}

//---------------------------------------------------------------------

bool ImagePluginOIIO::averagesChannels(pixelsCoords pixCoords, float &r, float &g, float &b, float &a)
{
	if(d->mImgBuf == nullptr)
	{
		std::cerr<<"["<<FILE_LINE_FUNC_STR<<"]image not loaded...abort."<<std::endl;
		return false;
	}

	if(!d->mImgBuf->read(0,0,false,TypeDesc::FLOAT))
	{
		std::cerr<<"["<<FILE_LINE_FUNC_STR<<"]image buffer can't read image...abort."<<std::endl;
		return false;
	}

	int nc = d->mImgBuf->nchannels();
	std::vector<float> total (nc, 0.0f);
	for(auto pixCoord : pixCoords)
	{
		float* pixel = OIIO_ALLOCA(float,nc);
		d->mImgBuf->getpixel(pixCoord.first, pixCoord.second, pixel);
		for (int c = 0; c < nc; c++)
			total[c] += pixel[c];
	}

	r = total[0]/float(pixCoords.size());
	g = total[1]/float(pixCoords.size());
	b = total[2]/float(pixCoords.size());
	if(nc>=3)
		a = total[3]/float(pixCoords.size());

	return true;
}