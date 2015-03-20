#include "MunsellColor.h"
#include "PreBuildUtil.h"

#include <QStringList>

#include <stdexcept>

class MunsellColor::Private
{
public:
	QString mHue;
	float	mLightnessVal;
	float	mChroma;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------

MunsellColor::MunsellColor(QString hue, float val, float chroma)
	: d(new Private)
{
	d->mChroma		 = chroma;
	d->mHue			 = hue;
	d->mLightnessVal = val;
}

MunsellColor::MunsellColor(QString formatedStr) : d(new Private)
{
	try						{ setFromString(formatedStr); }
	catch(std::exception& e){ std::cerr<<"[Failed construct MunsellColor] "+std::string(e.what())<<std::endl; }
}

MunsellColor::MunsellColor() : d(new Private)
{}

MunsellColor::~MunsellColor()
{
	delete d;
}

//---------------------------------------------------------------------

void MunsellColor::set(const QString &hue, const float &val, const float &chroma)
{
	d->mHue				= hue;
	d->mLightnessVal	= val;
	d->mChroma			= chroma;
}

/// '<Hue> <LightnessVal>/<Chroma>'example : 'N 2.75/'
void MunsellColor::setFromString(QString formatedStr)
{
	QStringList halfDecomposed = formatedStr.split(QRegExp("\\s+")); // spaces separator

	// we have the hue (second string contain both others within '/' separator)
	if(halfDecomposed.size() >= 2)
	{
		d->mHue				= halfDecomposed.first();
		d->mLightnessVal	= halfDecomposed.at(1).split("/").first().toFloat();
		d->mChroma			= halfDecomposed.at(1).split("/").last().toFloat();
	}
	else
	{
		set(QString(), 0, 0);
		throw std::invalid_argument("["+FILE_LINE_FUNC_STR+"] bad string format :"+formatedStr.toStdString());
	}
}

void	MunsellColor::setHue(const QString &hue)		{d->mHue=hue;}
void	MunsellColor::setLightnessVal(const float &val)	{d->mLightnessVal=val;}
void	MunsellColor::setChroma(const float &chroma)	{d->mChroma=chroma;}
QString MunsellColor::getHue()				const		{return d->mHue;}
float	MunsellColor::getLightnessVal()		const		{return d->mLightnessVal;}
float	MunsellColor::getChroma()			const		{return d->mChroma;}
QString	MunsellColor::getFormatedString()	const
{
	return QString("%1 %2/%3").arg(d->mHue).arg(d->mLightnessVal).arg(d->mChroma);
}

//---------------------------------------------------------------------

bool MunsellColor::operator==(const MunsellColor &munColor) const
{
	return munColor.getHue()			== d->mHue
		&& munColor.getLightnessVal()	== d->mLightnessVal
		&& munColor.getChroma()			== d->mChroma
		? true : false;
}

//---------------------------------------------------------------------

std::ostream& operator<<(std::ostream& stream, const MunsellColor &munColor)
{
	return stream<<munColor.getFormatedString().toStdString();
}