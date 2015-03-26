#pragma once
#include <QString>
#include <QImage>
#include <iostream>

class ImagePlugin;

class ColorSwatch
{
public:
	ColorSwatch(ImagePlugin* imgPlg);
	virtual ~ColorSwatch();

public:
	typedef QVector<double> GraphData1D;
	typedef QPair<GraphData1D,GraphData1D> GraphData2D;
	enum class DATA {REF, R, G, B, A};

public:
	/// return true or otherwise false and may throw an exception
	/// some predefined parameters/options are loaded here
	bool loadSettings(QString iniFile);

	/// return true or otherwise false and may throw an exception
	/// it not only load the image mask filename but also try to
	/// apply (overlay) it/on the raw image to get filled mask
	bool loadImages();

	/// when mask image loaded, try to extract patches samples (pixel origin, pixels content, channels averages)
	/// from defined (or autodetected background) and fill colorSwatchPatch data structure
	/// (averages are only used to get the right patches order but since it is computed by Qt, we don't fill it into colorSwatchPatch)
	bool fillPatchesPixelsFromMask();

	///
	GraphData2D getGraphData(DATA datalist);

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