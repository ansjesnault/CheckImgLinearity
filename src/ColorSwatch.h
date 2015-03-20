#pragma once
#include <QString>
#include <QImage>

class ImagePlugin;

class ColorSwatch
{
public:
	ColorSwatch(ImagePlugin* imgPlg);
	virtual ~ColorSwatch();

public:
	/// return true or otherwise throw an exception
	bool	loadSettings(QString iniFile);

	/// return true or otherwise throw an exception
	bool	loadImages();

public:
	QString imageFilePathName()		const;
	QString imageMaskFilePathName()	const;
	bool	haveImage()				const;
	bool	haveMask()				const;
	QImage	getMaskImg()			const;
	QImage	getQImage()				const;
	QString printPatchesInfo()		const;

public:
	friend	std::ostream& operator<<(std::ostream& stream, const ColorSwatch &colorSwatch);

private:
	class Private;
	Private* d;
};