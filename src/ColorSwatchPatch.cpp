#include "ColorSwatchPatch.h"

#include "MunsellColor.h"
#include "ImagePlugin.h"
#include "PreBuildUtil.h"

#include <QImage>
#include <QColor>

#include <iostream>
#include <memory>
#include <sstream>

class ColorSwatchPatch::Private
{
public:
	double							mReflectance;
	std::unique_ptr<MunsellColor>	mMunsellColor;
	std::unique_ptr<QImage>			mImg;
	QColor							mAverageRGB;
	int								mRelPixXbegin, mRelPixYbegin; ///< pixel coord origin of this patch (upper left) but in the mask/raw image pixel coord system
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------

ColorSwatchPatch::ColorSwatchPatch(double reflectance) : d(new Private)
{
	d->mReflectance		= reflectance;
	d->mRelPixXbegin	= 0;
	d->mRelPixYbegin	= 0;
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

void ColorSwatchPatch::setImage(const QImage* img, const int orgPixXrelFromMask, const int orgPixYrelFromMask)
{
	d->mImg.reset(new QImage(*img));
	d->mRelPixXbegin = orgPixXrelFromMask;
	d->mRelPixYbegin = orgPixYrelFromMask;
}

//---------------------------------------------------------------------

bool ColorSwatchPatch::computeAverageRGBpixel(ImagePlugin* imgPlg)
{
	if(d->mImg == nullptr && d->mRelPixXbegin == 0 && d->mRelPixYbegin == 0)
	{
		std::cerr<<"["<<FILE_LINE_FUNC_STR<<"] ERROR: cannot continue without a valid patch img and orgPixRelFromMask!"<<std::endl;
		return false;
	}

	// compute valid pixels coords of this patch relative from mask image (we assume raw img and mask have the same size)
	ImagePlugin::pixelsCoords pixCoords;
	for( int row = 0; row < d->mImg->height(); row++ )
		for( int col = 0; col < d->mImg->width(); col++ )
			if( d->mImg->valid(col, row) )
				pixCoords.push_back(ImagePlugin::makePixelCoord(d->mRelPixXbegin + col, d->mRelPixYbegin + row) );

	float r=0.0f, g=0.0f, b=0.0f, a=0.0f;
	bool result = imgPlg->averagesChannels(pixCoords, r, g, b, a);
	d->mAverageRGB.setRedF(r);
	d->mAverageRGB.setGreenF(g);
	d->mAverageRGB.setBlueF(b);
	d->mAverageRGB.setAlphaF(a);
	return result;
}

//---------------------------------------------------------------------

bool ColorSwatchPatch::haveAverageColor() const
{
	return d->mAverageRGB.isValid();
}

//---------------------------------------------------------------------

QColor ColorSwatchPatch::getAverageColor() const
{
	return d->mAverageRGB;
}

//---------------------------------------------------------------------

QString ColorSwatchPatch::printPatcheImgInfo() const
{
	std::stringstream ss;
	ss<<"[img="<<(d->mImg ? (d->mImg->isNull()?"NO":"LOADED") : "NO")
		<<" relPixCoord("<<d->mRelPixXbegin<<","<<d->mRelPixYbegin<<")"
		<<" avRGB("<<(haveAverageColor()?d->mAverageRGB.red():0)<<","
					<<(haveAverageColor()?d->mAverageRGB.green():0)<<","
					<<(haveAverageColor()?d->mAverageRGB.blue():0)<<","
					<<(haveAverageColor()?d->mAverageRGB.alpha():0)
		<<")]";
	return QString(ss.str().c_str());
}

//---------------------------------------------------------------------

std::ostream& operator<<(std::ostream& stream, const ColorSwatchPatch &colorSwatchPatch)
{
	QString dbStr = QString("%1").arg(colorSwatchPatch.getReflectance(), 0, 'f', 2);
	return stream	<<"[Reflec="<<dbStr.toStdString()<<" Clr="<<*colorSwatchPatch.getMunsellColor()<<"]\n"
					<<"\t\t"<<colorSwatchPatch.printPatcheImgInfo().toStdString();
}