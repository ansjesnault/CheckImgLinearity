#pragma once

#include <QString>
#include <iostream>

/// http://en.wikipedia.org/wiki/Munsell_color_system
class MunsellColor
{
public:
	MunsellColor(QString hue, float val, float chroma);
	MunsellColor(QString formatedStr);
	MunsellColor();
	virtual ~MunsellColor();

public:
	/// format: '<Hue> <LightnessVal>/<Chroma>'
	/// example:'N 2.75/'
	void setFromString(QString formatedStr);
	void set(const QString &hue, const float &val, const float &chroma);

	void	setHue(const QString &hue)	;
	void	setLightnessVal(const float &val)	;
	void	setChroma(const float &chroma);

	QString getHue()			const;
	float	getLightnessVal()	const;
	float	getChroma()			const;
	QString	getFormatedString() const;

public:
	bool operator==(const MunsellColor &munColor) const;
	friend std::ostream& operator<<(std::ostream& stream, const MunsellColor &munColor);

private:
	class Private;
	Private* d;
};