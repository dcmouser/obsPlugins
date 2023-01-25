#include "jrcolorhelper.hpp"
#include "winklerrgbhsv.hpp"






//---------------------------------------------------------------------------
uint32_t hueShiftColor(uint32_t color, int hueShift) {
	// see http://beesbuzz.biz/code/16-hsv-color-transforms
	// this doesnt seem to work quite right, but im running out of patience and it doesnt really matter it's just used to inject some color changes
	if (hueShift == 0) {
		return color;
	}

	// TEST -- this should leave color unchanged after shift
	if (false) {
		hueShift = 0;
	}

	// see https://stackoverflow.com/questions/61506398/how-to-unpack-uint32-t-colour-in-c

	// these do NOT work
	/*
	BYTE alpha = (color >> 24) & 0xff; // alpha?
	BYTE red = (color >> 16) & 0xff; // red
	BYTE green = (color >> 8) & 0xff; // green
	BYTE blue = color  & 0xff; // blue
	*/
	/*
	BYTE red = (color   & 0xFF000000) >> 8;
	BYTE green = (color & 0x00FF0000) >> 16;
	BYTE blue = (color  & 0x0000FF00) >> 24;
	*/

	// works
	BYTE red = GetRValue(color);
	BYTE green = GetGValue(color);
	BYTE blue = GetBValue(color);

	// convert rgb to hsv
	float fR = (float)red / 255.0f;
	float fG = (float)green / 255.0f;
	float fB = (float)blue / 255.0f;
	//
	float fH, fS, fV;
	RGBtoHSV(fR, fG, fB, fH, fS, fV);

	// now shift hue (which is from 0 to 360)
	float fHShifted = fH + hueShift;
	while (fHShifted < 0.0f) {
		fHShifted += 360.0f;
	}
	while (fHShifted > 360.0f) {
		fHShifted -= 360.0f;
	}

	// now from hsv BACK to rgb
	HSVtoRGB(fR, fG, fB, fHShifted, fS, fV);
	red = (BYTE)(fR * 255.0f);
	green = (BYTE)(fG * 255.0f);
	blue = (BYTE)(fB * 255.0f);

	return RGB(red, green, blue);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
float rot360(float val) {
	while (val < 0.0f) {
		val += 360.0f;
	}
	while (val > 360.0f) {
		val -= 360.0f;
	}
	return val;
}

float clamp01(float val) {
	while (val < 0.0f) {
		val = 0.0f;
	}
	while (val > 1.0f) {
		val = 1.0f;
	}
	return val;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
uint32_t hsvShiftColor(uint32_t color, int hueShift, int saturationShift, int valueShift) {
	// see http://beesbuzz.biz/code/16-hsv-color-transforms
	// this doesnt seem to work quite right, but im running out of patience and it doesnt really matter it's just used to inject some color changes
	if (hueShift == 0) {
		return color;
	}

	// TEST -- this should leave color unchanged after shift
	if (false) {
		hueShift = 0;
	}

	// see https://stackoverflow.com/questions/61506398/how-to-unpack-uint32-t-colour-in-c

	// these do NOT work
	/*
	BYTE alpha = (color >> 24) & 0xff; // alpha?
	BYTE red = (color >> 16) & 0xff; // red
	BYTE green = (color >> 8) & 0xff; // green
	BYTE blue = color  & 0xff; // blue
	*/
	/*
	BYTE red = (color   & 0xFF000000) >> 8;
	BYTE green = (color & 0x00FF0000) >> 16;
	BYTE blue = (color  & 0x0000FF00) >> 24;
	*/

	// works
	BYTE red = GetRValue(color);
	BYTE green = GetGValue(color);
	BYTE blue = GetBValue(color);

	// convert rgb to hsv
	float fR = (float)red / 255.0f;
	float fG = (float)green / 255.0f;
	float fB = (float)blue / 255.0f;
	//
	float fH, fS, fV;
	RGBtoHSV(fR, fG, fB, fH, fS, fV);

	// now shift hue (which is from 0 to 360)
	float fHShifted = rot360(fH + hueShift);
	float fSShifted = clamp01(fS + (float)saturationShift/360.0f);
	float fVShifted = clamp01(fV + (float)valueShift/360.0f);

	// now from hsv BACK to rgb
	HSVtoRGB(fR, fG, fB, fHShifted, fSShifted, fVShifted);
	red = (BYTE)(fR * 255.0f);
	green = (BYTE)(fG * 255.0f);
	blue = (BYTE)(fB * 255.0f);

	return RGB(red, green, blue);
}
//---------------------------------------------------------------------------





























//---------------------------------------------------------------------------
uint32_t hueShiftColor_v1_old(uint32_t color, int hueShift) {
	// see http://beesbuzz.biz/code/16-hsv-color-transforms
	// this doesnt seem to work quite right, but im running out of patience and it doesnt really matter it's just used to inject some color changes
	if (hueShift == 0) {
		return color;
	}

	float H = (float)hueShift;// / 180.0f;
	float U = (float)cos(H*M_PI/180);
	float W = (float)sin(H*M_PI/180);

//	BYTE red = (color   & 0xFF000000) >> 8;
//	BYTE green = (color & 0x00FF0000) >> 16;
//	BYTE blue = (color  & 0x0000FF00) >> 24;
	BYTE red =   (color & 0xFF000000) >> 24;
	BYTE green = (color & 0x00FF0000) >> 16;
	BYTE blue =  (color & 0x0000FF00) >> 8;

	double redOut = (.299+.701*U+.168*W)*red
	+ (.587-.587*U+.330*W)*green
	+ (.114-.114*U-.497*W)*blue;
	double greenOut = (.299-.299*U-.328*W)*red
	+ (.587+.413*U+.035*W)*green
	+ (.114-.114*U+.292*W)*blue;
	double blueOut = (.299-.3*U+1.25*W)*red
	+ (.587-.588*U-1.05*W)*green
	+ (.114+.886*U-.203*W)*blue;

	//uint32_t colorOut = RGB(redOut, greenOut, blueOut);
	uint32_t colorOut = RGB(blueOut, greenOut, redOut);
	//blog(LOG_WARNING, "Asked hue shift from %d to %d (with %d)", color, colorOut, hueShift);
	return colorOut;
}
//---------------------------------------------------------------------------


