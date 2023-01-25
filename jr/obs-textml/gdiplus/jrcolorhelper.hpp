#pragma once

#include <graphics/math-defs.h>
#include <util/platform.h>
#include <util/util.hpp>
#include <obs-module.h>
#include <sys/stat.h>
#include <combaseapi.h>
#include <gdiplus.h>


uint32_t hueShiftColor_v1_old(uint32_t color, int hueShift);
uint32_t hueShiftColor(uint32_t color, int hueShift);
uint32_t hsvShiftColor(uint32_t color, int hueShift, int saturationShift, int valueShift);
