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
	void backgroundColor		(const QColor	&color);
	void outputAplliedMask		(const bool		&write);
	void outputPatches			(const bool		&write);
	void apllyAlphaMask			(const bool		&apply);

	QString getImageFilePathName()	const;
	bool	haveBackgroundColor()	const;
	QColor	backgroundColor()		const;
	QImage	getImage()				const;
	bool	outputAplliedMask()		const;
	bool	outputPatches()			const;
	bool	apllyAlphaMask()		const;

public:
	bool	loadImage();
	bool	applyMask(QImage* srcImg);

public:
	friend std::ostream& operator<<(std::ostream& stream, const ColorSwatchMask &mask);

private:
	class Private;
	Private *d;
};