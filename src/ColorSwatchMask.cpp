#include "ColorSwatchMask.h"

#include <QPainter>

#include <memory>

class ColorSwatchMask::Private
{
public:
	Private() : mImgMaskFile("mask.png")
	{}

	QString					mImgMaskFile;
	std::shared_ptr<QImage>	mImgMask;
	QColor					mBgColor;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------

ColorSwatchMask::ColorSwatchMask() : d(new Private)
{
	d->mImgMask = std::make_shared<QImage>();
}

ColorSwatchMask::ColorSwatchMask(QString filePathName) : d(new Private)
{
	d->mImgMask = std::make_shared<QImage>();
	d->mImgMaskFile = filePathName;
}

ColorSwatchMask::~ColorSwatchMask()
{
	delete d;
}

//---------------------------------------------------------------------

bool ColorSwatchMask::loadImage()
{
	if( !d->mImgMaskFile.isEmpty() )
	{
		d->mImgMask.reset(new QImage(d->mImgMaskFile) );
		if(d->mImgMask->format() == QImage::Format_Indexed8)
			d->mImgMask.reset( new QImage(d->mImgMask->convertToFormat(QImage::Format_ARGB32) ) );
	}

	/*Uncomment code to save/write image mask converted to ARGB32*/
	//QString maskFile = QString("mask_argb32.png");
	//d->mImgMask->save(maskFile);
	//std::cout<<"Save "<<maskFile.toStdString()<<" ["<<d->mImgMask->width()<<"x"<<d->mImgMask->height()<<"]"<<std::endl;
	
	return d->mImgMask->isNull() ? false : true;
}

//---------------------------------------------------------------------

bool ColorSwatchMask::applyMask(QImage* srcImg)
{
	bool result = false;
	if(d->mImgMask->isNull())
		if(!loadImage())
			return false;

	QImage		maskDest = d->mImgMask->format() == QImage::Format_Indexed8 ? d->mImgMask->convertToFormat(QImage::Format_ARGB32) : d->mImgMask->copy();
	QPainter	painter; // QPainter will not work with Format_Indexed8
	if( painter.begin(&maskDest) )
	{
		result = true;
		painter.setCompositionMode(QPainter::CompositionMode_DestinationAtop); //QPainter::CompositionMode_DestinationOver
		painter.drawImage(0, 0, *srcImg);
	}
	painter.end();
	d->mImgMask.reset( new QImage(maskDest) );

	/*Uncomment code to save/write image with alpha mask filled with patch color from raw image*/
	//QString maskFile = QString("mask_applied.png");
	//d->mImgMask->save(maskFile);
	//std::cout<<"Save "<<maskFile.toStdString()<<" ["<<d->mImgMask->width()<<"x"<<d->mImgMask->height()<<"]"<<std::endl;

	return result;
}

//---------------------------------------------------------------------

void ColorSwatchMask::setImageFilePathName	(const QString	&filePathName)	{d->mImgMaskFile = filePathName;}
void ColorSwatchMask::setBackgroundColor	(const QColor	&color)			{d->mBgColor	 = color;}

QImage	ColorSwatchMask::getImage()				const {return d->mImgMask?*d->mImgMask.get():QImage();}
QString ColorSwatchMask::getImageFilePathName()	const {return d->mImgMaskFile;}
bool	ColorSwatchMask::haveBackgroundColor()  const {return d->mBgColor.isValid();}
QColor	ColorSwatchMask::getBackgroundColor()	const {return d->mBgColor.isValid() ? d->mBgColor : QColor(Qt::black);}

//---------------------------------------------------------------------

std::ostream& operator<<(std::ostream& stream, const ColorSwatchMask &mask)
{
	return stream<<"\t["<<(mask.getImageFilePathName().isEmpty()?"-":"X")<<"] Mask image:\t"<<mask.getImageFilePathName().toStdString()<<"\n"
		<<"\t["<<(mask.getImage().isNull()?"-] NOT":"X]")<<" loaded\n"
		<<"\t["<<(mask.getBackgroundColor().isValid()?"X":"-")<<"] Background color:\t("
			<<mask.getBackgroundColor().redF()	<<"; "
			<<mask.getBackgroundColor().greenF()<<"; "
			<<mask.getBackgroundColor().blueF()	<<"; "
			<<mask.getBackgroundColor().alphaF()<<")\n";
}
