#pragma once

#include <QColor>
#include <QImage>
#include <QString>

#include <iostream>

class ColorSwatchMask
{
public:
	ColorSwatchMask();
	ColorSwatchMask(QString filePathName);
	virtual ~ColorSwatchMask();

public:
	void setImageFilePathName	(const QString	&filePathName);
	void setBackgroundColor		(const QColor	&color);

	QString getImageFilePathName()	const;
	bool	haveBackgroundColor()	const;
	QColor	getBackgroundColor()	const;
	QImage	getImage()				const;

public:
	bool	loadImage();
	bool	applyMask(QImage* srcImg);

public:
	friend std::ostream& operator<<(std::ostream& stream, const ColorSwatchMask &mask);

private:
	class Private;
	Private *d;
};