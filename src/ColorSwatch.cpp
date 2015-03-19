#include "ColorSwatch.h"
#include "ColorSwatchPatch.h"
#include "MunsellColor.h"
#include "ImagePlugin.h"

#include <QVector>
#include <QSettings>
#include <QDir>
#include <QFile>

#include <iostream>
#include <memory>
#include <stdexcept>

class ColorSwatch::Private
{
public:
	QString										mColorswatchImgFile;
	QString										mMaskImgFile;
	std::shared_ptr<QImage>						mImgMask;
	QVector<std::shared_ptr<ColorSwatchPatch>>	mPatchesList;
	ImagePlugin*								mImgPlg;
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
	delete d;
}

//---------------------------------------------------------------------

bool ColorSwatch::loadSettings(QString iniFile)
{
	bool result = false;
	QDir iniFilePath(iniFile); // iniFile is absolute
	iniFilePath.cdUp();
	QSettings settings(iniFile, QSettings::Format::IniFormat);

	// Get files 
	d->mColorswatchImgFile	= QString();
	d->mMaskImgFile			= QString();
	settings.beginGroup("files");
	{
		QString colorswatchFile( settings.value("colorswatch").toString() );
		if( !QFile::exists( d->mColorswatchImgFile = QDir::isRelativePath(colorswatchFile) ? iniFilePath.absoluteFilePath(colorswatchFile) : colorswatchFile ) )
			throw std::invalid_argument("[ColorSwatch::loadSettings] The specified file does not exist : " + d->mColorswatchImgFile.toStdString() );
		else
			result = true;

		if(settings.childKeys().contains("mask"))
		{
			QString maskFile( settings.value("mask").toString() );
			if( !QFile::exists( d->mMaskImgFile = QDir::isRelativePath(maskFile) ? iniFilePath.absoluteFilePath(maskFile) : maskFile ) )
			{
				throw std::invalid_argument("[ColorSwatch::loadSettings] The specified file does not exist : " + d->mMaskImgFile.toStdString() );
				result = false;
			}
		}
	}
	settings.endGroup();

	// Load all patches info, strip by strip
	d->mPatchesList.clear();
	QVector<double> reflectanceList;
	QStringList		isccnbsList;
	for(int i = 1; i <= settings.childGroups().filter("strip").size(); i++)
	{
		settings.beginGroup(QString("strip%1").arg(i));
		{
			reflectanceList = settings.value("reflectances").value<QVector<double> >();
			if(settings.childKeys().contains("ISCCNBS"))
			{
				isccnbsList = settings.value("ISCCNBS").value<QStringList>();
				if(reflectanceList.size() != isccnbsList.size())
				{
					throw std::invalid_argument("[ColorSwatch::loadSettings] "+QString("strip:%1").arg(i).toStdString()+" have not same reflectances and ISCC–NBS number count.");
					result = false;
				}
			}
		}
		settings.endGroup();

		int j = 0;
		for(double db : reflectanceList)
		{
			d->mPatchesList.append(std::make_shared<ColorSwatchPatch>(this, db));
			if( isccnbsList.size()-j > 0 )
				d->mPatchesList.last()->setMunsellColor(MunsellColor(isccnbsList.at(j++)));
			else
			{
				throw std::invalid_argument("[ColorSwatch::loadSettings] The ColorSwatchPatch with reflectance ["+QString("%1").arg(db).toStdString()+"] will not have any MunsellColor.");
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
	if( !d->mColorswatchImgFile.isEmpty() )
		result = d->mImgPlg->loadImage(d->mColorswatchImgFile);

	if( !d->mMaskImgFile.isEmpty() )
		d->mImgMask.reset(new QImage(d->mMaskImgFile) );

	return result;
}

//---------------------------------------------------------------------

bool ColorSwatch::haveImage()
{
	return !d->mImgPlg->toQImage().isNull();
}

//---------------------------------------------------------------------

QImage ColorSwatch::getQImage()
{
	return d->mImgPlg->toQImage();
}

//---------------------------------------------------------------------

bool ColorSwatch::haveMask()
{
	return !d->mImgMask->isNull();
}

//---------------------------------------------------------------------

QImage ColorSwatch::getMask()
{
	return *d->mImgMask.get();
}