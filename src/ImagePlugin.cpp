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

float ImagePluginQt::readSinglePixelChannel(int x, int y, int channel)
{
	if(!d->mQimg)
	{
		std::cerr<<"["<<FILE_LINE_FUNC_STR<<"]Cannot continue without a valid QImage loaded."<<std::endl;
		return false;
	}
	if(d->mQimg->valid(x, y))
	{
		QRgb pix = d->mQimg->pixel(x,y);
		switch(channel)
		{
		case 0:		return qRed(pix)/255.0f;	break;
		case 1:		return qGreen(pix)/255.0f;	break;
		case 2:		return qBlue(pix)/255.0f;	break;
		case 3:		return qAlpha(pix)/255.0f;	break;
		default:	return 0.0f;
		}
	}
	return 0.0f;
}

//---------------------------------------------------------------------

bool ImagePluginQt::averagesChannels(pixelsCoords pixCoords, float &r, float &g, float &b, float &a)
{
	if(!d->mQimg)
	{
		std::cerr<<"["<<FILE_LINE_FUNC_STR<<"]Cannot continue without a valid QImage loaded."<<std::endl;
		return false;
	}

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

bool ImagePluginQt::save(QString filename)
{
	if(!d->mQimg)
	{
		std::cerr<<"["<<FILE_LINE_FUNC_STR<<"]Cannot continue without a valid QImage loaded."<<std::endl;
		return false;
	}
	std::cout<<"Saving QImage: "<<filename.toStdString()<<std::flush;
	bool ok = d->mQimg->save(filename);
	std::cout<<(ok?" ...Done":"...FAILED")<<std::endl;
	return ok;
}




//---------------------------------------------------------------------
//---------   ImagePluginOIIO  ----------------------------------------
//---------------------------------------------------------------------

#include <OpenImageIO/platform.h> // for pixel allocation
#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

OIIO_NAMESPACE_USING;

class ImagePluginOIIO::Private
{
public:
	std::string					mCurrentFileName;
	std::shared_ptr<ImageBuf>	mImgBuf;
	std::shared_ptr<QImage>		mQimg;
};

//---------------------------------------------------------------------

ImagePluginOIIO::ImagePluginOIIO() 
	: ImagePlugin()
	, d(new Private)
{
	//activate default ColorSpace (mean this class handle/impl toColorSpace function)
	mColorSpace="Linear";
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
	d->mImgBuf.reset( new ImageBuf(d->mCurrentFileName) );
	d->mQimg.reset();
	return d->mImgBuf.get() ? true : false;
}

//---------------------------------------------------------------------

QImage ImagePluginOIIO::toQImage()
{
	if(d->mImgBuf == nullptr)
	{
		std::cerr<<"["<<FILE_LINE_FUNC_STR<<"]image not loaded...abort."<<std::endl;
		return QImage();
	}

	// shortcut necessary to avoid reloading same images each time we need it (Qt GUI resize event)
	if(d->mQimg != nullptr)
		return *d->mQimg.get();

	bool useNative = false;
	ImageSpec spec = (useNative ? d->mImgBuf->nativespec() : d->mImgBuf->spec());
	//std::cout<<"file format name :"<<d->mImgBuf->file_format_name()<<std::endl;
	//std::cout<<(useNative?"native format ":"format ")<<spec.format<<std::endl;
	if(spec.nchannels < 3)
	{
		std::cerr<<"["<<FILE_LINE_FUNC_STR<<"]Only channels numbers > 3 is allowed for QImage conversion...abort."<<std::endl;
		return QImage();
	}

	d->mQimg.reset(new QImage(spec.width, spec.height, QImage::Format_RGB32) );

	/* // this code could be use if you use (instead of imageBuf) ImageInput pointer : d->mImgIn.reset(ImageInput::open(d->mCurrentFileName));
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
	*/

	// TODO: try to add progress callback to read() function
	if( !d->mImgBuf->read(0, 0, false, spec.format) ) //spec.format //TypeDesc::BASETYPE::FLOAT
		std::cerr<<"["<<FILE_LINE_FUNC_STR<<"]Cannot read imageBuf...abort."<<std::endl;

	// This switch demonstrate how to manipulate specific data format or to handle directly default floating pixel format (conversion is auto handled internaly)
	switch(spec.format.basetype)
	{
	case TypeDesc::BASETYPE::FLOAT : 
		{
			// ImageBuf iterator is based on <BUFT,USERT=float> the type of data stored in buffer AND the type of retrive data you want to manipulate
			for( ImageBuf::ConstIterator<float> it(*d->mImgBuf.get()); !it.done(); ++it)
			{
				QColor color;// float [0-1]
				color.setRedF	(it[0]);
				color.setGreenF	(it[1]);
				color.setBlueF	(it[2]);
				d->mQimg->setPixel(it.x(), it.y(), color.rgb());
			}
			break;
		}
	case TypeDesc::BASETYPE::UINT8 :
		{
			for( ImageBuf::ConstIterator<unsigned char, unsigned char> it(*d->mImgBuf.get()); !it.done(); ++it)
			{
				QColor color(it[0], it[1], it[2]); // UINT8 [0-255]
				d->mQimg->setPixel(it.x(), it.y(), color.rgb());
			}
			break;
		}
	default:
		{
			std::cerr<<"["<<FILE_LINE_FUNC_STR<<"] "<<spec.format<<" format is not yet handled here. Try forcing float format."<<std::endl;
			d->mImgBuf->reset(d->mCurrentFileName);
			if(d->mImgBuf->read(0, 0, true, TypeDesc::BASETYPE::FLOAT)) // force FLOAT buffer convertion
			{
				for( ImageBuf::ConstIterator<float> it(*d->mImgBuf.get()); !it.done(); ++it)
				{
					QColor color;	// float [0-1]
					color.setRedF	(it[0]);
					color.setGreenF	(it[1]);
					color.setBlueF	(it[2]);
					d->mQimg->setPixel(it.x(), it.y(), color.rgb());
				}
			}
			else
			{
				std::cerr<<"["<<FILE_LINE_FUNC_STR<<"]"<<"Failed to read...abort"<<std::endl;
				return QImage();
			}
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

float ImagePluginOIIO::readSinglePixelChannel(int x, int y, int channel)
{
	if(d->mImgBuf == nullptr)
	{
		std::cerr<<"["<<FILE_LINE_FUNC_STR<<"] image not loaded...abort."<<std::endl;
		return false;
	}

	if(!d->mImgBuf->initialized())
		d->mImgBuf->read(0, 0, true, TypeDesc::BASETYPE::FLOAT);

	int nc = d->mImgBuf->nchannels();
	float* pixel = OIIO_ALLOCA(float,nc);
	d->mImgBuf->getpixel(x, y, pixel);
	return pixel[channel];
}

//---------------------------------------------------------------------

bool ImagePluginOIIO::averagesChannels(pixelsCoords pixCoords, float &r, float &g, float &b, float &a)
{
	if(d->mImgBuf == nullptr)
	{
		std::cerr<<"["<<FILE_LINE_FUNC_STR<<"] image not loaded...abort."<<std::endl;
		return false;
	}
	
	if(!d->mImgBuf->read(0,0,false,TypeDesc::FLOAT))
	{
		std::cerr<<"["<<FILE_LINE_FUNC_STR<<"] image buffer can't read image...abort."<<std::endl;
		return false;
	}

	int nc = d->mImgBuf->nchannels();
	std::vector<float> total (nc, 0.0f);
	for(auto pixCoord : pixCoords)
	{
		float* pixel = OIIO_ALLOCA(float,nc);
		d->mImgBuf->getpixel(pixCoord.first, pixCoord.second, pixel);
		for (int c = 0; c <= nc; c++)
			total[c] += pixel[c];
	}

	r = total[0]/float(pixCoords.size());
	g = total[1]/float(pixCoords.size());
	b = total[2]/float(pixCoords.size());
	if(nc>=3)
		a = total[3]/float(pixCoords.size());

	return true;
}

//---------------------------------------------------------------------

bool ImagePluginOIIO::save(QString filename)
{
	if(d->mImgBuf == nullptr)
	{
		std::cerr<<"["<<FILE_LINE_FUNC_STR<<"] image not loaded...abort."<<std::endl;
		return false;
	}
	std::cout<<"Saving QImage: "<<filename.toStdString()<<std::flush;
	bool ok = d->mImgBuf->write(filename.toStdString());
	std::cout<<(ok?" ...Done":"...FAILED")<<std::endl;
	return ok;
}

//---------------------------------------------------------------------

bool ImagePluginOIIO::colorSpaceConversion()
{
	if(d->mImgBuf == nullptr)
	{
		std::cerr<<"["<<FILE_LINE_FUNC_STR<<"] image not loaded...abort."<<std::endl;
		return false;
	}
	if(mColorSpace == "Linear"			|| 
		mColorSpace == "sRGB"			||
		mColorSpace == "GammaCorrected" ||
		mColorSpace == "AdobeRGB"		||
		mColorSpace == "Rec709"			|| 
		mColorSpace == "KodakLog"		)
	{
		if(d->mImgBuf->spec().get_string_attribute("oiio:ColorSpace") != mColorSpace.toStdString())
		{
			std::shared_ptr<ImageBuf> ccSrc (d->mImgBuf); // color-corrected buffer
			if (d->mImgBuf->spec().format != TypeDesc::FLOAT) 
			{
				// If the original src buffer isn't float, make a scratch space that is float.
				ImageSpec floatSpec = d->mImgBuf->spec();
				floatSpec.set_format (TypeDesc::FLOAT);
				ccSrc.reset (new ImageBuf (floatSpec));
			}
			ccSrc->read(0,0,true,TypeDesc::FLOAT);
			ImageBufAlgo::colorconvert(*ccSrc, *d->mImgBuf, "current", mColorSpace.toStdString() );
			
			if(ccSrc->has_error())
				std::cerr<<"["<<FILE_LINE_FUNC_STR<<"] color-corrected buffer error: "<<ccSrc->geterror()<<std::endl;
			if(d->mImgBuf->has_error())
				std::cerr<<"["<<FILE_LINE_FUNC_STR<<"] mImgBuf buffer error: "<<d->mImgBuf->geterror()<<std::endl;
			
			std::swap(d->mImgBuf, ccSrc);

			d->mImgBuf->write("imgBuf.CR2");
			d->mImgBuf->write("imgBuf.png");
			ccSrc->write("ccSrc.CR2");
			ccSrc->write("ccSrc.png");

			return true;
		}
		else
		{
			std::cout<<"["<<FILE_LINE_FUNC_STR<<"] no need colorspace conversion, current one is: ["<<mColorSpace.toStdString()<<"]"<<std::endl;
			return false;
		}
	}
	else
	{
		std::cerr<<"["<<FILE_LINE_FUNC_STR<<"] colorspace destination name ["<<mColorSpace.toStdString()<<"] is not handled"<<std::endl;
		return false;
	}
}