#include "ColorSwatchPatch.h"
#include "ColorSwatch.h"

#include <iostream>

class ColorSwatchPatch::Private
{
public:
	double			mReflectance;
	MunsellColor	mMunsellColor;
	ColorSwatch*	mColorSwatch;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------

ColorSwatchPatch::ColorSwatchPatch(ColorSwatch* colorSwatch, double reflectance) : d(new Private)
{
	d->mColorSwatch = colorSwatch;
	d->mReflectance = reflectance;
	std::cout<<"d->mReflectance = "<<d->mReflectance<<std::endl;
}

ColorSwatchPatch::~ColorSwatchPatch()
{
	delete d;
}

//---------------------------------------------------------------------

void ColorSwatchPatch::setMunsellColor(MunsellColor munsellColor)
{
	d->mMunsellColor = munsellColor;
	std::cout<<"d->mMunsellColor = "<<d->mMunsellColor<<std::endl;
}

//---------------------------------------------------------------------