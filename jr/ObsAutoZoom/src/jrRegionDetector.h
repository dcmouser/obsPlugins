//---------------------------------------------------------------------------
// see https://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.58.3213&rep=rep1&type=pdf
//---------------------------------------------------------------------------


#pragma once

//---------------------------------------------------------------------------
// obs
#include <obs-module.h>
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// defines for region connected component labeling algorithm
#define DefPixelRgbaType								uint32_t
#define DefPixelEnumType								unsigned char
#define DefLabelType									short

#define DefMaxRegionsStore								500
#define DefMaxRegionsAbort								450

#define DefAbortOnTooManyForegroundPixels				true
#define DefMaxForegroundPixelsPercentAbort				0.33f
//
#define DefMaxLoopsBeforeEmergencyAbort					2000
//#define DefMaxLoopsBeforeEmergencyAbort				200
//#define DefMaxLoopsBeforeEmergencyAbort				20000
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// pixel rgba to enum colors
#define DefRdPixelRgbaColorBackground		0x00000000
#define DefRdPixelRgbaColor2				0xFF0000FF
#define DefRdPixelRgbaColor1				0xFF00FF00
#define DefRdPixelRgbaColor3				0xFFFFFFFF
//
#define DefRdPixelEnumColorBackground		0  
#define DefRdPixelEnumColor1				1
#define DefRdPixelEnumColor2				2
#define DefRdPixelEnumColor3				3
#define DefRdPixelEnumColorUnknown			4
#define DefRdPixelEnumMax					5
// meaning of label data
#define DefRegLabelUnset					0
#define DefRegLabelMarked					-1
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
#define regiondebug(format, ...) mydebug(format, ##__VA_ARGS__)
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
class JrRegionSummary {
public:
	// a region is defined by a bounding box and a count of how many pixels were found inside it
	int x1, y1, x2, y2;
	float density;
	unsigned long area;
	float aspect;
	int label;
	bool valid;
	//
	unsigned long pixelCountAll;
	unsigned long pixelCountBorder;
	unsigned long pixelCountInterior;
	unsigned long pixelCountInteriorForeground;
	unsigned long pixelCountPerEnumColor[DefRdPixelEnumMax];
	unsigned long pixelCountBorderPerEnumColor[DefRdPixelEnumMax];
	unsigned long pixelCountInteriorPerEnumColor[DefRdPixelEnumMax];
	unsigned long pixelCountHoleBoundingBoxes;
public:
	void init();
};




class JrRegionDetector {
public:
	JrRegionSummary regions[DefMaxRegionsStore];
	//
	int width, height;
	DefPixelEnumType* pixels;
	DefLabelType* labels;
	DefLabelType* exteriorLabels;
	DefLabelType currentLabel;
	int foundRegions;
	int foundRegionsValid;
	unsigned long maxLoopCountBeforeAbort;

public:
	void rdInit();
	void rdFree();
	//
	void rdResize(int rwidth, int rheight);
	int fillFromStagingMemory(DefPixelRgbaType* internalObsRgbAData, uint32_t linesize);
	//
	JrRegionSummary* getRegionpByIndex(int i) { return &regions[i]; }
	//
	int doConnectedComponentLabeling();
	//
	void initializeRegionData(DefLabelType labelIndex);
	void updateRegionDataOnForegroundPixel(DefLabelType labelIndex, int x, int y, bool pixelIsOnBorder, DefPixelEnumType pixelEnum);
	void postProcessRegionData();
	//
	bool doContourTrace(uint8_t external, DefLabelType currentLabel, int16_t x, int16_t y);
	//
	inline int CompPointOffset(int x, int y) { return width * y + x; }
	inline int CompRgbaByteOffset(int x, int y, int linesize) { return y * linesize + x * 4; };
//	int CompPointOffset(int x, int y);
//	int CompRgbaByteOffset(int x, int y, int linesize);
public:
	void calibrateScanRegion(JrRegionSummary* regionp, unsigned long &colorSumR, unsigned long &colorSumG, unsigned long &colorSumB, unsigned long& pixelCount,  uint32_t linesize,  uint8_t* internalObsRgbAData);
public:
	void setNothingFound() { foundRegions = foundRegions = 0; };
public:
	void doRenderToInternalMemoryPostProcessing(DefPixelRgbaType* internalObsRgbAData, uint32_t linesize, int optionGapFillSize);
	DefPixelEnumType convertRgbaToPixelColorEnum(DefPixelRgbaType pixelColor);
public:
	bool isPixelEnumConsideredForeground(DefPixelEnumType pixelEnum, bool contextWalkingOutside);
};
//---------------------------------------------------------------------------
