#pragma once

#include <QString>
#include <QImage>

class ImagePlugin
{
public:    
    /// Use as filter menu on Open (example: 'Image (*.png *.jpg *.bmp)')
    virtual QString getImageFilterExtensions() = 0;

	/// Load the image mask in memory for next use
	virtual bool	loadImageMask(QString filename) = 0;
	virtual bool	isImageMaskLoaded() = 0;
    
	/// Load the image in memory for next use
	virtual bool	loadImage(QString filename) = 0;

	/// Get a conversion to a QImage (allow to show something into the GUI)
	virtual QImage	toQImage() = 0;
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

	virtual bool	loadImageMask(QString filename);
	virtual bool	isImageMaskLoaded();

	virtual bool	loadImage(QString filename);

	virtual QImage	toQImage();

private:
    class Private;
    Private *d;
};