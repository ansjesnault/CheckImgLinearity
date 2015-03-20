#include "ColorSwatchMask.h"

#include <memory>

class ColorSwatchMask::Private
{
public:
	QString						mImgMaskFile;
	std::shared_ptr<QImage>		mImgMask;

	QColor mSampleColor;
	QColor mBgColor;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------

ColorSwatchMask::ColorSwatchMask() : d(new Private)
{
	d->mImgMask = std::make_shared<QImage>();
}

ColorSwatchMask::ColorSwatchMask(QString filePathName) : d(new Private)
{
	//d->mImgMask = std::make_shared<QImage>();
	d->mImgMaskFile = filePathName;
}

ColorSwatchMask::~ColorSwatchMask()
{
	delete d;
}

//---------------------------------------------------------------------

bool ColorSwatchMask::loadImage()
{
	if( !d->mImgMaskFile.isEmpty() )
	{
		d->mImgMask.reset(new QImage(d->mImgMaskFile) );
		if( !d->mImgMask->isNull() )
			return true;
	}
	return false;
}

//---------------------------------------------------------------------

void ColorSwatchMask::setImageFilePathName	(const QString	&filePathName)	{d->mImgMaskFile = filePathName;}
void ColorSwatchMask::setSampleColor		(const QColor	&color)			{d->mSampleColor = color;}
void ColorSwatchMask::setBackgroundColor	(const QColor	&color)			{d->mBgColor	 = color;}

QImage	ColorSwatchMask::getImage()				const {return d->mImgMask?*d->mImgMask.get():QImage();}
QString ColorSwatchMask::getImageFilePathName()	const {return d->mImgMaskFile;}
QColor	ColorSwatchMask::getSampleColor()		const {return d->mSampleColor;}
QColor	ColorSwatchMask::getBackgroundColor()	const {return d->mBgColor;}

//---------------------------------------------------------------------

std::ostream& operator<<(std::ostream& stream, const ColorSwatchMask &mask)
{
	return stream<<"\t["<<(mask.getImageFilePathName().isEmpty()?"-":"X")<<"] Mask image:\t"<<mask.getImageFilePathName().toStdString()<<"\n"
		<<"\t["<<(mask.getImage().isNull()?"-] NOT":"X]")<<" loaded\n"
		<<"\t["<<(mask.getSampleColor().isValid()?"X":"-")<<"] Sample color:\t("
			<<mask.getSampleColor().redF()	<<"; "
			<<mask.getSampleColor().greenF()<<"; "
			<<mask.getSampleColor().blueF()	<<"; "
			<<mask.getSampleColor().alphaF()<<")\n"
		<<"\t["<<(mask.getBackgroundColor().isValid()?"X":"-")<<"] Background color:\t("
			<<mask.getBackgroundColor().redF()	<<"; "
			<<mask.getBackgroundColor().greenF()<<"; "
			<<mask.getBackgroundColor().blueF()	<<"; "
			<<mask.getBackgroundColor().alphaF()<<")\n";
}

