#include "jrMarkerlessManager.h"
#include "jrfuncs.h"
#include "obshelpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <ctype.h>
#include <string.h>



/*
JrMarkerlessEntry* JrMarkerlessManager::findFirstEntryMatchingSourceIndex(int index) {
	return NULL;
}



void JrMarkerlessManager::computeCoordinatesForEntryWithSource(JrMarkerlessEntry* entryp, int& x1, int& y1, int& x2, int& y2) {
	// ATTN: TODO
}
*/


bool JrMarkerlessManager::parseSettingString(const char* settingBuf) {

	// the idea of this function is to parse the user list of markerless entries and find the one currently selected by an option and then UPDATE the markerless coords based on the source dimensions and zoom and outputsize of the current view
	// therefore we must invoke this function whenver a tracked source changes size or when user switches the entry # they want to use for markerless.
	// ATTN:TODO - i think we also need to invoke this whenever we SWITCH to the viewing a new source which could be related to the markerless choice

	// markerless source and coords say what should be shown when no markers are found -- whether this is the full source view or something more zoomed
	// parse the opt_markerlessCycleListBuf, and then fill markerlessCoords based on which opt_markerlessCycleIndex user has selected
	// gracefully handle when index is beyond what is found in the list, just correct and modulus it so it cycles around
	// note that ther markerlessCoords are only valid for the source# specified, so it is up to the plugin to switch to the desired source when moving to chosen markerless source and coords
	// note this only gets called on coming back from options or when hotkey triggers a change, so it can be "slowish"
	// format of opt_markerlessCycleListBuf is "1,0 | 2,0 | 2,1 | 2,2 | ...
	//
	bool foundGoodMarkers = false;
	char markerlessBufTemp[DefMarkerlessCycleListBufMaxSize];
	char alignBuf[80];
	int sourceNumber;
	float zoomLevel;
	bool validSyntax = true;

	clearEntries();
	//mydebug("in markerless parseSettingString: %s.",settingBuf);

	// temp copy so we can use strtok
	strcpy(markerlessBufTemp, settingBuf);
	char entrybuf[255];
	while (jrLeftCharpSplit(markerlessBufTemp, entrybuf, '|')) {
		// got an entry

		// reset defaults
		sourceNumber = 0;
		zoomLevel = 0;
		strcpy(alignBuf, "");

		// now split into comma separated tuples
		char assignBuf[80];
		char varname[80];
		char varval[80];
		while (jrLeftCharpSplit(entrybuf, assignBuf, ',')) {
			// found a tuple now divide by = character
			jrLeftCharpSplit(assignBuf, varname, '=');
			strcpy(varval, assignBuf);
			jrtrim(varname);
			strlwr(varname);
			jrtrim(varval);
			// parse it
			if (strcmp(varname, "") == 0) {
				continue;
			}
			if (strcmp(varname,"s")==0) {
				sourceNumber = atoi(varval);
			} else if (strcmp(varname,"z")==0) {
				zoomLevel = (float)atof(varval);;
			} else if (strcmp(varname,"a")==0) {
				strcpy(alignBuf, varval);
			}
		}
		// add entry
		validSyntax &= addMarkerEntry(sourceNumber, zoomLevel, alignBuf);
	}

	return validSyntax;
}



bool JrMarkerlessManager::addMarkerEntry(int sourceNumber, float zoomLevel, const char* alignBuf) {
	// zoomLevel 0 means at original scale, and preserve aspect ratio -- this makes it ok to have black bars outside so the whole source is visible and fits LONGER dimension
	// loomLevel 1 and greater means preserve aspect ratio and ZOOM and fit (center) the SHORTER dimension, meaning there will never be black bars
	// the output aspect is outputWidth and outputHeight
	// and the source is queried source width and source height
	// gracefully handle case where source is zero sizes

	//mydebug("in setMarkerlessCoordsForSourceNumberAndZoomLevel (%d): sourcenum=%d, zoom=%f align=%s.",entryCount, sourceNumber, zoomLevel, alignBuf);

	if (entryCount >= DefMaxMarkerlessEntries) {
		// too many
		return false;
	}

	JrMarkerlessEntry* entryp = &mentries[entryCount];

	// and lastly record the source number for going markerless
	entryp->sourceIndex = sourceNumber;
	entryp->zoomLevel = zoomLevel;

	//mydebug("setMarkerlessCoordsForSourceNumberAndZoomLevel stage 4");
	if (strcmp(alignBuf, "ul") == 0) {
		entryp->alignxmod = 0;
		entryp->alignymod = 0;
	}
	else if (strcmp(alignBuf, "uc") == 0) {
		entryp->alignxmod = 1;
		entryp->alignymod = 0;
	}
	else if (strcmp(alignBuf, "ur") == 0) {
		entryp->alignxmod = 2;
		entryp->alignymod = 0;
	}
	else if (strcmp(alignBuf, "ml") == 0) {
		entryp->alignxmod = 0;
		entryp->alignymod = 1;
	}
	else if (strcmp(alignBuf, "mc") == 0) {
		entryp->alignxmod = 1;
		entryp->alignymod = 1;
	}
	else if (strcmp(alignBuf, "mr") == 0) {
		entryp->alignxmod = 2;
		entryp->alignymod = 1;
	}
	else if (strcmp(alignBuf, "ll") == 0) {
		entryp->alignxmod = 0;
		entryp->alignymod = 2;
	}
	else if (strcmp(alignBuf, "lc") == 0) {
		entryp->alignxmod = 1;
		entryp->alignymod = 2;
	}
	else if (strcmp(alignBuf, "lr") == 0) {
		entryp->alignxmod = 2;
		entryp->alignymod = 2;
	}
	else {
		entryp->alignxmod = 1;
		entryp->alignymod = 1;
	}

	//mydebug("Added parsed markerless entry %d.", entryCount);

	// increase entry count
	++entryCount;

	return true;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
JrMarkerlessEntry* JrMarkerlessManager::lookupMarkerlessEntry(int entryStartIndex, int sourceIndexRequired) {
	int index = entryStartIndex;
	for (;; ++index) {
		if (index >= entryCount) {
			index = 0;
			if (index >= entryStartIndex) {
				// we cycled around, nothing found
				break;
			}
		}
		if (sourceIndexRequired == -1) {
			// they just want this entry
			return &mentries[index];
		}
		// looking for a specific source #
		if (mentries[index].sourceIndex == sourceIndexRequired) {
			return &mentries[index];
		}
	}
	// not found
	return NULL;
}
//---------------------------------------------------------------------------

