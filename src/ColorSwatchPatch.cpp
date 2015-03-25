#include "ColorSwatchPatch.h"

#include "ColorSwatch.h"
#include "MunsellColor.h"

#include <iostream>
#include <memory>

#include <QImage>

class ColorSwatchPatch::Private
{
public:
	double							mReflectance;
	std::unique_ptr<MunsellColor>	mMunsellColor;
	std::unique_ptr<ColorSwatch>	mColorSwatch;
	std::unique_ptr<QImage>			mImg;
	QRgb							mAverageRGB;
	int								mRelPixXbegin, mRelPixYbegin; ///< pixel coord of origin of this patch (upper right) but in the image pixel coord system
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------

ColorSwatchPatch::ColorSwatchPatch(ColorSwatch* colorSwatch, double reflectance) : d(new Private)
{
	d->mColorSwatch.reset(colorSwatch);
	d->mReflectance = reflectance;
}

ColorSwatchPatch::~ColorSwatchPatch()
{
	delete d;
}

//---------------------------------------------------------------------

void ColorSwatchPatch::setReflectance(double reflectance)
{
	d->mReflectance = reflectance;
}

double ColorSwatchPatch::getReflectance() const
{
	return d->mReflectance;
}

void ColorSwatchPatch::setMunsellColor(const MunsellColor* munsellColor)
{
	d->mMunsellColor.reset(const_cast<MunsellColor*>(munsellColor));
}

MunsellColor* ColorSwatchPatch::getMunsellColor() const
{
	return d->mMunsellColor.get();
}

void ColorSwatchPatch::setImage(QImage* img, int orgPixXrelFromMask, int orgPixYrelFromMask)
{
	d->mImg.reset(new QImage(*img));
	d->mRelPixXbegin = orgPixXrelFromMask;
	d->mRelPixYbegin = orgPixYrelFromMask;
}

//---------------------------------------------------------------------

std::ostream& operator<<(std::ostream& stream, const ColorSwatchPatch &colorSwatchPatch)
{
	QString dbStr = QString("%1").arg(colorSwatchPatch.getReflectance(), 0, 'f', 2);
	return stream	<<"[Reflectance="<<dbStr.toStdString()<<"]\t"
					<<"[MunsellColor="<<*colorSwatchPatch.getMunsellColor()<<"]";
}