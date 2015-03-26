#pragma once

#include <QString>
#include <QColor>

#include <iostream>

class MunsellColor;
class QImage;
class ImagePlugin;

class ColorSwatchPatch
{
public:
	ColorSwatchPatch(double reflectance);
	virtual ~ColorSwatchPatch();

public:
	void			setReflectance(		const double reflectance);
	double			getReflectance()	const;

	void			setMunsellColor(	const MunsellColor* munsellColor);
	MunsellColor*	getMunsellColor()	const;

	void			setImage(const QImage* img, const int orgPixXrelFromMask, const int orgPixYrelFromMask);
	QString			printPatcheImgInfo() const;

public:
	bool	computeAverageRGBpixel(ImagePlugin* imgPlg);
	bool	haveAverageColor()	const;
	QColor	getAverageColor()	const;

public:
	friend std::ostream& operator<<(std::ostream& stream, const ColorSwatchPatch &colorSwatchPatch);

private:
	class Private;
	Private *d;
};