//---------------------------------------------------------------------------
#include <cstdio>
//
#include "jrPlugin.h"
#include "jrfuncs.h"
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

















