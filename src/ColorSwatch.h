#pragma once
#include <QString>
#include <QImage>
#include <vector>

class ImagePlugin;

class ColorSwatch
{
public:
	ColorSwatch(ImagePlugin* imgPlg);
	virtual ~ColorSwatch();

public:
	enum class MASK_OPERATION {LESS, LESS_EQUAL, GREATER, GREATER_EQUAL};

public:
	/// return true or otherwise throw an exception
	bool	loadSettings(QString iniFile);

	/// return true or otherwise throw an exception
	bool	loadImages();

	/// 
	void	fillPatchesPixelsFromMask();

public:
	QString rawFilePathName()		const;
	QString imageFilePathName()		const;
	bool	haveImage()				const;
	bool	haveMask()				const;
	QImage	getMaskImg()			const;
	QImage	getQImage()				const;
	QString printPatchesInfo()		const;
	QString printMaskInfo()			const;

public:
	friend	std::ostream& operator<<(std::ostream& stream, const ColorSwatch &colorSwatch);

private:
	class Private;
	Private* d;
};