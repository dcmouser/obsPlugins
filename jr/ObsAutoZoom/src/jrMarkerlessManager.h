#pragma once


#include "jrPluginDefs.h"

class JrMarkerlessEntry {
public:
	int sourceIndex;
	float zoomLevel;
	int alignxmod, alignymod;
	int x1, y1, x2, y2;
};



class JrMarkerlessManager {
	JrMarkerlessEntry mentries[DefMaxMarkerlessEntries];
	int entryCount;
public:
	void init() { clearEntries(); };
	void clearEntries() { entryCount = 0; }
	int getEntryCount() { return entryCount; }
	JrMarkerlessEntry* getEntryByIndex(int ix) { return &mentries[ix]; };
	//JrMarkerlessEntry* findFirstEntryMatchingSourceIndex(int index);
	//void computeCoordinatesForEntryWithSource(JrMarkerlessEntry* entryp, int& x1, int& y1, int& x2, int& y2);
public:
	bool parseSettingString(const char* settingBuf);
	bool addMarkerEntry(int sourceNumber, float zoomLevel, const char* alignBuf);
public:
	JrMarkerlessEntry* lookupMarkerlessEntry(int entryStartIndex, int sourceIndexRequired);
};
