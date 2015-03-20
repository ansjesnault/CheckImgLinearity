#include "ColorSwatch.h"

#include "PreBuildUtil.h"
#include "ColorSwatchPatch.h"
#include "MunsellColor.h"
#include "ImagePlugin.h"

#include <QVector>
#include <QSettings>
#include <QVariant>
#include <QDir>
#include <QFile>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <cstring>


class ColorSwatch::Private
{
public:
	QString						mImgFile;
	QString						mImgMaskFile;
	std::shared_ptr<QImage>		mImgMask;
	QVector<ColorSwatchPatch*>	mPatchesList;
	ImagePlugin*				mImgPlg;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------

ColorSwatch::ColorSwatch(ImagePlugin* imgPlg) : d(new Private)
{
	d->mImgPlg	= imgPlg;
	d->mImgMask = std::make_shared<QImage>();
}

ColorSwatch::~ColorSwatch()
{
	d->mPatchesList.clear();
	delete d;
	std::cout<<"ColorSwatch deleted"<<std::endl;
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
	d->mImgFile	= QString();
	d->mImgMaskFile			= QString();
	settings.beginGroup("files");
	{
		QString colorswatchFile( settings.value("colorswatch").toString() );
		if( !QFile::exists( d->mImgFile = QDir::isRelativePath(colorswatchFile) ? iniFilePath.absoluteFilePath(colorswatchFile) : colorswatchFile ) )
			throw std::invalid_argument("["+FILE_LINE_FUNC_STR+"] The specified file does not exist : " + d->mImgFile.toStdString() );
		else
			result = true;

		if(settings.childKeys().contains("mask"))
		{
			QString maskFile( settings.value("mask").toString() );
			if( !QFile::exists( d->mImgMaskFile = QDir::isRelativePath(maskFile) ? iniFilePath.absoluteFilePath(maskFile) : maskFile ) )
			{
				throw std::invalid_argument("["+FILE_LINE_FUNC_STR+"] The specified file does not exist : " + d->mImgMaskFile.toStdString() );
				result = false;
			}
		}
	}
	settings.endGroup();

	// Load all patches info, strip by strip
	d->mPatchesList.clear();
	QVector<QVariant> reflectanceList;
	QStringList		isccnbsList;
	for(int i = 1; i <= settings.childGroups().filter("strip").size(); i++)
	{
		settings.beginGroup(QString("strip:%1").arg(i));
		{
			reflectanceList = settings.value("reflectances").toList().toVector();
			if(settings.childKeys().contains("ISCCNBS"))
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
				d->mPatchesList.last()->setMunsellColor( new MunsellColor(isccnbsList.at(j++)) );
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
	if( !d->mImgFile.isEmpty() )
		result = d->mImgPlg->loadImage(d->mImgFile);

	if( !d->mImgMaskFile.isEmpty() )
	{
		d->mImgMask.reset(new QImage(d->mImgMaskFile) );
		if( !d->mImgMask->isNull() )
		{
			if( d->mImgPlg->size().height() != d->mImgMask->size().height()
				||
				d->mImgPlg->size().width() != d->mImgMask->size().width()
			  )
			{
				throw std::length_error("["+FILE_LINE_FUNC_STR+"] Image file and mask image haven't the same size!");
				result = false;
			}
		}
		else
		{
			d->mImgMask.reset();
			throw std::invalid_argument("["+FILE_LINE_FUNC_STR+"] Mask image cannot be loaded!");
			result = false;
		}
	}

	return result;
}

//---------------------------------------------------------------------

QString ColorSwatch::imageFilePathName() const
{
	return d->mImgFile;
}

QString ColorSwatch::imageMaskFilePathName() const
{
	return d->mImgMaskFile;
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
	return !d->mImgMask->isNull();
}

QImage ColorSwatch::getMaskImg() const
{
	return *d->mImgMask.get();
}

QString ColorSwatch::printPatchesInfo() const
{
	std::stringstream ss;
	for(ColorSwatchPatch* sample : d->mPatchesList)
		ss<<"Patch:     " << *sample <<"\n";
	return QString(ss.str().c_str());
}

//---------------------------------------------------------------------

std::ostream& operator<<(std::ostream& stream, const ColorSwatch &colorSwatch)
{
	return stream<<"ColorSwatch:\n"
		<<"Image:     "	<<colorSwatch.imageFilePathName().toStdString()<<"\n"
		<<"ImageMask: "	<<colorSwatch.imageMaskFilePathName().toStdString()<<"\n"
		<<colorSwatch.printPatchesInfo().toStdString();
		
}