//---------------------------------------------------------------------------
#include <cstdio>
//
#include "jrPlugin.h"
//
#include "../../jrcommon/src/jrhelpers.hpp"
#include "../../jrcommon/src/jrobshelpers.hpp"
//---------------------------------------------------------------------------
















//---------------------------------------------------------------------------
void JrPlugin::parseTextCordsString(const char* coordStrIn, int* x1, int* y1, int* x2, int* y2, int defx1, int defy1, int defx2, int defy2, int maxwidth, int maxheight) {
	char coordStr[80];
	strcpy(coordStr, coordStrIn);

	char* cpos;
	if (cpos = strtok(coordStr, ",x")) {
		*x1 = parseCoordStr(cpos, maxwidth);
	} else {
		*x1 = defx1 >=0 ? defx1 : maxwidth+defx1;
	}
	if (cpos = strtok(NULL, ",x")) {
		*y1 = parseCoordStr(cpos, maxheight);
	} else {
		*y1 = defy1 >=0 ? defy1 : maxheight+defy1;
	}
	if (cpos = strtok(NULL, ",x")) {
		*x2 = parseCoordStr(cpos, maxwidth);
	} else {
		*x2 = defx2 >=0 ? defx2 : maxwidth+defx2;
	}
	if (cpos = strtok(NULL, ",x")) {
		*y2 = parseCoordStr(cpos, maxheight);
	} else {
		*y2 = defy2 >=0 ? defy2 : maxheight+defy2;
	}
}


int JrPlugin::parseCoordStr(const char* cpos, int max) {
	int ival = atoi(cpos);
	if (ival < 0) {
		return max + ival;
	}
	return ival;
}
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------
void JrPlugin::fillFloatListFromString(char* commaList, float* floatList, int max) {
	char buf[80];
	strcpy(buf, commaList);
	char* tokpos = buf;
	char* cpos;
	int i = 0;
	for (; i < max; ++i) {
		if (cpos = strtok(tokpos, ",")) {
			tokpos = NULL;
			floatList[i] = (float)atof(cpos);
		}
		else {
			break;
		}
	}
	for (; i < max; ++i) {
		floatList[i] = 0;
	}
}
//---------------------------------------------------------------------------










