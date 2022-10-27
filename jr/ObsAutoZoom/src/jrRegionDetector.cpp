//---------------------------------------------------------------------------
#include "jrRegionDetector.h"
#include "obsHelpers.h"
#include "jrPluginDefs.h"
//---------------------------------------------------------------------------



/*
int JrRegionDetector::CompPointOffset(int x, int y) {
	if (x < 0 || y<0 || x > width - 1 || y>height - 1) {
		mydebug("ERRORR BAD XY IN REGION B %d x %d ------------------- !!!!!!!!!!!!!!!!!!!!!!!!!!!",x,y);
	}
	return width * y + x;
}
int JrRegionDetector::CompRgbaByteOffset(int x, int y, int linesize) {
	if (x < 0 || y<0 || x > width-1 || y>height - 1) {
		mydebug("ERRORR BAD XY IN REGION A %d x %d ------------------- !!!!!!!!!!!!!!!!!!!!!!!!!!!",x,y);
	}
	return y * linesize + x * 4;
};
*/









//---------------------------------------------------------------------------
void JrRegionSummary::init() {
    pixelCountAll = 0;
    pixelCountBorder = 0;
	pixelCountInterior = 0;
	pixelCountInteriorForeground = 0;
    for (int i = 0; i < DefRdPixelEnumMax; ++i) {
        pixelCountPerEnumColor[i]=0;
        pixelCountBorderPerEnumColor[i]=0;
        pixelCountInteriorPerEnumColor[i]=0;
    }
    pixelCountHoleBoundingBoxes = 0;
}
//---------------------------------------------------------------------------


















//---------------------------------------------------------------------------
void JrRegionDetector::rdInit() {
	pixels = NULL;
	labels = NULL;
    exteriorLabels = NULL;
	width = 0;
	height = 0;
    foundRegions = 0;
    foundRegionsValid = 0;
}

void JrRegionDetector::rdFree() {
	if (pixels) {
		bfree(pixels);
		pixels = NULL;
	}
	if (labels) {
		bfree(labels);
		labels = NULL;
	}
	if (exteriorLabels) {
		bfree(exteriorLabels);
		exteriorLabels = NULL;
	}
}



void JrRegionDetector::rdResize(int rwidth, int rheight) {
	// free existing pixel data
	rdFree();

	width = rwidth;
	height = rheight;

	// allocate enough space for border of pixels around, for easier neighborhood checking

    //int pixelcount = (width * height);
    // test
    unsigned long pixelsDataSize = (width+2) * (height+2);

    pixels = (unsigned char*) bzalloc(sizeof(DefPixelEnumType) * pixelsDataSize);
	labels = (short*) bzalloc(sizeof(DefLabelType) * pixelsDataSize);
	exteriorLabels = (short*) bzalloc(sizeof(DefLabelType) * pixelsDataSize);

    //
    foundRegions = 0;
    foundRegionsValid = 0;

    //mydebug("Allocating space for (%dx%d) %lu maxloops = %lu.", width, height, pixelCount, maxLoopCountBeforeAbort);
    
}
//---------------------------------------------------------------------------























//---------------------------------------------------------------------------
DefPixelEnumType JrRegionDetector::convertRgbaToPixelColorEnum(DefPixelRgbaType pixelColor) {

    // convert from rgba pixel colors output by our effect processor and preprocessing to an 8 bit simple color id#
    if (pixelColor == DefRdPixelRgbaColorBackground) {
        return DefRdPixelEnumColorBackground;
    }
    if (pixelColor == DefRdPixelRgbaColor1) {
        return DefRdPixelEnumColor1;
    }
    if (pixelColor == DefRdPixelRgbaColor2) {
        return DefRdPixelEnumColor2;
    }
    if (pixelColor == DefRdPixelRgbaColor3) {
        return DefRdPixelEnumColor3;
    }

    // unknown error
    return DefRdPixelEnumColorUnknown;
}



int JrRegionDetector::fillFromStagingMemory(DefPixelRgbaType* internalObsRgbAData, uint32_t linesize) {
	// ATTN: later optimize this for speed
    DefPixelEnumType pixelEnumColor;
    unsigned int foregroundPixelCount = 0;
	//
	// pixelThreshold test
	//uint8_t pixelThreshold = 254;
	uint8_t pixelThreshold = 254;

    unsigned long maxLoopPixelsBeforeAbort = (unsigned long)((float)width * (float) height * DefMaxForegroundPixelsPercentAbort);

	// we pad a border around the entire grid
	//
    int pixelsDataSize = width * height;
	//
	// fill our internal pixel grid
	for (int iy = 0; iy < height; ++iy) {
        for (int ix = 0; ix < width; ++ix) {
            pixelEnumColor = convertRgbaToPixelColorEnum(internalObsRgbAData[CompPointOffset(ix, iy)]);
            // set it to 1 if its our marker or 0 otherwise
            pixels[CompPointOffset(ix,iy)] = pixelEnumColor;
            // debug
            if (DefAbortOnTooManyForegroundPixels) {
                //if (pixelEnumColor != DefRdPixelEnumColorBackground) {
                if (pixelEnumColor == DefRdPixelEnumColor1) {
                    ++foregroundPixelCount;
                    if (foregroundPixelCount > maxLoopPixelsBeforeAbort && maxLoopPixelsBeforeAbort > 0) {
                        iy = height;
                        ix = width;
                        //mydebug("Aborting fillFromStagingMemory because of too many foreground pixels (%d vs %d).", foregroundPixelCount, maxLoopPixelsBeforeAbort);
                        break;
                    }
                }
            }
		}
	}

	// reset labels all to Unset
	memset(labels, DefRegLabelUnset, pixelsDataSize * sizeof(DefLabelType));
    memset(exteriorLabels, DefRegLabelUnset, pixelsDataSize * sizeof(DefLabelType));

    // return number of foreground pixels found, for debugging
    return foregroundPixelCount;
}
//---------------------------------------------------------------------------

































//---------------------------------------------------------------------------
// see https://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.58.3213&rep=rep1&type=pdf
// see https://github.com/BlockoS/blob

/* Extract blob contour (external or internal). */
bool JrRegionDetector::doContourTrace(uint8_t external, DefLabelType currentLabel, int16_t x, int16_t y) {
    int16_t dx[8] = { 1, 1, 0,-1,-1,-1, 0, 1 };
    int16_t dy[8] = { 0, 1, 1, 1, 0,-1,-1,-1 };

    if (currentLabel <= 0) {
        mydebug("ERROR!!!!!!!!!!!! doContourTrace with currentLabel below or equal to 0??.");
        return false;
    }

    //regiondebug("ATTN: IN DoContourTrace %d at location %d,%d type %d.", currentLabel, x,y, external);


    // ATTN: NEW TEST via paper
    // !!!!! 8/26/22 this seems to fix it(!)
    int i;
    if (DefUseJesseCounterTraceFix) {
        i = external ? 0 : 1;
    }
    else {
        i = external ? 7 : 3;
    }


    int j;

    int16_t x0 = x;
    int16_t y0 = y;

    int16_t xx = -1;
    int16_t yy = -1;

    int done = 0;

    // bug debug
    int loopcount = 0;

    // assign label this point
    labels[CompPointOffset(x0, y0)] = currentLabel;
    DefPixelEnumType pixelEnum = pixels[CompPointOffset(x0, y0)];
    //
    updateRegionDataOnForegroundPixel(currentLabel, x0, y0, external, pixelEnum);

    // bounding box
    int bbxmin = x0;
    int bbymin = y0;
    int bbxmax = x0;
    int bbymax = y0;

    while (!done)
    {
        if (++loopcount > DefMaxLoopsBeforeEmergencyAbort && DefMaxLoopsBeforeEmergencyAbort>0) {
            //mydebug("Emergency doContourTrace abort regDetectorDoContourTrace due to high loop count.");
            // ATTN: this triggering seems to be a possible related to crash cause -- but why would that be
            return false;
        }

        /* Scan around current pixel in clockwise order. */
        for(j=0; j<8; j++, i=(i+1)&7)
        {
            const int16_t x1 = x0 + dx[i];
            const int16_t y1 = y0 + dy[i];

            if ((x1 < 0) || (x1 >= width)) { continue; }
            if ((y1 < 0) || (y1 >= height)) { continue; }

            const int offset = CompPointOffset(x1, y1);

            // we have a problem -- we are being called at a pixel that TOUCHES a pixel on the exterior border, so that our attempt to trace a hole is conflicting with our previous trace of border
            // we have stumbled onto ourselves
            if (!external && labels[offset] == currentLabel && exteriorLabels[offset]!=0) {
                // we are doing an INTERNAL trace but we hit an EXTERNAL contour -- that means we are TOUCING
                //mydebug("ATTN: interior hole trace for region %d stumbled on exterior contour checking point %d,%d is already labeled as %d exteriorLabel = %d.", currentLabel,  x1, y1, labels[offset], exteriorLabels[offset]);
                // in this case i think we just stop
                // though this MAY cause some problems in our main contour loop regarding coming across a pixel that should be labeled but isnt..
                return true;
            } else {
                // check for foreground or background color?
                pixelEnum = pixels[offset];
                //if (pixelEnum != DefRdPixelEnumColorBackground) {
                //if ((external && (pixelEnum==DefRdPixelEnumColor1 || pixelEnum==DefRdPixelEnumColor3)) || (!external && pixelEnum != DefRdPixelEnumColorBackground)) {
                if (pixelEnum==DefRdPixelEnumColor1 || pixelEnum==DefRdPixelEnumColor3) {
                    // foreground pixel
                    if ((xx < 0) && (yy < 0)) {
                        // initial.. why is this being tested in such an inefficient fashion??
                        // this seems wrong, the other branch still checks xx and yy
                        xx = x1;
                        yy = y1;
                    }
                    else {
                        /* We are done if we crossed the first 2 contour points again. */
                        done = ((x == x0) && (xx == x1))
                            && ((y == y0) && (yy == y1));
                    }

                    // assign label to this exterior foreground border pixel
                    if (false) {
                        // already labeled for us, this is WEIRD
                        // let's leave everything else as is but not update bounding box or pixel counts..
                        // EVEN though i feel like this is a problem with code
                        // mydebug("contour checking point %d,%d is already labeled as %d.", x1, y1, labels[offset]);
                    } else {
                        if (labels[offset] != currentLabel) {
                            labels[offset] = currentLabel;
                            if (external) {
                                // if we are doing an external boundary trace, record this
                                exteriorLabels[offset] = currentLabel;
                            }
                            //
                            updateRegionDataOnForegroundPixel(currentLabel, x1, y1, external, pixelEnum);
                            //
                            // bounding box
                            if (x1 < bbxmin) {
                                bbxmin = x1;
                            }
                            if (y1 < bbymin) {
                                bbymin = y1;
                            }
                            if (x1 > bbxmax) {
                                bbxmax = x1;
                            }
                            if (y1 > bbymax) {
                                bbymax = y1;
                            }
                        } else {
                            // we have crossed onto ourselves so don't recount it..?
                        }
                    }

                    // update x0,y0
                    x0 = x1;
                    y0 = y1;
                    break;
                }
                else {
                    labels[offset] = DefRegLabelMarked;
                }
            }
        }
        /* Isolated point. */
        if (8 == j) {
            done = 1;
        }
        /* Compute next start position. */
        /* 1. Compute the neighbour index of the previous point. */

        // ATTN: NEW TEST via paper
        int previous = (i+4) % 8;
        //int previous = (i+4) & 7;
        /* 2. Next search index is previous + 2 (mod 8). */
        //i = (previous + 2) & 7;
        i = (previous + 2) % 8;
    }


    // set bounding box info
    JrRegionSummary* region = &regions[currentLabel - 1];
    if (external) {
        // external bounding box
        region->x1 = bbxmin;
        region->y1 = bbymin;
        region->x2 = bbxmax;
        region->y2 = bbymax;
        unsigned long boundingBoxArea = (region->x2 - region->x1) * (region->y2 - region->y1);
        //mydebug("Adding exterior region trace bounding box to region %d of size %lu (%d x %d).", currentLabel, boundingBoxArea, (region->x2 - region->x1), (region->y2 - region->y1));
    } else {
        unsigned long boundingBoxArea = (bbxmax - bbxmin) * (bbymax - bbymin);
        region->pixelCountHoleBoundingBoxes += boundingBoxArea;
        //mydebug("Adding hole bounding box to region %d of size %lu (%d x %d).", currentLabel, boundingBoxArea,  (bbxmax - bbxmin), (bbymax - bbymin));
    }
    
    //regiondebug("ATTN: OUT doContourTrace.");    
    return true;
}









void JrRegionDetector::initializeRegionData(DefLabelType labelIndex) {
    if (labelIndex <= 0) {
        // this doesn't happen
        mydebug("ERROR initializeRegionData with labelIndex==0.");
        return;
    }
    if (labelIndex >= DefMaxRegionsStore-1) {
        return;
    }
    //mydebug("ATTN: initializing regiondata label %d.", labelIndex);
    JrRegionSummary* region = &regions[labelIndex-1];
    region->init();
}


void JrRegionDetector::updateRegionDataOnForegroundPixel(DefLabelType labelIndex, int x, int y, bool pixelIsOnBorder, DefPixelEnumType pixelEnum) {
    if (labelIndex <= 0) {
        // what is this situation? it is responsible for delayed memory corruption crashing 9/14/22
        // see below where we refer to labelIndex-1
        if (DefDebugComplainBadIndexInRegionDetector) {
            mydebug("ERROR updateRegionDataOnForegroundPixel with labelIndex==0.");
        }
        return;
    }

    if (labelIndex >= DefMaxRegionsStore-1) {
        return;
    }
    JrRegionSummary* region = &regions[labelIndex-1];
    if (region->pixelCountAll == 0) {
        // first pixel in region, initialize bounding box
        if (labelIndex > foundRegions) {
            foundRegions = labelIndex;
        }
        /*
        region->x1 = region->x2 = x;
        region->y1 = region->y2 = y;
        */
    }
    else {
        /*
        // adjust bounding box
        if (x < region->x1) {
            region->x1 = x;
        }
        if (x > region->x2) {
            region->x2 = x;
        }
        if (y < region->y1) {
            region->y1 = y;
        }
        if (y > region->y2) {
            region->y2 = y;
        }
        */
    }
    // increment pixel counts
    ++region->pixelCountAll;
    ++region->pixelCountPerEnumColor[pixelEnum];
    if (pixelIsOnBorder) {
        ++region->pixelCountBorder;
        ++region->pixelCountBorderPerEnumColor[pixelEnum];
    } else {
        ++region->pixelCountInterior;
        ++region->pixelCountInteriorPerEnumColor[pixelEnum];
    }
}



bool JrRegionDetector::isPixelEnumConsideredForeground(DefPixelEnumType pixelEnum, bool contextWalkingOutside) {
    if (contextWalkingOutside) {
        if (pixelEnum == DefRdPixelEnumColor1) {
            return true;
        }
        return false;
    }

    if (pixelEnum != DefRdPixelEnumColorBackground) {
        return true;
    }
    return false;
}



int JrRegionDetector::doConnectedComponentLabeling() {
	// start with label 1
    foundRegions = 0;
    foundRegionsValid = 0;
    currentLabel = 1;
    DefPixelEnumType pixelEnum;
    bool abort = false;


    //mydebug("ATTN: IN doConnectedComponentLabeling.");

    initializeRegionData(currentLabel);

	// walk pixels top to bottom, then left to right
	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{
            pixelEnum = pixels[CompPointOffset(i, j)];

            // pure background we can skip
            if (DefRdPixelEnumColorBackground == pixelEnum) {
                continue;
            }
            bool isPreferredColor = (DefRdPixelEnumColor1 == pixelEnum || DefRdPixelEnumColor3 == pixelEnum);


            //mydebug("Pixel value at %dx%d is %d", i, j, pixelEnum);

            // ok so from here down we know we are on a colored foreground pixel

            const DefPixelEnumType abovePixel = (j > 0) ? pixels[CompPointOffset(i, j - 1)] : DefRdPixelEnumColorBackground;
			const DefPixelEnumType belowPixel = (j < height-1) ? pixels[CompPointOffset(i, j + 1)] : DefRdPixelEnumColorBackground;
            const DefLabelType belowLabel = (j < height-1) ? labels[CompPointOffset(i, j + 1)] : DefRegLabelMarked;

			const DefLabelType label = labels[CompPointOffset(i, j)];

            /* 1. new external countour */
//            if ((DefRegLabelUnset == label) && (DefRdPixelEnumColorBackground == abovePixel)) {
            if (isPreferredColor && (DefRegLabelUnset == label) && (DefRdPixelEnumColor1 != abovePixel && DefRdPixelEnumColor3 != abovePixel)) {
                // main work done here
				// a new region exterior border
                // trace around it and label exterior border
                abort |= !doContourTrace(1, currentLabel, i, j);
				// advance blob label
                ++currentLabel;
                // initialize new region data
                initializeRegionData(currentLabel);
                //mydebug("regiondata currentLabel = %d.", currentLabel);
                if (currentLabel > DefMaxRegionsAbort) {
                    // aborting because there are just too many
                    //regiondebug("ATTN: aborting doConnectedComponentLabeling due to too many regions found.");
                    abort = true;
                    break;
                }
            }
            /* 2. new internal countour */
//            else if ((DefRdPixelEnumColorBackground == belowPixel) && (DefRegLabelUnset == belowLabel))
            else if (isPreferredColor && (DefRdPixelEnumColor1 != belowPixel && DefRdPixelEnumColor3 != belowPixel) && (DefRegLabelUnset == belowLabel))
            {
                // sanity test
                // is this branch responsible for a problem passing a 0 label?
                // ATTN: can we use this to count HOLES in the region and record that for use in filtering valid regions (ie rejecting regions with large holes)
				DefLabelType workingPointLabel = (label != DefRegLabelUnset) ? label : labels[CompPointOffset(i-1, j)];

                if (workingPointLabel <= 0) {
                    // ATTN: this should not happen in original code..
                    //mydebug("WARNING: UNEXPECTEDLY hit workingPointLabel = %d (label = %d) at location %d,%d (wxh = %d x %d).", workingPointLabel, label, i,j, width, height);
                    // let's try using 3. internal element code from below
                    if (true) {
                        // label interior element
                        const DefLabelType interiorLabel = (i > 0) ? labels[CompPointOffset(i - 1, j)] : DefRegLabelUnset;
                        labels[CompPointOffset(i, j)] = interiorLabel;
                        if (interiorLabel != DefRegLabelUnset) {
                            updateRegionDataOnForegroundPixel(interiorLabel, i, j, false, pixelEnum);
                        }
                    }
                } else {
                    /* add a new internal contour to the corresponding blob. */
                    abort |= !doContourTrace(0, workingPointLabel, i, j);
                }
            }
            /* 3. internal element */
            else if (DefRegLabelUnset == label) {
                // label interior element -- we do this on ANY color other than background
                const DefLabelType interiorLabel = (i > 0) ? labels[CompPointOffset(i - 1, j)] : DefRegLabelUnset;
                labels[CompPointOffset(i, j)] = interiorLabel;
                if (interiorLabel != DefRegLabelUnset) {
                    updateRegionDataOnForegroundPixel(interiorLabel, i, j, false, pixelEnum);
                }
            }
            if (abort) {
                break;
            }
		}
        if (abort) {
            break;
        }
    }

    // post process
    postProcessRegionData();

    //mydebug("ATTN: OUT doConnectedComponentLabeling foundregions = %d.", foundRegions);

    // return # regions gound
    return foundRegions;
}




void JrRegionDetector::postProcessRegionData() {
    JrRegionSummary* region;
    //mydebug("postProcessRegionData foundregions = %d.", foundRegions);
    for (int i = 0; i < foundRegions; ++i) {
        region = &regions[i];
        int rwidth = (region->x2 - region->x1) + 1;
        int rheight = (region->y2 - region->y1) + 1;

        // mydebug("Calculating aspect with %d,%d, ", rwidth, rheight);
        // we know the bounding box size and how many pixels were labeled within the region, so we can now calculate the "density" of the region in terms of foreground pixels.  aperfectly aligned quared will ahve a density maxed out at 1.0
        region->label = i;
        region->area = (unsigned long) rheight * (unsigned long) rwidth;
        region->aspect = rheight < rwidth ? (float)rheight / (float)rwidth : (float)rwidth / (float)rheight;
        if (region->pixelCountAll == 0) {
            region->density = 0.0;
        } else {
            region->density = (float)region->pixelCountAll / (float)(region->pixelCountHoleBoundingBoxes + region->pixelCountAll);
        }
    }
}








void JrRegionDetector::calibrateScanRegion(JrRegionSummary* regionp, unsigned long &colorSumR, unsigned long &colorSumG, unsigned long &colorSumB, unsigned long& foregroundPixelsFount,  uint32_t linesize,  uint8_t* internalObsRgbAData) {
	uint8_t pixelThreshold = 254;
    uint8_t r, g, b;
    unsigned char* bpos;

    for (int iy = regionp->y1; iy < regionp->y2; ++iy) {
        for (int ix = regionp->x1; ix < regionp->x2; ++ix) {
            bpos = &internalObsRgbAData[CompRgbaByteOffset(ix, iy, linesize)];
            uint8_t alpha = *(bpos+3);
            if (alpha > pixelThreshold) {
                // this is a chroma pixel we care about it's color
                r = *(bpos);
                g = *(bpos + 1);
                b = *(bpos + 2);
                colorSumR += (unsigned long)r;
                colorSumG += (unsigned long)g;
                colorSumB += (unsigned long)b;
                //mydebug("calibrateScanRegion %dx%d: r=%d, g=%d, b=%d, a=%d.", ix, iy, r, g, b, alpha);
                ++foregroundPixelsFount;
            } else {
                // background, meaning didnt chroma match so we ignore it
                //mydebug("calibrateScanRegion %dx%d: low alpha %d.", ix, iy, alpha);
            }
        }
    }
}
//---------------------------------------------------------------------------















//---------------------------------------------------------------------------
void JrRegionDetector::doRenderToInternalMemoryPostProcessing(DefPixelRgbaType* internalObsRgbAData, uint32_t linesize, int optionGapFillSize) {
    // ATTN: we could try doing a dilation blur type operation using a filter instead of this hand code
    bool flagDilateIntoGreenAlso = true;

    if (optionGapFillSize == 0) {
        // nothing to do
        return;
    }

    int foregroundPixelCount = 0;
    //
    DefPixelRgbaType pixelVal;
    DefPixelRgbaType pixelVald = 0;
    //
    unsigned long offset;
    unsigned long hitTargetCount = 0;
    unsigned long hitDilateCount = 0;

    // we start in and end early just for efficiency since we shouldnt need to mess with pixels on border
    int sy = 0 + optionGapFillSize;
    int sx = 0 + optionGapFillSize;
    int ex = width - optionGapFillSize;
    int ey = height - optionGapFillSize;
    //
    DefPixelRgbaType drawPixel;
    //

    //
    for (int iy = sy; iy < ey; ++iy) {
        for (int ix = sx; ix < ex; ++ix) {
            // get alpha val
            pixelVal = internalObsRgbAData[CompPointOffset(ix, iy)];
            // we are looking for a particular color and alpha that we will dilate
            // does pixel match
            if (pixelVal == DefRdPixelRgbaColor1) {

                // test
                //internalObsRgbAData[CompPointOffset(ix, iy)] = DefRdPixelRgbaColor2;
                // 
                // found a hit
                ++hitTargetCount;
                // ok now we will dilate THIS particular pixel
                int dsx = ix - optionGapFillSize;
                int dsy = iy - optionGapFillSize;
                int dex = ix + optionGapFillSize;
                int dey = iy + optionGapFillSize;
                int dx, dy;

                // dilate left
                for (dx = ix - 1; dx >= dsx; --dx) {
                    offset = CompPointOffset(dx, iy);
                    pixelVald = internalObsRgbAData[offset];
                    if (pixelVald != DefRdPixelRgbaColorBackground) {
                        break;
                    }
                }
                if (dx >= dsx && dx < ix - 1) {
                    // we stopped early but after at least 2 pixels, did we hit target color?
                    if (pixelVald == DefRdPixelRgbaColor2 || flagDilateIntoGreenAlso) {
                        // yes, so back in opposite direction and color these blank pixels
                        // paint either our color 3 if we are bridging gap between green and magenta, or green if we are just pure dilating
                        drawPixel = (pixelVald == DefRdPixelRgbaColor1) ? DefRdPixelRgbaColor1 : DefRdPixelRgbaColor3;
                        for (dx+=1; dx<ix; ++dx) {
                            offset = CompPointOffset(dx, iy);
                            internalObsRgbAData[offset] = drawPixel;
                            ++hitDilateCount;
                       }
                    }
                }

                // dilate right
                for (dx = ix + 1; dx <= dex; ++dx) {
                    offset = CompPointOffset(dx, iy);
                    pixelVald = internalObsRgbAData[offset];
                    if (pixelVald != DefRdPixelRgbaColorBackground) {
                        break;
                    }
                }
                if (dx <= dex && dx > ix + 1) {
                    // we stopped early but after at least 2 pixels, did we hit target color?
                    if (pixelVald == DefRdPixelRgbaColor2 || flagDilateIntoGreenAlso) {
                        // yes, so back in opposite direction and color these blank pixels
                        // paint either our color 3 if we are bridging gap between green and magenta, or green if we are just pure dilating
                        drawPixel = (pixelVald == DefRdPixelRgbaColor1) ? DefRdPixelRgbaColor1 : DefRdPixelRgbaColor3;
                        for (dx-=1; dx>ix; --dx) {
                            offset = CompPointOffset(dx, iy);
                            internalObsRgbAData[offset] = drawPixel;
                            ++hitDilateCount;
                       }
                    }
                }

                // dilate up
                for (dy = iy - 1; dy >= dsy; --dy) {
                    offset = CompPointOffset(ix, dy);
                    pixelVald = internalObsRgbAData[offset];
                    if (pixelVald != DefRdPixelRgbaColorBackground) {
                        break;
                    }
                }
                if (dy >= dsy && dy < iy - 1) {
                    // we stopped early but after at least 2 pixels, did we hit target color?
                    if (pixelVald == DefRdPixelRgbaColor2 || flagDilateIntoGreenAlso) {
                        // yes, so back in opposite direction and color these blank pixels
                        // paint either our color 3 if we are bridging gap between green and magenta, or green if we are just pure dilating
                        drawPixel = (pixelVald == DefRdPixelRgbaColor1) ? DefRdPixelRgbaColor1 : DefRdPixelRgbaColor3;
                        for (dy+=1; dy<iy; ++dy) {
                            offset = CompPointOffset(ix, dy);
                            internalObsRgbAData[offset] = drawPixel;
                            ++hitDilateCount;
                       }
                    }
                }

                // dilate down
                for (dy = iy + 1; dy <= dey; ++dy) {
                    offset = CompPointOffset(ix, dy);
                    pixelVald = internalObsRgbAData[offset];
                    if (pixelVald != DefRdPixelRgbaColorBackground) {
                        break;
                    }
                }
                if (dy <= dey && dy > iy + 1) {
                    // we stopped early but after at least 2 pixels, did we hit target color?
                    if (pixelVald == DefRdPixelRgbaColor2 || flagDilateIntoGreenAlso) {
                        // yes, so back in opposite direction and color these blank pixels
                        // paint either our color 3 if we are bridging gap between green and magenta, or green if we are just pure dilating
                        drawPixel = (pixelVald == DefRdPixelRgbaColor1) ? DefRdPixelRgbaColor1 : DefRdPixelRgbaColor3;
                        for (dy-=1; dy>iy; --dy) {
                            offset = CompPointOffset(ix, dy);
                            internalObsRgbAData[offset] = drawPixel;
                            ++hitDilateCount;
                       }
                    }
                }


            }
        }
    }

    //mydebug("Finished doRenderToInternalMemoryPostProcessing with %lu and %lu.", hitTargetCount, hitDilateCount);
}
//---------------------------------------------------------------------------
