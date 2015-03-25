#pragma once

#include <iostream>

class ColorSwatch;
class MunsellColor;
class QImage;

class ColorSwatchPatch
{
public:
	ColorSwatchPatch(ColorSwatch* colorSwatch, double reflectance);
	virtual ~ColorSwatchPatch();

public:
	void			setReflectance(		const double reflectance);
	double			getReflectance()	const;

	void			setMunsellColor(	const MunsellColor* munsellColor);
	MunsellColor*	getMunsellColor()	const;

	void			setImage(QImage* img, int orgPixXrelFromMask, int orgPixYrelFromMask);

public:
	friend std::ostream& operator<<(std::ostream& stream, const ColorSwatchPatch &colorSwatchPatch);

private:
	class Private;
	Private *d;
};