#pragma once

#include "MunsellColor.h"

class ColorSwatch;

class ColorSwatchPatch
{
public:
	ColorSwatchPatch(ColorSwatch* colorSwatch, double reflectance);
	virtual ~ColorSwatchPatch();

public:
	void setMunsellColor(MunsellColor munsellColor);

private:
	class Private;
	Private *d;
};