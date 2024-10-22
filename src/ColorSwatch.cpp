#include "ColorSwatch.h"

#include "PreBuildUtil.h"
#include "ColorSwatchPatch.h"
#include "MunsellColor.h"
#include "ImagePlugin.h"
#include "ColorSwatchMask.h"

#include <QSettings>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QColor>
#include <QVector>
#include <QMap>

#include <memory>		// shared_ptr ...
#include <stdexcept>	// exceptions ...
#include <sstream>		// stringstream ...
#include <algorithm>	// std::sort ...


class ColorSwatch::Private
{
public:
	QString	mRawFile;

	std::shared_ptr<ColorSwatchMask>	mMask;
	QVector<ColorSwatchPatch*>			mPatchesList;

	ImagePlugin* mImgPlg; // not owned by this class
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------

ColorSwatch::ColorSwatch(ImagePlugin* imgPlg) : d(new Private)
{
	d->mImgPlg	= imgPlg;
}

ColorSwatch::~ColorSwatch()
{
	d->mPatchesList.clear();
	delete d;
}

//---------------------------------------------------------------------

bool ColorSwatch::loadSettings(QString iniFile)
{
	bool result = false;
	
	QSettings settings(iniFile, QSettings::Format::IniFormat);

	// we assume iniFile is absolute file path
	QDir iniFilePath(iniFile);
	iniFilePath.cdUp();

	// Get files 
	d->mRawFile		= QString();
	settings.beginGroup("colorswatch");
	{
		QString colorswatchRawfile( settings.value("rawfile").toString() ); // [MANDATORY]
		if( !QFile::exists( d->mRawFile = QDir::isRelativePath(colorswatchRawfile) ? iniFilePath.absoluteFilePath(colorswatchRawfile) : colorswatchRawfile ) )
			throw std::invalid_argument("["+FILE_LINE_FUNC_STR+"] The specified file does not exist : " + d->mRawFile.toStdString() );
		else
			result = true;
	}
	settings.endGroup();

	// Load mask info
	d->mMask.reset();
	if(settings.childGroups().contains("mask"))  // [OPTIONAL]
	{
		settings.beginGroup("mask");
		QString maskFile( settings.value("file").toString() ); // [MANDATORY  if mask section set]
		if( !QFile::exists( maskFile = QDir::isRelativePath(maskFile) ? iniFilePath.absoluteFilePath(maskFile) : maskFile ) )
		{
			throw std::invalid_argument("["+FILE_LINE_FUNC_STR+"] The specified file does not exist : " + maskFile.toStdString() );
			result = false;
		}
		else
			d->mMask.reset(new ColorSwatchMask(maskFile) );

		if(settings.childKeys().contains("backgroundcolor") && d->mMask) // [OPTIONAL]
		{
			QString colorStr = settings.value("backgroundcolor").toString();
			QColor bgColor( colorStr );
			if(!bgColor.isValid())
			{
				throw std::invalid_argument("["+FILE_LINE_FUNC_STR+"] The specified color is invalid : " + colorStr.toStdString() );
				result = false;
			}
			else
				d->mMask->backgroundColor(bgColor);
		}

		if(settings.childKeys().contains("applyAlphaMask") && d->mMask) // [OPTIONAL]
		{
			QString outApplied = settings.value("applyAlphaMask").toString();
			if( outApplied.contains("ON",Qt::CaseInsensitive) || outApplied.contains("true",Qt::CaseInsensitive) || outApplied.contains("1",Qt::CaseInsensitive) )
				d->mMask->apllyAlphaMask(true);
			else if( outApplied.contains("OFF",Qt::CaseInsensitive) || outApplied.contains("false",Qt::CaseInsensitive) || outApplied.contains("0",Qt::CaseInsensitive) )
				d->mMask->apllyAlphaMask(false);
			else
				throw std::invalid_argument("["+FILE_LINE_FUNC_STR+"] cannot read 'outputApplied'(="+outApplied.toStdString()+"). Values could be ON|on|true|TRUE|1|OFF|off|false|FALSE|0");
		}

		if(settings.childKeys().contains("outputApplied") && d->mMask) // [OPTIONAL]
		{
			QString outApplied = settings.value("outputApplied").toString();
			if( outApplied.contains("ON",Qt::CaseInsensitive) || outApplied.contains("true",Qt::CaseInsensitive) || outApplied.contains("1",Qt::CaseInsensitive) )
				d->mMask->outputAplliedMask(true);
			else if( outApplied.contains("OFF",Qt::CaseInsensitive) || outApplied.contains("false",Qt::CaseInsensitive) || outApplied.contains("0",Qt::CaseInsensitive) )
				d->mMask->outputAplliedMask(false);
			else
				throw std::invalid_argument("["+FILE_LINE_FUNC_STR+"] cannot read 'outputApplied'(="+outApplied.toStdString()+"). Values could be ON|on|true|TRUE|1|OFF|off|false|FALSE|0");
		}

		if(settings.childKeys().contains("outputPatches") && d->mMask) // [OPTIONAL]
		{
			QString outApplied = settings.value("outputPatches").toString();
			if( outApplied.contains("ON",Qt::CaseInsensitive) || outApplied.contains("true",Qt::CaseInsensitive) || outApplied.contains("1",Qt::CaseInsensitive) )
				d->mMask->outputPatches(true);
			else if( outApplied.contains("OFF",Qt::CaseInsensitive) || outApplied.contains("false",Qt::CaseInsensitive) || outApplied.contains("0",Qt::CaseInsensitive) )
				d->mMask->outputPatches(false);
			else
				throw std::invalid_argument("["+FILE_LINE_FUNC_STR+"] cannot read 'outputApplied'(="+outApplied.toStdString()+"). Values could be ON|on|true|TRUE|1|OFF|off|false|FALSE|0");
		}

		settings.endGroup();
	}

	// Load all patches info, strip by strip
	d->mPatchesList.clear();
	QVector<QVariant> reflectanceList;
	QStringList		isccnbsList;
	for(int i = 1; i <= settings.childGroups().filter("strip").size(); i++)
	{
		settings.beginGroup(QString("strip:%1").arg(i));
		{
			reflectanceList = settings.value("reflectances").toList().toVector(); // [MANDATORY if strip section set]
			if(settings.childKeys().contains("ISCCNBS")) // [OPTIONAL]
			{
				isccnbsList = settings.value("ISCCNBS").toStringList();
				if(reflectanceList.size() != isccnbsList.size())
				{
					throw std::invalid_argument("["+FILE_LINE_FUNC_STR+"] "+QString("strip:%1").arg(i).toStdString()+" section have not same reflectances and ISCC�NBS number count.");
					result = false;
				}
			}
		}
		settings.endGroup();

		int j = 0;
		for(QVariant db : reflectanceList)
		{
			d->mPatchesList.append( new ColorSwatchPatch(db.toDouble()) );
			if( isccnbsList.size()-j > 0 )
				d->mPatchesList.last()->setMunsellColor( new MunsellColor(isccnbsList.at(j++)) ); // TODO: use QColor HSV 
			else
			{
				throw std::invalid_argument("["+FILE_LINE_FUNC_STR+"] The ColorSwatchPatch with reflectance ["+QString("%1").arg(db.toDouble()).toStdString()+"] will not have any MunsellColor.");
				result = false;
			}
		}
	}

	return result;
}

//---------------------------------------------------------------------

bool ColorSwatch::loadImages()
{
	bool result = false;

	// apply settings by loading images
	if( !d->mRawFile.isEmpty() )
	{
		if(result = d->mImgPlg->loadImage(d->mRawFile))
		{
			if(d->mMask)
			{
				if( d->mMask->loadImage() )
				{
					QSize img	(d->mImgPlg->size().width(),			d->mImgPlg->size().height());
					QSize mask	(d->mMask->getImage().size().width(),	d->mMask->getImage().size().height());
					if(img != mask)
					{
						QString imgResl		= QString("img(%1,%2)").arg(img.width()).arg(img.height());
						QString imaskResl	= QString("mask(%1,%2)").arg(mask.width()).arg(mask.height());
						QString resolComp	= QString("%1 vs %2").arg(imgResl).arg(imaskResl);
						std::cerr<<resolComp.toStdString()<<std::endl;
						throw std::length_error("["+FILE_LINE_FUNC_STR+"]Image file and mask image haven't the same size! ");
					}
					else if( result && d->mMask->apllyAlphaMask() )
					{
						if(!d->mMask->applyMask( &d->mImgPlg->toQImage() ) )
						{
							writeImage2QImage();
							throw std::logic_error("["+FILE_LINE_FUNC_STR+"] Mask aplication FAILED...");
						}
						else
							std::cout<<"Mask loaded and applied it to the image."<<std::endl;
					}
					std::cout<<"Mask loaded wihtout applying it to the image."<<std::endl;
				}
				else
					throw std::invalid_argument("["+FILE_LINE_FUNC_STR+"] Mask image cannot be loaded!");
			}
			else
				throw std::invalid_argument("["+FILE_LINE_FUNC_STR+"] No mask img provided!");
		}
		else
			throw std::logic_error("["+FILE_LINE_FUNC_STR+"] Cannot load raw image!");
	}
	else
		throw std::invalid_argument("["+FILE_LINE_FUNC_STR+"] No raw image filename provided!");
	
	if(!result)
		d->mMask.reset();

	return result;
}

//---------------------------------------------------------------------

QString ColorSwatch::rawFilePathName() const
{
	return d->mRawFile;
}

bool ColorSwatch::haveImage() const
{
	return !d->mImgPlg->toQImage().isNull();
}

QImage ColorSwatch::getQImage() const
{
	return d->mImgPlg->toQImage();
}

bool ColorSwatch::haveMask() const
{
	return d->mMask;
}

QImage ColorSwatch::getMaskImg() const
{
	return d->mMask->getImage();
}

QString ColorSwatch::printPatchesInfo() const
{
	std::stringstream ss;
	ss<<"["<<d->mPatchesList.size()<<"] Patches:\t\n";
	for(int i=0; i<d->mPatchesList.size(); i++)
		ss<<"\tPatch ["<<i<<"]:"<< *d->mPatchesList[i] <<"\n";
	return QString(ss.str().c_str());
}

QString ColorSwatch::printMaskInfo() const
{
	std::stringstream ss;
	d->mMask ? ss<<"[X] Mask :\n"<<*d->mMask.get() : ss<<"[-] Mask :\n";
	return QString(ss.str().c_str());
}

//---------------------------------------------------------------------

std::ostream& operator<<(std::ostream& stream, const ColorSwatch &colorSwatch)
{
	return stream<<"ColorSwatch:\n"
		<<"["<<(colorSwatch.rawFilePathName().isEmpty()?"-":"X")<<"] Raw image:\t"			<<colorSwatch.rawFilePathName().toStdString()<<"\n"
		<<colorSwatch.printMaskInfo().toStdString()
		<<colorSwatch.printPatchesInfo().toStdString();
}

//---------------------------------------------------------------------

bool ColorSwatch::fillPatchesPixelsFromMask()
{
	bool result = false;
	if(!d->mMask)
		throw std::domain_error("["+FILE_LINE_FUNC_STR+"] Cannot fill patches without a valid loaded mask!");
	

	// try to auto detect background color
	QMap<QRgb, int> clrMap;
	for ( int row = 0; row < d->mMask->getImage().height(); row++ )
		for ( int col = 0; col < d->mMask->getImage().width(); col++ )
			if(d->mMask->getImage().valid(col,row))
				clrMap[d->mMask->getImage().pixel(col, row)]++;

	int	maxRgbCount = 0;
	for(auto& rgba : clrMap.keys())
		maxRgbCount = maxRgbCount < clrMap.value(rgba) ? clrMap.value(rgba) : maxRgbCount;

	QRgb maxRgba = clrMap.key(maxRgbCount);
	if( !d->mMask->haveBackgroundColor() )	
		d->mMask->backgroundColor( QColor(maxRgba) );
	else if(QRgb curBgRgba = d->mMask->backgroundColor().rgba() != maxRgba)
		std::cerr<<"WARNING: ["+FILE_LINE_FUNC_STR+"] Dectected background ("<<qRed(maxRgba)<<","<<qGreen(maxRgba)<<","<<qBlue(maxRgba)<<","<<qAlpha(maxRgba)<<")"
				<<"	is not the one provided in settings [default use] ("<<qRed(curBgRgba)<<","<<qGreen(curBgRgba)<<","<<qBlue(curBgRgba)<<","<<qAlpha(curBgRgba)<<")"
				<<std::endl;

	// try to extract sample patches pixels
	class Patch
	{
	public:
		Patch(QImage* img, QRgb av, int relX, int relY)
			: mImg(img), mAverage(av), mRelPixXbegin(relX), mRelPixYbegin(relY)
		{}
		QImage* mImg;
		QRgb	mAverage;
		int		mRelPixXbegin , mRelPixYbegin;
	};
	QVector<Patch*> patches;
	QRgb			bgRgb = d->mMask->backgroundColor().rgba();
	QImage			mask = d->mMask->getImage();
	unsigned int i = 0;
	for( int row = 0; row < mask.height(); row++ )
	{
		for( int col = 0; col < mask.width(); col++ )
		{
			if( mask.valid(col, row) )
			{
				QRgb rgba = mask.pixel(col, row);
				if( rgba != bgRgb )
				{
					// extract a square region to create patch
					int maxRow = row;
					int maxCol = col;
					while ( mask.pixel(col		, maxRow) != bgRgb ) { maxRow++; }
					while ( mask.pixel(maxCol	, row	) != bgRgb ) { maxCol++; }
					QImage* patchImg = new QImage( maxCol-col, maxRow-row,QImage::Format_RGB32);
					float somRed = 0.0f, somGreen = 0.0f, somBlue = 0.0f, somAlpha = 0.0f;
					int nbPixels = 0;
					for(int r=0, localRow = row; localRow < maxRow; localRow++, r++ )
					{
						for(int c=0, localCol = col; localCol < maxCol; localCol++, c++ )
						{
							QRgb val = mask.pixel(localCol, localRow);
							if( val != bgRgb ) // in case the mask is not really an aligned square
							{
								//somRed	 += qRed(val);
								//somGreen += qGreen(val);
								//somBlue	 += qBlue(val);
								//somAlpha += qAlpha(val);

								nbPixels ++;
								float pixRed = d->mImgPlg->readSinglePixelChannel(localCol, localRow, 0);
								somRed += pixRed;

								float pixGreen = d->mImgPlg->readSinglePixelChannel(localCol, localRow, 1);
								somGreen += pixGreen;

								float pixBlue = d->mImgPlg->readSinglePixelChannel(localCol, localRow, 2);
								somBlue += pixBlue; 

								QColor clr;
								clr.setRedF(pixRed);
								clr.setGreenF(pixGreen);
								clr.setBlueF(pixBlue);

								val = clr.rgb();

								patchImg->setPixel(c,r,val);
								mask.setPixel(localCol, localRow, bgRgb);
							}
						}
					}
					QRgb patchRgbaAverage = qRgba((somRed/nbPixels)*255.0f, (somGreen/nbPixels)*255.0f, (somBlue/nbPixels)*255.0f, (somAlpha/nbPixels)*255.0f);
					patches.push_back( new Patch(patchImg, patchRgbaAverage, col, row) );

					// save patch QImage in order of detection
					if(d->mMask->outputPatches())
					{
						QString patchFile = QString("patch_%1.png").arg(i++);
						patchImg->save(patchFile);
						std::cout<<"Saved patch (in order of detection):"<<patchFile.toStdString()<<" ["<<patchImg->width()<<"x"<<patchImg->height()<<"]"<<std::endl;;
					}
				}
			}
		}
	}


	// try to relie list of the local mask patches with the ColorSwatchPatch list
	std::sort(patches.begin(), patches.end(), [](Patch* lhs, Patch* rhs) 
		{
			return	( qRed(lhs->mAverage) + qGreen(lhs->mAverage) + qBlue(lhs->mAverage) + qAlpha(lhs->mAverage) )
					< ( qRed(rhs->mAverage) + qGreen(rhs->mAverage) + qBlue(rhs->mAverage) + qAlpha(rhs->mAverage) );
		} ); // from black (0) to white (255)

	if(patches.size() != d->mPatchesList.size())
	{
		writeImage2QImage();
		throw std::length_error("["+FILE_LINE_FUNC_STR+"] Detected patches are not equal to number of provided patches reflectance ("+(patches.size() < d->mPatchesList.size() ? "<)" : ">)") );
	}

	for(int i=0; i<patches.size(); i++)
		if(d->mPatchesList.size() >= i)
			d->mPatchesList[i]->setImage(patches[i]->mImg, patches[i]->mRelPixXbegin, patches[i]->mRelPixYbegin);

	patches.clear();


	// compute averages pixels (but this time) using the SDK img provided and directly from the raw img pixels coords
	if(!d->mImgPlg)
		throw std::logic_error("["+FILE_LINE_FUNC_STR+"] Cannot computeAverageRGBpixel since SDK image is not available!");

	for(ColorSwatchPatch* patch : d->mPatchesList)
		patch->computeAverageRGBpixel(d->mImgPlg);

	return result = true;
}

//---------------------------------------------------------------------

ColorSwatch::GraphData2D ColorSwatch::getGraphData(DATA datalist)
{
	GraphData2D data;
	switch(datalist)
	{
	case DATA::REF : // linearity
		{
			// first point (min)
			data.first.push_back(0);	//x (reflectance %)
			data.second.push_back(0);	//y (float channel average [0-1])
			// second point (max)
			data.first.push_back(100);	//x (reflectance %)
			data.second.push_back(1);	//y (float channel average [0-1])
			break;
		}
	case DATA::R : //red channel patches averages
		{
			for(ColorSwatchPatch* patch : d->mPatchesList)
			{
				data.first.push_back (patch->getReflectance());
				data.second.push_back(patch->getAverageColor().redF());
			}
			break;
		}
	case DATA::G : //green channel patches averages
		{
			for(ColorSwatchPatch* patch : d->mPatchesList)
			{
				data.first.push_back (patch->getReflectance());
				data.second.push_back(patch->getAverageColor().greenF());
			}
			break;
		}
	case DATA::B : //red channel patches averages
		{
			for(ColorSwatchPatch* patch : d->mPatchesList)
			{
				data.first.push_back (patch->getReflectance());
				data.second.push_back(patch->getAverageColor().blueF());
			}
			break;
		}
	case DATA::A : //alpha channel patches averages
		{
			for(ColorSwatchPatch* patch : d->mPatchesList)
			{
				data.first.push_back (patch->getReflectance());
				data.second.push_back(patch->getAverageColor().alphaF());
			}
			break;
		}
	default : break;
	}
	return data;
}

//---------------------------------------------------------------------

void ColorSwatch::writeImage2QImage()
{
	// save/write image mask converted to ARGB32 to help see what happened
	if(!d->mMask->getImage().isNull())
	{
		QString maskFile = QString("mask_argb32.png");
		d->mMask->getImage().save(maskFile);
		std::cout<<"Save "<<maskFile.toStdString()<<" ["<<d->mMask->getImage().width()<<"x"<<d->mMask->getImage().height()<<"]"<<std::endl;
	}
}