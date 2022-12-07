#pragma once

#include <windows.h>
#include <ctime>
//
// obs
#include <obs-module.h>
//
#include <graphics/matrix4.h>
#include <graphics/vec2.h>
#include <graphics/vec4.h>
//
#include "obsHelpers.h"


void jrazUint32ToRgbVec(uint32_t color, struct vec3& clvec);
void jrazUint32ToHsvVec(uint32_t color, struct vec3& clvec);
void jrazUint32ToRgbaVec(uint32_t color, struct vec4& clvec);
void RGBtoHSV(float& fR, float& fG, float fB, float& fH, float& fS, float& fV);
void jrazFillRgbaVec(vec4& colorvec, float red, float green, float blue, float alpha);
