#pragma once

#include <QString>
#include <QImage>

#include <vector>
#include <utility>

class ImagePlugin
{
protected:
	/// it's up to the sub class to internaly activate this option
	QString mColorSpace;

public:
	typedef std::pair<int,int>		pixelCoord;
	typedef std::vector<pixelCoord> pixelsCoords;
	static	pixelCoord makePixelCoord(int x, int y){return std::make_pair(x, y);}

public:
	bool	withColorSpaceHandler()	{return mColorSpace.isEmpty() ? false : true;}
	QString	colorSpace()			{return mColorSpace;}
	bool	toColorSpace(QString colorSpaceName) {mColorSpace=colorSpaceName; return colorSpaceConversion();}

protected:
	/// need to be overloaded by subclass supporting colorspace and should use mColorSpace
	virtual bool colorSpaceConversion() {return false;}

public:
    /// Use as filter menu on Open (example: 'Image (*.png *.jpg *.bmp)')
    virtual QString getImageFilterExtensions() = 0;
    
	/// Load the image in memory for next use
	virtual bool	loadImage(QString filename) = 0;

	/// Get a conversion to a QImage (allow to show something into the GUI)
	virtual QImage	toQImage() = 0;

	/// Get the image resolution
	virtual QSize	size() = 0;

	virtual float readSinglePixelChannel(int x, int y, int channel) = 0;

	/// Get the averages pixel channels given a list of pixel coord x,y
	virtual bool averagesChannels(pixelsCoords pixCoords, float &r, float &g, float &b, float &a) = 0;

	/// Try to write an output filename from the opened/loaded image (based on the file extension) 
	virtual bool save(QString filename) = 0;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------

class ImagePluginQt : public ImagePlugin
{
public:
	ImagePluginQt();
	virtual ~ImagePluginQt();

public:
    /// create filter string for all formats supported by QImage
    virtual QString getImageFilterExtensions();
	virtual bool	loadImage(QString filename);
	virtual QImage	toQImage();
	virtual QSize	size();
	virtual float	readSinglePixelChannel(int x, int y, int channel);
	virtual bool	averagesChannels(pixelsCoords pixCoords, float &r, float &g, float &b, float &a);
	virtual bool	save(QString filename);

private:
    class Private;
    Private *d;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------

class ImagePluginOIIO : public ImagePlugin
{
public:
	ImagePluginOIIO();
	virtual ~ImagePluginOIIO();

protected:
	virtual bool colorSpaceConversion();

public:
    /// create filter string for all formats supported by QImage
	virtual QString getImageFilterExtensions();
	virtual bool	loadImage(QString filename);
	virtual QImage	toQImage();
	virtual QSize	size();
	virtual float	readSinglePixelChannel(int x, int y, int channel);
	virtual bool	averagesChannels(pixelsCoords pixCoords, float &r, float &g, float &b, float &a);
	virtual bool	save(QString filename);

private:
    class Private;
    Private *d;
};