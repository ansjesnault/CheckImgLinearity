#include "ColorSwatch.h"

#include "PreBuildUtil.h"
#include "ColorSwatchPatch.h"
#include "MunsellColor.h"
#include "ImagePlugin.h"
#include "ColorSwatchMask.h"

#include <QVector>
#include <QSettings>
#include <QVariant>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QColor>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <cstring>


class ColorSwatch::Private
{
public:
	QString						mRawFile;
	QString						mImgFile;

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
	d->mImgFile		= QString();
	settings.beginGroup("colorswatch");
	{
		QString colorswatchRawfile( settings.value("rawfile").toString() ); // [MANDATORY]
		if( !QFile::exists( d->mRawFile = QDir::isRelativePath(colorswatchRawfile) ? iniFilePath.absoluteFilePath(colorswatchRawfile) : colorswatchRawfile ) )
			throw std::invalid_argument("["+FILE_LINE_FUNC_STR+"] The specified file does not exist : " + d->mRawFile.toStdString() );
		else
			result = true;

		if(settings.childKeys().contains("file")) // [OPTIONAL]
		{
			QString colorswatchFile( settings.value("file").toString() );
			if( !QFile::exists( d->mImgFile = QDir::isRelativePath(colorswatchFile) ? iniFilePath.absoluteFilePath(colorswatchFile) : colorswatchFile ) )
			{
				throw std::invalid_argument("["+FILE_LINE_FUNC_STR+"] The specified file does not exist : " + d->mImgFile.toStdString() );
				result = false;
			}
		}
	}
	settings.endGroup();

	// Load mask info
	d->mMask.reset();
	if(settings.childGroups().contains("mask"))  // [OPTIONAL]
	{
		settings.beginGroup("mask");
		QString maskFile( settings.value("file").toString() ); // [MANDATORY  if mask section set]
		if( !QFile::exists( /*d->mImgMaskFile*/maskFile = QDir::isRelativePath(maskFile) ? iniFilePath.absoluteFilePath(maskFile) : maskFile ) )
		{
			throw std::invalid_argument("["+FILE_LINE_FUNC_STR+"] The specified file does not exist : " + /*d->mImgMaskFile*/maskFile.toStdString() );
			result = false;
		}
		else
			d->mMask.reset(new ColorSwatchMask(maskFile) );

		if(settings.childKeys().contains("samplecolor") && d->mMask)  // [OPTIONAL]
		{
			QString colorStr = settings.value("samplecolor").toString();
			QColor sampleColor( colorStr );
			if(!sampleColor.isValid())
			{
				throw std::invalid_argument("["+FILE_LINE_FUNC_STR+"] The specified color is invalid : " + colorStr.toStdString() );
				result = false;
			}
			else
				d->mMask->setSampleColor(sampleColor);
		}

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
				d->mMask->setBackgroundColor(bgColor);
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
					throw std::invalid_argument("["+FILE_LINE_FUNC_STR+"] "+QString("strip:%1").arg(i).toStdString()+" section have not same reflectances and ISCC–NBS number count.");
					result = false;
				}
			}
		}
		settings.endGroup();

		int j = 0;
		for(QVariant db : reflectanceList)
		{
			d->mPatchesList.append( new ColorSwatchPatch(this, db.toDouble() ) );
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
		result = d->mImgPlg->loadImage(d->mRawFile);

	if(d->mMask)
	{
		if( d->mMask->loadImage() )
		{
			if( d->mImgPlg->size().height() != d->mMask->getImage().size().height()
				||
				d->mImgPlg->size().width() != d->mMask->getImage().size().width()
				)
			{
				d->mMask.reset();
				throw std::length_error("["+FILE_LINE_FUNC_STR+"] Image file and mask image haven't the same size!");
			}
		}
		else
		{
			d->mMask.reset();
			throw std::invalid_argument("["+FILE_LINE_FUNC_STR+"] Mask image cannot be loaded!");
		}
	}

	return result;
}

//---------------------------------------------------------------------

QString ColorSwatch::rawFilePathName() const
{
	return d->mRawFile;
}

QString ColorSwatch::imageFilePathName() const
{
	return d->mImgFile;
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
	for(ColorSwatchPatch* sample : d->mPatchesList)
		ss<<"\tPatch: " << *sample <<"\n";
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
		<<"["<<(colorSwatch.imageFilePathName().isEmpty()?"-":"X")<<"] Raw image:\t"			<<colorSwatch.imageFilePathName().toStdString()<<"\n"
		<<"["<<(colorSwatch.rawFilePathName().isEmpty()?"-":"X")	<<"] Other format image:\t"	<<colorSwatch.rawFilePathName().toStdString()
		<<colorSwatch.printMaskInfo().toStdString()
		<<colorSwatch.printPatchesInfo().toStdString();
}

//---------------------------------------------------------------------

void ColorSwatch::fillPatchesPixelsFromMask()
{

}

//---------------------------------------------------------------------