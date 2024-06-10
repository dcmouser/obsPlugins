// obs-text is part of OBS
// multi-line modifications here by jessereichler@gmail.com

#include <graphics/math-defs.h>
#include <util/platform.h>
#include <util/util.hpp>
#include <obs-module.h>
#include <sys/stat.h>
#include <combaseapi.h>
#include <gdiplus.h>
#include <algorithm>
#include <string>
#include <memory>
#include <locale>

#include "jrcolorhelper.hpp"


//---------------------------------------------------------------------------
// new 11/28/22 - adjustments do NOT reset on newlines
#define DefClearAdjustmentsOnHardNewline false
#define DefClearAdjustmentsOnNNewline false
//---------------------------------------------------------------------------











using namespace std;
using namespace Gdiplus;

#define warning(format, ...)                                           \
	blog(LOG_WARNING, "[%s] " format, obs_source_get_name(source), \
	     ##__VA_ARGS__)

#define warn_stat(call)                                                   \
	do {                                                              \
		if (stat != Ok)                                           \
			warning("%s: %s failed (%d)", __FUNCTION__, call, \
				(int)stat);                               \
	} while (false)

#ifndef clamp
#define clamp(val, min_val, max_val) \
	if (val < min_val)           \
		val = min_val;       \
	else if (val > max_val)      \
		val = max_val;
#endif

#define MIN_SIZE_CX 2
#define MIN_SIZE_CY 2
#define MAX_SIZE_CX 16384
#define MAX_SIZE_CY 16384

#define MAX_AREA (4096LL * 4096LL)

/* ------------------------------------------------------------------------- */

/* clang-format off */

#define S_FONT                          "font"
#define S_USE_FILE                      "read_from_file"
#define S_FILE                          "file"
#define S_TEXT                          "text"
#define S_COLOR                         "color"
#define S_GRADIENT                      "gradient"
#define S_GRADIENT_COLOR                "gradient_color"
#define S_GRADIENT_DIR                  "gradient_dir"
#define S_GRADIENT_OPACITY              "gradient_opacity"
#define S_ALIGN                         "align"
#define S_VALIGN                        "valign"
#define S_OPACITY                       "opacity"
#define S_BKCOLOR                       "bk_color"
#define S_BKOPACITY                     "bk_opacity"
#define S_VERTICAL                      "vertical"
#define S_OUTLINE                       "outline"
#define S_OUTLINE_SIZE                  "outline_size"
#define S_OUTLINE_COLOR                 "outline_color"
#define S_OUTLINE_OPACITY               "outline_opacity"
#define S_CHATLOG_MODE                  "chatlog"
#define S_CHATLOG_LINES                 "chatlog_lines"
#define S_EXTENTS                       "extents"
#define S_EXTENTS_WRAP                  "extents_wrap"
#define S_EXTENTS_CX                    "extents_cx"
#define S_EXTENTS_CY                    "extents_cy"
#define S_TRANSFORM                     "transform"
#define S_ANTIALIASING                  "antialiasing"

#define S_ALIGN_LEFT                    "left"
#define S_ALIGN_CENTER                  "center"
#define S_ALIGN_RIGHT                   "right"

#define S_VALIGN_TOP                    "top"
#define S_VALIGN_CENTER                 S_ALIGN_CENTER
#define S_VALIGN_BOTTOM                 "bottom"

#define S_TRANSFORM_NONE                0
#define S_TRANSFORM_UPPERCASE           1
#define S_TRANSFORM_LOWERCASE           2
#define S_TRANSFORM_STARTCASE           3

#define S_ANTIALIASING_NONE             0
#define S_ANTIALIASING_STANDARD         1

#define T_(v)                           obs_module_text(v)
#define T_FONT                          T_("Font")
#define T_USE_FILE                      T_("ReadFromFile")
#define T_FILE                          T_("TextFile")
#define T_TEXT                          T_("Text")
#define T_COLOR                         T_("Color")
#define T_GRADIENT                      T_("Gradient")
#define T_GRADIENT_COLOR                T_("Gradient.Color")
#define T_GRADIENT_DIR                  T_("Gradient.Direction")
#define T_GRADIENT_OPACITY              T_("Gradient.Opacity")
#define T_ALIGN                         T_("Alignment")
#define T_VALIGN                        T_("VerticalAlignment")
#define T_OPACITY                       T_("Opacity")
#define T_BKCOLOR                       T_("BkColor")
#define T_BKOPACITY                     T_("BkOpacity")
#define T_VERTICAL                      T_("Vertical")
#define T_OUTLINE                       T_("Outline")
#define T_OUTLINE_SIZE                  T_("Outline.Size")
#define T_OUTLINE_COLOR                 T_("Outline.Color")
#define T_OUTLINE_OPACITY               T_("Outline.Opacity")
#define T_CHATLOG_MODE                  T_("ChatlogMode")
#define T_CHATLOG_LINES                 T_("ChatlogMode.Lines")
#define T_EXTENTS                       T_("UseCustomExtents")
#define T_EXTENTS_WRAP                  T_("UseCustomExtents.Wrap")
#define T_EXTENTS_CX                    T_("Width")
#define T_EXTENTS_CY                    T_("Height")
#define T_TRANSFORM                     T_("Transform")
#define T_ANTIALIASING                  T_("Antialiasing")

#define T_FILTER_TEXT_FILES             T_("Filter.TextFiles")
#define T_FILTER_ALL_FILES              T_("Filter.AllFiles")

#define T_ALIGN_LEFT                    T_("Alignment.Left")
#define T_ALIGN_CENTER                  T_("Alignment.Center")
#define T_ALIGN_RIGHT                   T_("Alignment.Right")

#define T_VALIGN_TOP                    T_("VerticalAlignment.Top")
#define T_VALIGN_CENTER                 T_ALIGN_CENTER
#define T_VALIGN_BOTTOM                 T_("VerticalAlignment.Bottom")

#define T_TRANSFORM_NONE                T_("Transform.None")
#define T_TRANSFORM_UPPERCASE           T_("Transform.Uppercase")
#define T_TRANSFORM_LOWERCASE           T_("Transform.Lowercase")
#define T_TRANSFORM_STARTCASE           T_("Transform.Startcase")

#define S_MULTILINEGROUP_TEXT		"Multi-line tweaks (see help for formatting syntax)"
#define S_MULTILINE			"multiline"
#define S_MULTILINE_TEXT		T_("Enable multi-line features and color effects (uncheck to revert to normal Text source behavior)")
#define S_MULTILINE_DEF			true
#define S_WRAPLEN			"wraplen"
#define S_WRAPLEN_TEXT			T_("Word wrap position")
#define S_WRAPLEN_DEF			40
#define S_LINESPACEADJUST		"lineSpaceAdjust"
#define S_LINESPACEADJUST_TEXT		T_("Line spacing adjustment")
#define S_LINESPACEADJUST_DEF		-20
#define S_GRADIENTPERLINE		"gradientPerLine"
#define S_GRADIENTPERLINE_TEXT		T_("Gradients are per line")
#define S_GRADIENTPERLINE_DEF		true
#define S_NOTES				"notes"
#define S_NOTES_TEXT			T_("Private Notes")
#define S_NOTES_DEF			""
#define S_HueQuickShift			"hueQuickShift"
#define S_HueQuickShift_TEXT		"Quick hue shift"
#define S_HueQuickShift_DEF		0
#define S_SaturationQuickShift		"saturationQuickShift"
#define S_SaturationQuickShift_TEXT	"Quick sat shift"
#define S_SaturationQuickShift_DEF	0
#define S_ValueQuickShift		"valueQuickShift"
#define S_ValueQuickShift_TEXT		"Quick val shift"
#define S_ValueQuickShift_DEF		0

#define S_FontSizeQuickScale		"fontSizeQuickScale"
#define S_FontSizeQuickScale_TEXT	"Quick font size modifier"
#define S_FontSizeQuickScale_DEF	100


#define DEF_MAX_LINES			200

/* clang-format on */

/* ------------------------------------------------------------------------- */









//---------------------------------------------------------------------------
class TextModifier {
public:
	float fontSizeAdjustment;
	float lineSpaceAdjustment;
	int hueShift;
	int valueShift;
	int saturationShift;
	float wordWrapAdjustment;
	int xoff, yoff;
	uint32_t color1, color2;
	int fontWeight;
	int fontStyleMask;
	std::wstring modifiedFace;
public:
	TextModifier() { reset(); };
	void reset() {  modifiedFace = L"";  fontStyleMask = -1;  fontWeight = 0; fontSizeAdjustment = 1.0f; lineSpaceAdjustment = 1.0f; hueShift = 0; valueShift = 0; saturationShift = 0; wordWrapAdjustment = 1.0f; xoff = 0; yoff = 0; color1 = 0; color2 = 0; };
};
//---------------------------------------------------------------------------








static inline DWORD get_alpha_val(uint32_t opacity)
{
	return ((opacity * 255 / 100) & 0xFF) << 24;
}


static inline DWORD calc_color(uint32_t color, uint32_t opacity, int hueShift, int saturationShift, int valueShift)
{
	//return color & 0xFFFFFF | get_alpha_val(opacity);
	return hsvShiftColor(color,hueShift, saturationShift, valueShift, true) & 0xFFFFFF | get_alpha_val(opacity);
}

/* ------------------------------------------------------------------------- */



static inline wstring to_wide(const char *utf8)
{
	wstring text;

	size_t len = os_utf8_to_wcs(utf8, 0, nullptr, 0);
	text.resize(len);
	if (len)
		os_utf8_to_wcs(utf8, 0, &text[0], len + 1);

	return text;
}

static inline uint32_t rgb_to_bgr(uint32_t rgb)
{
	return ((rgb & 0xFF) << 16) | (rgb & 0xFF00) | ((rgb & 0xFF0000) >> 16);
}

/* ------------------------------------------------------------------------- */

template<typename T, typename T2, BOOL WINAPI deleter(T2)> class GDIObj {
	T obj = nullptr;

	inline GDIObj &Replace(T obj_)
	{
		if (obj)
			deleter(obj);
		obj = obj_;
		return *this;
	}

public:
	inline GDIObj() {}
	inline GDIObj(T obj_) : obj(obj_) {}
	inline ~GDIObj() { deleter(obj); }

	inline T operator=(T obj_)
	{
		Replace(obj_);
		return obj;
	}

	inline operator T() const { return obj; }

	inline bool operator==(T obj_) const { return obj == obj_; }
	inline bool operator!=(T obj_) const { return obj != obj_; }
};

using HDCObj = GDIObj<HDC, HDC, DeleteDC>;
using HFONTObj = GDIObj<HFONT, HGDIOBJ, DeleteObject>;
using HBITMAPObj = GDIObj<HBITMAP, HGDIOBJ, DeleteObject>;

/* ------------------------------------------------------------------------- */

enum class Align {
	Left,
	Center,
	Right,
};

enum class VAlign {
	Top,
	Center,
	Bottom,
};

struct TextSource {
// new attempt to add some padding
	int paddingLeft = 20;
	int paddingRight = 30;
	int paddingTop = 25;
	int paddingBottom = 40;
//
	obs_source_t *source = nullptr;

	gs_texture_t *tex = nullptr;
	uint32_t cx = 0;
	uint32_t cy = 0;

	HDCObj hdc;
	Graphics graphics;

	HFONTObj hfont;
	unique_ptr<Font> font;

	bool read_from_file = false;
	string file;
	time_t file_timestamp = 0;
	bool update_file = false;
	float update_time_elapsed = 0.0f;

	wstring text;
	wstring face;
	int face_size = 0;
	uint32_t color = 0xFFFFFF;
	uint32_t color2 = 0xFFFFFF;
	float gradient_dir = 0;
	uint32_t opacity = 100;
	uint32_t opacity2 = 100;
	uint32_t bk_color = 0;
	uint32_t bk_opacity = 0;
	Align align = Align::Left;
	VAlign valign = VAlign::Top;
	bool gradient = false;
	bool bold = false;
	bool italic = false;
	bool underline = false;
	bool strikeout = false;
	bool antialiasing = true;
	bool vertical = false;

	bool use_outline = false;
	float outline_size = 0.0f;
	uint32_t outline_color = 0;
	uint32_t outline_opacity = 100;

	bool use_extents = false;
	bool wrap = false;
	uint32_t extents_cx = 0;
	uint32_t extents_cy = 0;

	int text_transform = S_TRANSFORM_NONE;

	bool chatlog_mode = false;
	int chatlog_lines = 6;

	// new tweaks
	bool multineTweakEnable;
	int tweakWraplen;
	bool gradientPerLine;
	float tweakHeightAdjust;
	//
	float fontSizeQuickScale = 1.0f;
	int hueQuickShift = 0;
	int saturationQuickShift = 0;
	int valueQuickShift = 0;
	//
	float lineHeightOrig[DEF_MAX_LINES];
	float lineHeight[DEF_MAX_LINES];
	std::string lastFontUpdateString;
	bool flagClearNextLineFontSizeAdjustment;

	/* --------------------------- */

	inline TextSource(obs_source_t *source_, obs_data_t *settings)
		: source(source_),
		  hdc(CreateCompatibleDC(nullptr)),
		  graphics(hdc)
	{
		obs_source_update(source, settings);
	}

	inline ~TextSource()
	{
		if (tex) {
			obs_enter_graphics();
			gs_texture_destroy(tex);
			obs_leave_graphics();
		}
	}

	void UpdateFont(TextModifier &tmodifier, bool flagForceUpdate);
	void GetStringFormat(StringFormat &format);
	void RemoveNewlinePadding(const StringFormat &format, RectF &box);
	void CalculateTextSizes(const StringFormat &format, RectF &bounding_box, SIZE &text_size);
	void RenderOutlineText(Graphics &graphics, const GraphicsPath &path, const Brush &brush, TextModifier &tmodifier);
	void RenderText();
	void LoadFileText();
	void TransformText();
	void SetAntiAliasing(Graphics &graphics_bitmap);

	const char *GetMainString(const char *str);

	inline void Update(obs_data_t *settings);
	inline void Tick(float seconds);
	inline void Render();
protected:
	bool jrSplitWStringOnChar(std::wstring& mainstr, std::wstring& oneline, wchar_t wc);
	void parseLineSplitModString(std::wstring modString, TextModifier &tmodifier);
	bool jrSplitWStringLine(std::wstring& mainstr, std::wstring &oneline, TextModifier &tmodifier);
	void setCurrentBrushColorUsingHsvShift(LinearGradientBrush* brushp, uint32_t cl1, uint32_t cl2, TextModifier &tmodifier);
	uint32_t parseColorStr(std::wstring ws);
	void wstrim(std::wstring& s);
};

static time_t get_modified_timestamp(const char *filename)
{
	struct stat stats;
	if (os_stat(filename, &stats) != 0)
		return -1;
	return stats.st_mtime;
}

void TextSource::UpdateFont(TextModifier &tmodifier, bool flagForceUpdate)
{
	// computed modified values
	int fontSize = (int) ((float)face_size * tmodifier.fontSizeAdjustment * fontSizeQuickScale);
	int fontWeight = bold ? FW_BOLD : FW_DONTCARE;
	fontWeight += tmodifier.fontWeight;
	if (fontWeight < 0) fontWeight = 0;
	if (fontWeight > 1000) fontWeight = 1000;
	// italic, underline, strikeout
	bool fitalic = italic;
	bool funderline = underline;
	bool fstrikeout = strikeout;
	if (tmodifier.fontStyleMask != -1) {
		fitalic = (tmodifier.fontStyleMask & OBS_FONT_ITALIC) != 0;
		funderline = (tmodifier.fontStyleMask & OBS_FONT_UNDERLINE) != 0;
		fstrikeout = (tmodifier.fontStyleMask & OBS_FONT_STRIKEOUT) != 0;
	}

	// see https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-logfonta
	// create a string summarizing font details so we can tell when it has changed
	std::string fontUpdateString = "FS:" + std::to_string(fontSize);
	fontUpdateString += "|FW=" + std::to_string(fontWeight);
	fontUpdateString += "|FM=" + std::to_string(tmodifier.fontStyleMask);
	//
	wstring fontFace = face;
	if (tmodifier.modifiedFace != L"") {
		// convert from our stdstring font face to wstring used fontFace and then add it to updatestring
		fontFace = tmodifier.modifiedFace;
		//
		std::string fontFaceHashed(fontFace.length(), 0);
		std::transform(fontFace.begin(), fontFace.end(), fontFaceHashed.begin(), [] (wchar_t c) { return (char)c; });
		fontUpdateString += "|FF=" + fontFaceHashed;
	}

	//
	if (!flagForceUpdate && fontUpdateString == lastFontUpdateString) {
		// unchanged, nothing to do
		return;
	}

	// remeber this config
	lastFontUpdateString = fontUpdateString;

	// create font
	hfont = nullptr;
	font.reset(nullptr);

	LOGFONT lf = {};
	lf.lfHeight = fontSize;
	lf.lfWeight = fontWeight;
	lf.lfItalic = fitalic;
	lf.lfUnderline = funderline;
	lf.lfStrikeOut = fstrikeout;
	lf.lfQuality = ANTIALIASED_QUALITY;
	lf.lfCharSet = DEFAULT_CHARSET;

	if (!fontFace.empty()) {
		wcscpy(lf.lfFaceName, fontFace.c_str());
		hfont = CreateFontIndirect(&lf);

		std::string fontFaceHashed(fontFace.length(), 0);
		std::transform(fontFace.begin(), fontFace.end(), fontFaceHashed.begin(), [] (wchar_t c) { return (char)c; });
		//blog(LOG_WARNING,"Setting to textml font: %s.", fontFaceHashed.c_str());
	}

	if (!hfont) {
		// fallback to non modified font face
		wcscpy(lf.lfFaceName, face.c_str());
		hfont = CreateFontIndirect(&lf);
	}
	if (!hfont) {
		// fallback to arial
		wcscpy(lf.lfFaceName, L"Arial");
		hfont = CreateFontIndirect(&lf);
	}

	if (hfont)
		font.reset(new Font(hdc, hfont));
}

void TextSource::GetStringFormat(StringFormat &format)
{
	UINT flags = StringFormatFlagsNoFitBlackBox |
		     StringFormatFlagsMeasureTrailingSpaces;

	if (vertical)
		flags |= StringFormatFlagsDirectionVertical |
			 StringFormatFlagsDirectionRightToLeft;

	format.SetFormatFlags(flags);
	format.SetTrimming(StringTrimmingWord);

	switch (align) {
	case Align::Left:
		if (vertical)
			format.SetLineAlignment(StringAlignmentFar);
		else
			format.SetAlignment(StringAlignmentNear);
		break;
	case Align::Center:
		if (vertical)
			format.SetLineAlignment(StringAlignmentCenter);
		else
			format.SetAlignment(StringAlignmentCenter);
		break;
	case Align::Right:
		if (vertical)
			format.SetLineAlignment(StringAlignmentNear);
		else
			format.SetAlignment(StringAlignmentFar);
	}

	switch (valign) {
	case VAlign::Top:
		if (vertical)
			format.SetAlignment(StringAlignmentNear);
		else
			format.SetLineAlignment(StringAlignmentNear);
		break;
	case VAlign::Center:
		if (vertical)
			format.SetAlignment(StringAlignmentCenter);
		else
			format.SetLineAlignment(StringAlignmentCenter);
		break;
	case VAlign::Bottom:
		if (vertical)
			format.SetAlignment(StringAlignmentFar);
		else
			format.SetLineAlignment(StringAlignmentFar);
	}
}

/* GDI+ treats '\n' as an extra character with an actual render size when
 * calculating the texture size, so we have to calculate the size of '\n' to
 * remove the padding.  Because we always add a newline to the string, we
 * also remove the extra unused newline. */
void TextSource::RemoveNewlinePadding(const StringFormat &format, RectF &box)
{
	RectF before;
	RectF after;
	Status stat;

	stat = graphics.MeasureString(L"W", 2, font.get(), PointF(0.0f, 0.0f),
				      &format, &before);
	warn_stat("MeasureString (without newline)");

	stat = graphics.MeasureString(L"W\n", 3, font.get(), PointF(0.0f, 0.0f),
				      &format, &after);
	warn_stat("MeasureString (with newline)");

	float offset_cx = after.Width - before.Width;
	float offset_cy = after.Height - before.Height;

	if (!vertical) {
		if (offset_cx >= 1.0f)
			offset_cx -= 1.0f;

		if (valign == VAlign::Center)
			box.Y -= offset_cy * 0.5f;
		else if (valign == VAlign::Bottom)
			box.Y -= offset_cy;
	} else {
		if (offset_cy >= 1.0f)
			offset_cy -= 1.0f;

		if (align == Align::Center)
			box.X -= offset_cx * 0.5f;
		else if (align == Align::Right)
			box.X -= offset_cx;
	}

	box.Width -= offset_cx;
	box.Height -= offset_cy;
}

bool TextSource::jrSplitWStringOnChar(std::wstring& mainstr, std::wstring& oneline, wchar_t wc) {
	if (mainstr == L"") {
		return false;
	}
	size_t pos = mainstr.find(wc);
	if (pos == std::wstring::npos) {
		oneline = mainstr;
		wstrim(oneline);
		mainstr = L"";
		return true;
	}
	// split it
	oneline = mainstr.substr(0,pos);
	mainstr = mainstr.substr(pos + 1);
	//
	wstrim(oneline);
	wstrim(mainstr);
	return true;
}


void TextSource::wstrim(std::wstring& s) {
	// Remove leading and trailing whitespace
	static const wchar_t whitespace[] = L" \n\t\v\r\f";
	s.erase( 0, s.find_first_not_of(whitespace) );
	s.erase( s.find_last_not_of(whitespace) + 1U );
}


void TextSource::parseLineSplitModString(std::wstring modString, TextModifier &tmodifier) {
	std::wstring oneline;
	while (jrSplitWStringOnChar(modString, oneline, L',')) {
		if (oneline.length() < 2) {
			continue;
		}
		//blog(LOG_WARNING, "modstring is '%s'.", modString.c_str());
		std::wstring amt = oneline.substr(2);
		if (oneline[0] == L'f' || oneline[0] == L'F') {
			if (oneline[1] == L'+') {
				tmodifier.fontSizeAdjustment =
					1.0f +
					(float)_wtof(amt.c_str()) / 100.0f;
			} else if (oneline[1] == L'-') {
				tmodifier.fontSizeAdjustment =
					1.0f -
					(float)_wtof(amt.c_str()) / 100.0f;
			}
			if (false) {
				// testing
				std::string str1, str2;
				std::transform(oneline.begin(), oneline.end(), std::back_inserter(str1), [](wchar_t c) {	return (char)c;	});
				std::transform(amt.begin(), amt.end(), std::back_inserter(str2), [](wchar_t c) {	return (char)c;	});
				//blog(LOG_WARNING, "font size adjust is online (%s vs %s) maps to '%f'.", str1.c_str(), str2.c_str(), tmodifier.fontSizeAdjustment);
			}
		} else if (oneline[0] == L'l' || oneline[0] == L'L') {
			if (oneline[1] == L'+') {
				tmodifier.lineSpaceAdjustment =
					1.0f +
					(float)_wtof(amt.c_str()) / 100.0f;
			} else if (oneline[1] == L'-') {
				tmodifier.lineSpaceAdjustment =
					1.0f -
					(float)_wtof(amt.c_str()) / 100.0f;
			}
		} else if (oneline[0] == L'h' || oneline[0] == L'H') {
			if (oneline[1] == L'+') {
				tmodifier.hueShift = _wtoi(amt.c_str());
			} else if (oneline[1] == L'-') {
				tmodifier.hueShift = -_wtoi(amt.c_str());
			}
		} else if (oneline[0] == L's' || oneline[0] == L'S') {
			if (oneline[1] == L'+') {
				tmodifier.saturationShift = _wtoi(amt.c_str());
			} else if (oneline[1] == L'-') {
				tmodifier.saturationShift = -_wtoi(amt.c_str());
			}
		} else if (oneline[0] == L'v' || oneline[0] == L'V') {
			if (oneline[1] == L'+') {
				tmodifier.valueShift = _wtoi(amt.c_str());
			} else if (oneline[1] == L'-') {
				tmodifier.valueShift = -_wtoi(amt.c_str());
			}
		} else if (oneline[0] == L'w' || oneline[0] == L'W') {
			if (oneline[1] == L'+') {
				tmodifier.wordWrapAdjustment =
					1.0f +
					(float)_wtoi(amt.c_str()) / 100.0f;
			} else if (oneline[1] == L'-') {
				tmodifier.wordWrapAdjustment =
					1.0f -
					(float)_wtoi(amt.c_str()) / 100.0f;
			}
		} else if (oneline[0] == L'x' || oneline[0] == L'X') {
			if (oneline[1] == L'+') {
				tmodifier.xoff =
					_wtoi(amt.c_str());
			} else if (oneline[1] == L'-') {
				tmodifier.xoff =
					0 - _wtoi(amt.c_str());
			}
		} else if (oneline[0] == L'y' || oneline[0] == L'Y') {
			if (oneline[1] == L'+') {
				tmodifier.yoff =
					_wtoi(amt.c_str());
			} else if (oneline[1] == L'-') {
				tmodifier.yoff =
					0 - _wtoi(amt.c_str());
			}
		} else if (oneline[0] == L'c' || oneline[0] == L'C') {
			if (oneline.length() >= 8) {
				amt = oneline.substr(3);
				if (oneline[1] == L'1' && oneline[2] == L'=') {
					amt = oneline.substr(3);
					tmodifier.color1 = parseColorStr(amt);
				}
				else if (oneline[1] == L'2' && oneline[2] == L'=') {
					amt = oneline.substr(3);
					tmodifier.color2 = parseColorStr(amt);
				}
			}
		} else if (oneline[0] == L'b' || oneline[0] == L'B') {
			if (oneline[1] == L'+') {
				tmodifier.fontWeight =
					_wtoi(amt.c_str());
			} else if (oneline[1] == L'-') {
				tmodifier.fontWeight =
					0 - _wtoi(amt.c_str());
			}
		} else if (oneline[0] == L'i' || oneline[0] == L'I') {
			if (oneline[1] == L'+') {
				tmodifier.fontStyleMask =
					_wtoi(amt.c_str());
			} else if (oneline[1] == L'-') {
				tmodifier.fontStyleMask = -1;
			}
		} else if (oneline[0] == L'a' && oneline[1] == L'=') {
			tmodifier.modifiedFace = amt;
		}
	}
}


bool TextSource::jrSplitWStringLine(std::wstring &mainstr, std::wstring &oneline, TextModifier &tmodifier) {
	// split off the next line from mainstr into oneline, doing wrap if needed
	int slen = (int)mainstr.length();
	if (slen == 0) {
		return false;
	}

	int pos = 0;
	int lastSpacePosStart = -1;
	int lastSpacePosEnd = -1;
	int breakLinePos = -1;
	int resumePos = -1;
	int lineLen = 0;
	bool flagInSpaces = false;
	wchar_t w;
	bool flagInSpecialMod = false;
	int realStartPos = 0;
	// we do NOT clear this, so that split lines keep their adjustments
	if (flagClearNextLineFontSizeAdjustment) {
		tmodifier.reset();
		flagClearNextLineFontSizeAdjustment = false;
	}

	// special font size adjustments
	for (int charCount=0; pos < slen; ++pos) {
		w = mainstr[pos];
		if (charCount == 0) {
			if (pos == 0 && w == L'[') {
				flagInSpecialMod = true;
				continue;
			} else if (flagInSpecialMod) {
				if (pos>1 && w == L']') {
					// done special mod
					realStartPos = pos+1;
					flagInSpecialMod = false;
					std::wstring modString = mainstr.substr(1, pos-1);
					parseLineSplitModString(modString, tmodifier);
				}
				continue;
			}
			// drop down
		}

		if (isspace(w)) {
			// track first and last space consecutively
			lastSpacePosEnd = pos;
			if (!flagInSpaces) {
				lastSpacePosStart = pos;
			}
			if (w == 13 || w == 10 || w==12) {
				// newline break
				breakLinePos = lastSpacePosStart;
				resumePos = lastSpacePosEnd + 1;
				flagClearNextLineFontSizeAdjustment = DefClearAdjustmentsOnHardNewline;
				if (w == 13 && pos < slen - 1 && mainstr[pos + 1] == 10) {
					// skip over the \13 \10
					++resumePos;
				}
				break;
			}
			flagInSpaces = true;
		} else {
			if (w == L'\\') {
				if (pos < slen - 1 && (mainstr[pos + 1] == L'n' || mainstr[pos + 1] == L'N')) {
					// newline equive
					if (lastSpacePosStart != -1 && flagInSpaces) {
						breakLinePos = lastSpacePosStart;
					} else {
						breakLinePos = pos;
					}
					resumePos = pos + 2;
					flagClearNextLineFontSizeAdjustment = DefClearAdjustmentsOnNNewline;
					break;
				}
			}
			if (charCount >= tweakWraplen*tmodifier.wordWrapAdjustment && tweakWraplen>0) {
				// ok we want to break on the last space/newline
				if (lastSpacePosStart > -1) {
					// break at last space
					breakLinePos = lastSpacePosStart;
					resumePos = lastSpacePosEnd + 1;
					break;
				} else {
					// no spaces so we are forced to break HERE
					breakLinePos = pos;
					resumePos = pos;
					break;
				}
			} else {
				++lineLen;
				flagInSpaces = false;
			}
		}
		charCount += 1;
	}
	if (pos == slen) {
		// we hit end of string without needing to split
		oneline = mainstr;
		mainstr = L"";
		return true;
	}
	// snap it off
	oneline = mainstr.substr(realStartPos, breakLinePos-realStartPos);
	mainstr = mainstr.substr(resumePos);
	return true;
}








void TextSource::CalculateTextSizes(const StringFormat &format,
				    RectF &bounding_box, SIZE &text_size)
{
	RectF layout_box;
	RectF temp_box;
	Status stat;
	TextModifier tmodifier;

	if (!text.empty()) {
		if (multineTweakEnable) {
			// new method does homebrew word wrap and height line adjustmane
			std::wstring tempText = text;
			std::wstring oneLine;
			RectF bounding_box_temp;
			int lineNumber = 1;
			lineHeightOrig[0] = 1.0f;
			lineHeight[0] = 1.0f;
			tmodifier.reset();
			
			bounding_box.X = bounding_box.Y = bounding_box.Width = bounding_box.Height = 0;
			flagClearNextLineFontSizeAdjustment = true;
			while (jrSplitWStringLine(tempText, oneLine, tmodifier)) {
				if (oneLine.length() >= 2 && oneLine[0] == L'/' && oneLine[1] == L'/') {
					// comment
					continue;
				}
				UpdateFont(tmodifier, false);

				bounding_box_temp.X = bounding_box_temp.Y = bounding_box_temp.Width = bounding_box_temp.Height = 0;

				stat = graphics.MeasureString(
					oneLine.c_str(), (int)oneLine.size() + 1, font.get(),
					PointF(0.0f, 0.0f), &format, &bounding_box_temp);
				if (use_outline) {
					bounding_box_temp.Width += outline_size;
					bounding_box_temp.Height += outline_size;
				}

				// adjust total height (sum of all adjusted lines) and width (max of all)
				bounding_box.Width = max(bounding_box.Width, bounding_box_temp.Width);
				lineHeightOrig[lineNumber] = (float)bounding_box_temp.Height;
				lineHeight[lineNumber] = ( (float)bounding_box_temp.Height * tweakHeightAdjust);
				if (tmodifier.lineSpaceAdjustment != 1.0f && lineNumber > 0) {
					// effects line height of the previous line
					lineHeight[lineNumber - 1] *= tmodifier.lineSpaceAdjustment;
				}
				//blog(LOG_WARNING, "Line %d returned height %f tweaked with %f to %f.", lineNumber, bounding_box_temp.Height, tweakHeightAdjust, lineHeight[lineNumber]);
				if (lineNumber > 1) {
					bounding_box.Height = bounding_box.Height + lineHeight[lineNumber-1];
				}
				else {
					// attn attempt to give it some extra height from first line
					// bounding_box.Height = bounding_box.Height + bounding_box_temp.Height;
				}
				if (lineNumber < DEF_MAX_LINES) {
					++lineNumber;
				}
			}
			if (lineNumber > 1) {
				bounding_box.Height = bounding_box.Height + lineHeight[lineNumber-1];
			}


			// we make the bounding box height use the full height of the last line
			if (lineNumber > 1 && (true || tempText.length()>0)) {
				//bounding_box.Height += (float)bounding_box_temp.Height * (1.0f - tweakHeightAdjust);
				// extra padding? use last line info? is this 0?
				if (true) {
					//float extraY = bounding_box_temp.Height * 0.25f;
					float extraY = bounding_box_temp.Height * 0.15f;
					float extraX = extraY;
					bounding_box.Height += extraY;
					bounding_box.Width += extraX;
					// kludge for first line offset used to move entire text up or down
					// ATTN: we now 3/21/23 have a y offset option that i think should eliminate need for this kludge?
					if (false) {
						bounding_box.Height += (lineHeight[0] - 1.0f) * lineHeight[1];
					}
				}
			}

			// new padding
			bounding_box.Width += paddingLeft+paddingRight;
			bounding_box.Height += paddingTop+paddingBottom;
			// extra kludge of space
			int fontSize = (int) ((float)face_size * tmodifier.fontSizeAdjustment * fontSizeQuickScale);
			bounding_box.Height += (float) fontSize * 0.25f;

			temp_box = bounding_box;
		}
		else if (use_extents && wrap) {
			// extents not used??
			layout_box.X = layout_box.Y = 0;
			layout_box.Width = float(extents_cx);
			layout_box.Height = float(extents_cy);

			if (use_outline) {
				layout_box.Width -= outline_size;
				layout_box.Height -= outline_size;
			}

			stat = graphics.MeasureString(text.c_str(),
						      (int)text.size() + 1,
						      font.get(), layout_box,
						      &format, &bounding_box);
			warn_stat("MeasureString (wrapped)");

			temp_box = bounding_box;
		} else {
			stat = graphics.MeasureString(
				text.c_str(), (int)text.size() + 1, font.get(),
				PointF(0.0f, 0.0f), &format, &bounding_box);
			warn_stat("MeasureString (non-wrapped)");

			temp_box = bounding_box;

			bounding_box.X = 0.0f;
			bounding_box.Y = 0.0f;

			RemoveNewlinePadding(format, bounding_box);

			if (use_outline) {
				bounding_box.Width += outline_size;
				bounding_box.Height += outline_size;
			}
		}
	}

	if (vertical) {
		if (bounding_box.Width < face_size) {
			text_size.cx = face_size;
			bounding_box.Width = float(face_size);
		} else {
			text_size.cx = LONG(bounding_box.Width + EPSILON);
		}

		text_size.cy = LONG(bounding_box.Height + EPSILON);
	} else {
		if (bounding_box.Height < face_size) {
			text_size.cy = face_size;
			bounding_box.Height = float(face_size);
		} else {
			text_size.cy = LONG(bounding_box.Height + EPSILON);
		}

		text_size.cx = LONG(bounding_box.Width + EPSILON);
	}

	if (!multineTweakEnable && use_extents) {
		text_size.cx = extents_cx;
		text_size.cy = extents_cy;
	}

	text_size.cx += text_size.cx % 2;
	text_size.cy += text_size.cy % 2;

	int64_t total_size = int64_t(text_size.cx) * int64_t(text_size.cy);

	/* GPUs typically have texture size limitations */
	clamp(text_size.cx, MIN_SIZE_CX, MAX_SIZE_CX);
	clamp(text_size.cy, MIN_SIZE_CY, MAX_SIZE_CY);

	/* avoid taking up too much VRAM */
	if (total_size > MAX_AREA) {
		if (text_size.cx > text_size.cy)
			text_size.cx = (LONG)MAX_AREA / text_size.cy;
		else
			text_size.cy = (LONG)MAX_AREA / text_size.cx;
	}

	/* the internal text-rendering bounding box for is reset to
	 * its internal value in case the texture gets cut off */
	bounding_box.Width = temp_box.Width;
	bounding_box.Height = temp_box.Height;
}

void TextSource::RenderOutlineText(Graphics &graphics, const GraphicsPath &path,
				   const Brush &brush, TextModifier &tmodifier)
{
	DWORD outline_rgba = calc_color(outline_color, outline_opacity, 0,0,0);
	Status stat;

	Pen pen(Color(outline_rgba), outline_size);
	stat = pen.SetLineJoin(LineJoinRound);
	warn_stat("pen.SetLineJoin");

	stat = graphics.DrawPath(&pen, &path);
	warn_stat("graphics.DrawPath");

	stat = graphics.FillPath(&brush, &path);
	warn_stat("graphics.FillPath");
}

void TextSource::setCurrentBrushColorUsingHsvShift(LinearGradientBrush* brushp, uint32_t cl1, uint32_t cl2, TextModifier &tmodifier) {
	brushp->SetLinearColors(Color(calc_color(cl1, opacity, tmodifier.hueShift + hueQuickShift, tmodifier.saturationShift + saturationQuickShift, tmodifier.valueShift + valueQuickShift)), Color(calc_color(cl2, opacity2, tmodifier.hueShift + hueQuickShift, tmodifier.saturationShift + saturationQuickShift, tmodifier.valueShift + valueQuickShift)));
}


void TextSource::RenderText()
{
	StringFormat format(StringFormat::GenericTypographic());
	Status stat;

	RectF box;
	RectF linebox;
	SIZE size;
	LinearGradientBrush *brushp;
	//
	TextModifier tmodifier;

	GetStringFormat(format);
	CalculateTextSizes(format, box, size);

	unique_ptr<uint8_t[]> bits(new uint8_t[size.cx * size.cy * 4]);
	Bitmap bitmap(size.cx, size.cy, 4 * size.cx, PixelFormat32bppARGB,
		      bits.get());

	Graphics graphics_bitmap(&bitmap);

	LinearGradientBrush brush(RectF(0, 0, (float)size.cx, (float)size.cy),
		Color(calc_color(color, opacity, 0,0,0)),
		Color(calc_color(color2, opacity2, 0,0,0)),
		gradient_dir, 1);
	brushp = &brush;

	DWORD full_bk_color = bk_color & 0xFFFFFF;

	if (!text.empty() || use_extents)
		full_bk_color |= get_alpha_val(bk_opacity);

	if ((size.cx > box.Width || size.cy > box.Height) && !use_extents) {
		stat = graphics_bitmap.Clear(Color(0));
		warn_stat("graphics_bitmap.Clear");

		SolidBrush bk_brush = Color(full_bk_color);
		stat = graphics_bitmap.FillRectangle(&bk_brush, box);
		warn_stat("graphics_bitmap.FillRectangle");
	} else {
		stat = graphics_bitmap.Clear(Color(full_bk_color));
		warn_stat("graphics_bitmap.Clear");
	}

	graphics_bitmap.SetCompositingMode(CompositingModeSourceOver);
	SetAntiAliasing(graphics_bitmap);

	if (!text.empty()) {

		if (multineTweakEnable) {
			// new method does homebrew word wrap and height line adjustmane
			std::wstring tempText = text;
			std::wstring oneLine;
			RectF bounding_box_temp;
			int lineNumber = 1;
			tmodifier.reset();

			// kludge little buffer on left hand side

			flagClearNextLineFontSizeAdjustment = true;
			float currentX = 0.0f;
			float currentY = 0.0f;
			bool flagDeleteBrush = false;
			//
			float marginFontSizeOffsetScale = 0.055f;
			float leftMarginX = (float)face_size * marginFontSizeOffsetScale;
			box.X = leftMarginX;

			if (align == Align::Right) {
				box.X = 0-leftMarginX;
				box.X = box.X - paddingLeft;
			}
			//
			uint32_t rcolor1 = color;
			uint32_t rcolor2 = color2;
			//uint32_t lastColor1 = rcolor1;
			//uint32_t lastColor2 = rcolor2;
			uint32_t lastColor1 = 0;
			uint32_t lastColor2 = 0;
			//
			// padding
			currentY += paddingTop;
			//
			while (jrSplitWStringLine(tempText, oneLine, tmodifier)) {
				if (oneLine.length() >= 2 && oneLine[0] == L'/' && oneLine[1] == L'/') {
					// comment
					continue;
				}
				UpdateFont(tmodifier, false);

				// currentX - try to offset tweak based on font
				float fsize = (float)face_size * tmodifier.fontSizeAdjustment;
				float leftMarginX = fsize * marginFontSizeOffsetScale;
				currentX = box.X - leftMarginX;
				currentX += paddingLeft;
				currentX += tmodifier.xoff;
				currentY += tmodifier.yoff;
				// we clear yoff each time
				tmodifier.yoff = 0;


				// a kludge for first line spacing adjustment, so user can use lineheight adjustment on first line to shift entire text up or down
				// ATTN: we now 3/21/23 have a y offset option that i think should eliminate need for this kludge?
				if (false && lineNumber == 1) {
					currentY += (lineHeight[0] - 1.0f) * lineHeight[1];
				}

				// the area this line will be drawn into
				linebox = RectF(currentX, currentY, box.Width, lineHeightOrig[lineNumber]);

				rcolor1 = (tmodifier.color1 != 0) ? tmodifier.color1 : color;
				rcolor2 = (tmodifier.color2 != 0) ? tmodifier.color2 : color2;
				if (!gradient) {
					rcolor2 = rcolor1;
				}



				int fHue = tmodifier.hueShift + hueQuickShift;
				int fSaturation = tmodifier.saturationShift + saturationQuickShift;
				int fValue = tmodifier.valueShift + valueQuickShift;


				//blog(LOG_WARNING, " hue shift = %d last was %d and tmodhue = %d and quick = %d.", fHue, lastHueShift, tmodifier.hueShift, hueQuickShift);
				if (gradientPerLine) {
					brushp = new LinearGradientBrush(RectF(linebox.X, linebox.Y, (float)size.cx, lineHeightOrig[lineNumber]),
						Color(calc_color(rcolor1, opacity, fHue, fSaturation, fValue)),
						Color(calc_color(rcolor2, opacity2, fHue, fSaturation, fValue)),
						gradient_dir, 1);
					flagDeleteBrush = true;
				} else {
					if (true || lastColor1 != rcolor1 || lastColor2 != rcolor2) {
						setCurrentBrushColorUsingHsvShift(brushp, rcolor1, rcolor2, tmodifier);
						}
				}
				lastColor1 = rcolor1;
				lastColor2 = rcolor2;

				if (use_outline) {
					// too close to left margin
					//box.Offset(outline_size / 2, outline_size / 2);
					box.Offset(outline_size, outline_size / 2);

					FontFamily family;
					GraphicsPath path;

					font->GetFamily(&family);
					stat = path.AddString(oneLine.c_str(), (int)oneLine.size(),
						&family, font->GetStyle(),
						font->GetSize(), linebox, &format);
					warn_stat("path.AddString");

					RenderOutlineText(graphics_bitmap, path, *brushp, tmodifier);
				}
				else {
					stat = graphics_bitmap.DrawString(oneLine.c_str(),
						(int)oneLine.size(),
						font.get(), linebox,
						&format, brushp);
					warn_stat("graphics_bitmap.DrawString");
				}
				// now adjust y
				currentY += lineHeight[lineNumber];
				if (lineNumber < DEF_MAX_LINES) {
					++lineNumber;
				}
				if (flagDeleteBrush) {
					delete brushp;
					brushp = NULL;
				}
			}
		} else {
			if (use_outline) {
				box.Offset(outline_size / 2, outline_size / 2);

				FontFamily family;
				GraphicsPath path;

				font->GetFamily(&family);
				stat = path.AddString(text.c_str(),
						      (int)text.size(), &family,
						      font->GetStyle(),
						      font->GetSize(), box,
						      &format);
				warn_stat("path.AddString");

				RenderOutlineText(graphics_bitmap, path, *brushp, tmodifier);
			} else {
				stat = graphics_bitmap.DrawString(
					text.c_str(), (int)text.size(),
					font.get(), box, &format, brushp);
				warn_stat("graphics_bitmap.DrawString");
			}

		}
	}



	if (!tex || (LONG)cx != size.cx || (LONG)cy != size.cy) {
		obs_enter_graphics();
		if (tex)
			gs_texture_destroy(tex);

		const uint8_t *data = (uint8_t *)bits.get();
		tex = gs_texture_create(size.cx, size.cy, GS_BGRA, 1, &data,
					GS_DYNAMIC);

		obs_leave_graphics();

		cx = (uint32_t)size.cx;
		cy = (uint32_t)size.cy;

	} else if (tex) {
		obs_enter_graphics();
		gs_texture_set_image(tex, bits.get(), size.cx * 4, false);
		obs_leave_graphics();
	}
}

const char *TextSource::GetMainString(const char *str)
{
	if (!str)
		return "";
	if (!chatlog_mode || !chatlog_lines)
		return str;

	int lines = chatlog_lines;
	size_t len = strlen(str);
	if (!len)
		return str;

	const char *temp = str + len;

	while (temp != str) {
		temp--;

		if (temp[0] == '\n' && temp[1] != 0) {
			if (!--lines)
				break;
		}
	}

	return *temp == '\n' ? temp + 1 : temp;
}

void TextSource::LoadFileText()
{
	BPtr<char> file_text = os_quick_read_utf8_file(file.c_str());
	text = to_wide(GetMainString(file_text));

	if (!text.empty() && text.back() != '\n')
		text.push_back('\n');
}

void TextSource::TransformText()
{
	const locale loc = locale(obs_get_locale());
	const ctype<wchar_t> &f = use_facet<ctype<wchar_t>>(loc);
	if (text_transform == S_TRANSFORM_UPPERCASE)
		f.toupper(&text[0], &text[0] + text.size());
	else if (text_transform == S_TRANSFORM_LOWERCASE)
		f.tolower(&text[0], &text[0] + text.size());
	else if (text_transform == S_TRANSFORM_STARTCASE) {
		bool upper = true;
		for (wstring::iterator it = text.begin(); it != text.end();
		     ++it) {
			const wchar_t upper_char = f.toupper(*it);
			const wchar_t lower_char = f.tolower(*it);
			if (upper && lower_char != upper_char) {
				upper = false;
				*it = upper_char;
			} else if (lower_char != upper_char) {
				*it = lower_char;
			} else {
				upper = iswspace(*it);
			}
		}
	}
}

void TextSource::SetAntiAliasing(Graphics &graphics_bitmap)
{
	if (!antialiasing) {
		graphics_bitmap.SetTextRenderingHint(
			TextRenderingHintSingleBitPerPixel);
		graphics_bitmap.SetSmoothingMode(SmoothingModeNone);
		return;
	}

	graphics_bitmap.SetTextRenderingHint(TextRenderingHintAntiAlias);
	graphics_bitmap.SetSmoothingMode(SmoothingModeAntiAlias);
}

#define obs_data_get_uint32 (uint32_t) obs_data_get_int

inline void TextSource::Update(obs_data_t *s)
{
	const char *new_text = obs_data_get_string(s, S_TEXT);
	obs_data_t *font_obj = obs_data_get_obj(s, S_FONT);
	const char *align_str = obs_data_get_string(s, S_ALIGN);
	const char *valign_str = obs_data_get_string(s, S_VALIGN);
	uint32_t new_color = obs_data_get_uint32(s, S_COLOR);
	uint32_t new_opacity = obs_data_get_uint32(s, S_OPACITY);
	bool new_gradient = obs_data_get_bool(s, S_GRADIENT);
	uint32_t new_color2 = obs_data_get_uint32(s, S_GRADIENT_COLOR);
	uint32_t new_opacity2 = obs_data_get_uint32(s, S_GRADIENT_OPACITY);
	float new_grad_dir = (float)obs_data_get_double(s, S_GRADIENT_DIR);
	bool new_vertical = obs_data_get_bool(s, S_VERTICAL);
	bool new_outline = obs_data_get_bool(s, S_OUTLINE);
	uint32_t new_o_color = obs_data_get_uint32(s, S_OUTLINE_COLOR);
	uint32_t new_o_opacity = obs_data_get_uint32(s, S_OUTLINE_OPACITY);
	uint32_t new_o_size = obs_data_get_uint32(s, S_OUTLINE_SIZE);
	bool new_use_file = obs_data_get_bool(s, S_USE_FILE);
	const char *new_file = obs_data_get_string(s, S_FILE);
	bool new_chat_mode = obs_data_get_bool(s, S_CHATLOG_MODE);
	int new_chat_lines = (int)obs_data_get_int(s, S_CHATLOG_LINES);
	bool new_extents = obs_data_get_bool(s, S_EXTENTS);
	bool new_extents_wrap = obs_data_get_bool(s, S_EXTENTS_WRAP);
	uint32_t n_extents_cx = obs_data_get_uint32(s, S_EXTENTS_CX);
	uint32_t n_extents_cy = obs_data_get_uint32(s, S_EXTENTS_CY);
	int new_text_transform = (int)obs_data_get_int(s, S_TRANSFORM);
	bool new_antialiasing = obs_data_get_bool(s, S_ANTIALIASING);

	const char *font_face = obs_data_get_string(font_obj, "face");
	int font_size = (int)obs_data_get_int(font_obj, "size");
	int64_t font_flags = obs_data_get_int(font_obj, "flags");
	bool new_bold = (font_flags & OBS_FONT_BOLD) != 0;
	bool new_italic = (font_flags & OBS_FONT_ITALIC) != 0;
	bool new_underline = (font_flags & OBS_FONT_UNDERLINE) != 0;
	bool new_strikeout = (font_flags & OBS_FONT_STRIKEOUT) != 0;

	uint32_t new_bk_color = obs_data_get_uint32(s, S_BKCOLOR);
	uint32_t new_bk_opacity = obs_data_get_uint32(s, S_BKOPACITY);

	multineTweakEnable = obs_data_get_bool(s, S_MULTILINE);
	tweakWraplen = (int)obs_data_get_int(s, S_WRAPLEN);
	tweakHeightAdjust = 1.0f + (float)obs_data_get_int(s, S_LINESPACEADJUST) / 100.0f;
	gradientPerLine = obs_data_get_bool(s, S_GRADIENTPERLINE);
	fontSizeQuickScale = (float)obs_data_get_int(s, S_FontSizeQuickScale) / 100.0f;
	hueQuickShift = (int)obs_data_get_int(s, S_HueQuickShift);
	saturationQuickShift = (int)obs_data_get_int(s, S_SaturationQuickShift);
	valueQuickShift = (int)obs_data_get_int(s, S_ValueQuickShift);

	/* ----------------------------- */
	TextModifier tmodifier;

	wstring new_face = to_wide(font_face);

	if (new_face != face || face_size != font_size || new_bold != bold ||
	    new_italic != italic || new_underline != underline ||
	    new_strikeout != strikeout) {

		face = new_face;
		face_size = font_size;
		bold = new_bold;
		italic = new_italic;
		underline = new_underline;
		strikeout = new_strikeout;

		UpdateFont(tmodifier, true);
	}
	else {
		// should we still update the font here in case theyve changed quick scale? I don't think we need to
		//UpdateFont(tmodifier, false);
	}

	/* ----------------------------- */

	new_color = rgb_to_bgr(new_color);
	new_color2 = rgb_to_bgr(new_color2);
	new_o_color = rgb_to_bgr(new_o_color);
	new_bk_color = rgb_to_bgr(new_bk_color);

	gradient = new_gradient;
	color = new_color;
	opacity = new_opacity;
	color2 = new_color2;
	opacity2 = new_opacity2;
	gradient_dir = new_grad_dir;
	vertical = new_vertical;

	bk_color = new_bk_color;
	bk_opacity = new_bk_opacity;
	use_extents = new_extents;
	wrap = new_extents_wrap;
	extents_cx = n_extents_cx;
	extents_cy = n_extents_cy;
	text_transform = new_text_transform;
	antialiasing = new_antialiasing;

	if (!gradient) {
		color2 = color;
		opacity2 = opacity;
	}

	read_from_file = new_use_file;

	chatlog_mode = new_chat_mode;
	chatlog_lines = new_chat_lines;

	if (read_from_file) {
		file = new_file;
		file_timestamp = get_modified_timestamp(new_file);
		LoadFileText();

	} else {
		text = to_wide(GetMainString(new_text));

		/* all text should end with newlines due to the fact that GDI+
		 * treats strings without newlines differently in terms of
		 * render size */
		if (!text.empty())
			text.push_back('\n');
	}
	TransformText();

	use_outline = new_outline;
	outline_color = new_o_color;
	outline_opacity = new_o_opacity;
	outline_size = roundf(float(new_o_size));

	if (strcmp(align_str, S_ALIGN_CENTER) == 0)
		align = Align::Center;
	else if (strcmp(align_str, S_ALIGN_RIGHT) == 0)
		align = Align::Right;
	else
		align = Align::Left;

	if (strcmp(valign_str, S_VALIGN_CENTER) == 0)
		valign = VAlign::Center;
	else if (strcmp(valign_str, S_VALIGN_BOTTOM) == 0)
		valign = VAlign::Bottom;
	else
		valign = VAlign::Top;

	RenderText();
	update_time_elapsed = 0.0f;

	/* ----------------------------- */

	obs_data_release(font_obj);
}

inline void TextSource::Tick(float seconds)
{
	if (!read_from_file)
		return;

	update_time_elapsed += seconds;

	if (update_time_elapsed >= 1.0f) {
		time_t t = get_modified_timestamp(file.c_str());
		update_time_elapsed = 0.0f;

		if (update_file) {
			LoadFileText();
			TransformText();
			RenderText();
			update_file = false;
		}

		if (file_timestamp != t) {
			file_timestamp = t;
			update_file = true;
		}
	}
}

inline void TextSource::Render()
{
	if (!tex)
		return;

	gs_effect_t *effect = obs_get_base_effect(OBS_EFFECT_DEFAULT);
	gs_technique_t *tech = gs_effect_get_technique(effect, "Draw");

	const bool previous = gs_framebuffer_srgb_enabled();
	gs_enable_framebuffer_srgb(true);

	gs_technique_begin(tech);
	gs_technique_begin_pass(tech, 0);

	gs_effect_set_texture_srgb(gs_effect_get_param_by_name(effect, "image"),
				   tex);
	gs_draw_sprite(tex, 0, cx, cy);

	gs_technique_end_pass(tech);
	gs_technique_end(tech);

	gs_enable_framebuffer_srgb(previous);
}





uint32_t TextSource::parseColorStr(std::wstring ws) {
	//return 0xFF4466;

	//blog(LOG_WARNING, "parseColorStr: %ls", ws);

	//int r, g, b;
	int r, g, b;
	int parsedCount = swscanf(ws.c_str(), L"#%02x%02x%02x", &r, &g, &b);
	if (parsedCount != 3) {
		return 0;
	}
	//blog(LOG_WARNING, " parsed %d, %d, %d.", r, g, b);
	uint32_t cl = ((uint32_t)r << 16 | (uint32_t)g << 8 | (uint32_t)b);
	//uint32_t cl = ((uint32_t)r << 16 | (uint32_t)g << 8 | (uint32_t)b);
	//uint32_t cl = 0xFF000000 | (uint32_t)r | ((uint32_t)b << 16) | ((uint32_t)g << 8);
	//blog(LOG_WARNING, " color: #%08x", cl);
	return cl;
}














/* ------------------------------------------------------------------------- */

static ULONG_PTR gdip_token = 0;

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-text", "en-US")
MODULE_EXPORT const char *obs_module_description(void)
{
	return "Windows GDI+ text source";
}

#define set_vis(var, val, show)                           \
	do {                                              \
		p = obs_properties_get(props, val);       \
		obs_property_set_visible(p, var == show); \
	} while (false)

static bool use_file_changed(obs_properties_t *props, obs_property_t *p,
			     obs_data_t *s)
{
	bool use_file = obs_data_get_bool(s, S_USE_FILE);

	set_vis(use_file, S_TEXT, false);
	set_vis(use_file, S_FILE, true);
	return true;
}

static bool outline_changed(obs_properties_t *props, obs_property_t *p,
			    obs_data_t *s)
{
	bool outline = obs_data_get_bool(s, S_OUTLINE);

	set_vis(outline, S_OUTLINE_SIZE, true);
	set_vis(outline, S_OUTLINE_COLOR, true);
	set_vis(outline, S_OUTLINE_OPACITY, true);
	return true;
}

static bool chatlog_mode_changed(obs_properties_t *props, obs_property_t *p,
				 obs_data_t *s)
{
	bool chatlog_mode = obs_data_get_bool(s, S_CHATLOG_MODE);

	set_vis(chatlog_mode, S_CHATLOG_LINES, true);
	return true;
}

static bool gradient_changed(obs_properties_t *props, obs_property_t *p,
			     obs_data_t *s)
{
	bool gradient = obs_data_get_bool(s, S_GRADIENT);

	set_vis(gradient, S_GRADIENT_COLOR, true);
	set_vis(gradient, S_GRADIENT_OPACITY, true);
	set_vis(gradient, S_GRADIENT_DIR, true);
	return true;
}

static bool extents_modified(obs_properties_t *props, obs_property_t *p,
			     obs_data_t *s)
{
	bool use_extents = obs_data_get_bool(s, S_EXTENTS);

	set_vis(use_extents, S_EXTENTS_WRAP, true);
	set_vis(use_extents, S_EXTENTS_CX, true);
	set_vis(use_extents, S_EXTENTS_CY, true);
	return true;
}


static bool tweaks_modified(obs_properties_t *props, obs_property_t *p,
			     obs_data_t *s)
{
	return true;
}


#undef set_vis

static obs_properties_t *get_properties(void *data)
{
	TextSource *s = reinterpret_cast<TextSource *>(data);
	string path;

	obs_properties_t *props = obs_properties_create();
	obs_property_t *p;

	obs_properties_add_font(props, S_FONT, T_FONT);

	p = obs_properties_add_bool(props, S_USE_FILE, T_USE_FILE);
	obs_property_set_modified_callback(p, use_file_changed);

	string filter;
	filter += T_FILTER_TEXT_FILES;
	filter += " (*.txt);;";
	filter += T_FILTER_ALL_FILES;
	filter += " (*.*)";

	if (s && !s->file.empty()) {
		const char *slash;

		path = s->file;
		replace(path.begin(), path.end(), '\\', '/');
		slash = strrchr(path.c_str(), '/');
		if (slash)
			path.resize(slash - path.c_str() + 1);
	}

	obs_properties_add_text(props, S_TEXT, T_TEXT, OBS_TEXT_MULTILINE);
	obs_properties_add_path(props, S_FILE, T_FILE, OBS_PATH_FILE,
				filter.c_str(), path.c_str());

	obs_properties_add_bool(props, S_ANTIALIASING, T_ANTIALIASING);


	obs_properties_t* propgroup = obs_properties_create();
	obs_properties_add_group(props, "tweaks", S_MULTILINEGROUP_TEXT, OBS_GROUP_NORMAL, propgroup);
	p=obs_properties_add_bool(propgroup, S_MULTILINE, S_MULTILINE_TEXT);
	p=obs_properties_add_bool(propgroup, S_GRADIENTPERLINE, S_GRADIENTPERLINE_TEXT);
	p=obs_properties_add_int_slider(propgroup, S_WRAPLEN, S_WRAPLEN_TEXT, 0, 255, 1);
	p=obs_properties_add_int_slider(propgroup, S_LINESPACEADJUST, S_LINESPACEADJUST_TEXT, -90, 90, 1);
	//
	p=obs_properties_add_int_slider(propgroup, S_FontSizeQuickScale, S_FontSizeQuickScale_TEXT, 25, 200, 1);
	p=obs_properties_add_int_slider(propgroup, S_HueQuickShift, S_HueQuickShift_TEXT, -360, 360, 1);
	p=obs_properties_add_int_slider(propgroup, S_SaturationQuickShift, S_SaturationQuickShift_TEXT, -360, 360, 1);
	p=obs_properties_add_int_slider(propgroup, S_ValueQuickShift, S_ValueQuickShift_TEXT, -360, 360, 1);
	//
	p=obs_properties_add_text(propgroup, S_NOTES, S_NOTES_TEXT, OBS_TEXT_MULTILINE);



	p = obs_properties_add_list(props, S_TRANSFORM, T_TRANSFORM,
				    OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(p, T_TRANSFORM_NONE, S_TRANSFORM_NONE);
	obs_property_list_add_int(p, T_TRANSFORM_UPPERCASE,
				  S_TRANSFORM_UPPERCASE);
	obs_property_list_add_int(p, T_TRANSFORM_LOWERCASE,
				  S_TRANSFORM_LOWERCASE);
	obs_property_list_add_int(p, T_TRANSFORM_STARTCASE,
				  S_TRANSFORM_STARTCASE);

	obs_properties_add_bool(props, S_VERTICAL, T_VERTICAL);

	obs_properties_add_color(props, S_COLOR, T_COLOR);
	p = obs_properties_add_int_slider(props, S_OPACITY, T_OPACITY, 0, 100,
					  1);
	obs_property_int_set_suffix(p, "%");

	p = obs_properties_add_bool(props, S_GRADIENT, T_GRADIENT);
	obs_property_set_modified_callback(p, gradient_changed);

	obs_properties_add_color(props, S_GRADIENT_COLOR, T_GRADIENT_COLOR);
	p = obs_properties_add_int_slider(props, S_GRADIENT_OPACITY,
					  T_GRADIENT_OPACITY, 0, 100, 1);
	obs_property_int_set_suffix(p, "%");
	obs_properties_add_float_slider(props, S_GRADIENT_DIR, T_GRADIENT_DIR,
					0, 360, 0.1);

	obs_properties_add_color(props, S_BKCOLOR, T_BKCOLOR);
	p = obs_properties_add_int_slider(props, S_BKOPACITY, T_BKOPACITY, 0,
					  100, 1);
	obs_property_int_set_suffix(p, "%");

	p = obs_properties_add_list(props, S_ALIGN, T_ALIGN,
				    OBS_COMBO_TYPE_LIST,
				    OBS_COMBO_FORMAT_STRING);
	obs_property_list_add_string(p, T_ALIGN_LEFT, S_ALIGN_LEFT);
	obs_property_list_add_string(p, T_ALIGN_CENTER, S_ALIGN_CENTER);
	obs_property_list_add_string(p, T_ALIGN_RIGHT, S_ALIGN_RIGHT);

	p = obs_properties_add_list(props, S_VALIGN, T_VALIGN,
				    OBS_COMBO_TYPE_LIST,
				    OBS_COMBO_FORMAT_STRING);
	obs_property_list_add_string(p, T_VALIGN_TOP, S_VALIGN_TOP);
	obs_property_list_add_string(p, T_VALIGN_CENTER, S_VALIGN_CENTER);
	obs_property_list_add_string(p, T_VALIGN_BOTTOM, S_VALIGN_BOTTOM);

	p = obs_properties_add_bool(props, S_OUTLINE, T_OUTLINE);
	obs_property_set_modified_callback(p, outline_changed);

	obs_properties_add_int(props, S_OUTLINE_SIZE, T_OUTLINE_SIZE, 1, 20, 1);
	obs_properties_add_color(props, S_OUTLINE_COLOR, T_OUTLINE_COLOR);
	p = obs_properties_add_int_slider(props, S_OUTLINE_OPACITY,
					  T_OUTLINE_OPACITY, 0, 100, 1);
	obs_property_int_set_suffix(p, "%");

	p = obs_properties_add_bool(props, S_CHATLOG_MODE, T_CHATLOG_MODE);
	obs_property_set_modified_callback(p, chatlog_mode_changed);

	obs_properties_add_int(props, S_CHATLOG_LINES, T_CHATLOG_LINES, 1, 1000,
			       1);

	p = obs_properties_add_bool(props, S_EXTENTS, T_EXTENTS);
	obs_property_set_modified_callback(p, extents_modified);

	obs_properties_add_int(props, S_EXTENTS_CX, T_EXTENTS_CX, 32, 8000, 1);
	obs_properties_add_int(props, S_EXTENTS_CY, T_EXTENTS_CY, 32, 8000, 1);
	obs_properties_add_bool(props, S_EXTENTS_WRAP, T_EXTENTS_WRAP);


	return props;
}

static void defaults(obs_data_t *settings, int ver)
{
	obs_data_t *font_obj = obs_data_create();
	obs_data_set_default_string(font_obj, "face", "Arial");
	obs_data_set_default_int(font_obj, "size", ver == 1 ? 36 : 256);

	obs_data_set_default_obj(settings, S_FONT, font_obj);
	obs_data_set_default_string(settings, S_ALIGN, S_ALIGN_LEFT);
	obs_data_set_default_string(settings, S_VALIGN, S_VALIGN_TOP);
	obs_data_set_default_int(settings, S_COLOR, 0xFFFFFF);
	obs_data_set_default_int(settings, S_OPACITY, 100);
	obs_data_set_default_int(settings, S_GRADIENT_COLOR, 0xFFFFFF);
	obs_data_set_default_int(settings, S_GRADIENT_OPACITY, 100);
	obs_data_set_default_double(settings, S_GRADIENT_DIR, 90.0);
	obs_data_set_default_int(settings, S_BKCOLOR, 0x000000);
	obs_data_set_default_int(settings, S_BKOPACITY, 0);
	obs_data_set_default_int(settings, S_OUTLINE_SIZE, 2);
	obs_data_set_default_int(settings, S_OUTLINE_COLOR, 0xFFFFFF);
	obs_data_set_default_int(settings, S_OUTLINE_OPACITY, 100);
	obs_data_set_default_int(settings, S_CHATLOG_LINES, 6);
	obs_data_set_default_bool(settings, S_EXTENTS_WRAP, true);
	obs_data_set_default_int(settings, S_EXTENTS_CX, 100);
	obs_data_set_default_int(settings, S_EXTENTS_CY, 100);
	obs_data_set_default_int(settings, S_TRANSFORM, S_TRANSFORM_NONE);
	obs_data_set_default_bool(settings, S_ANTIALIASING, true);

	obs_data_set_default_bool(settings, S_MULTILINE, S_MULTILINE_DEF);
	obs_data_set_default_bool(settings, S_GRADIENTPERLINE, S_GRADIENTPERLINE_DEF);
	obs_data_set_default_int(settings, S_WRAPLEN, S_WRAPLEN_DEF);
	obs_data_set_default_int(settings, S_LINESPACEADJUST, S_LINESPACEADJUST_DEF);
	obs_data_set_default_int(settings, S_HueQuickShift, S_HueQuickShift_DEF);
	obs_data_set_default_int(settings, S_SaturationQuickShift, S_SaturationQuickShift_DEF);
	obs_data_set_default_int(settings, S_ValueQuickShift, S_ValueQuickShift_DEF);
	obs_data_set_default_int(settings, S_FontSizeQuickScale, S_FontSizeQuickScale_DEF);

	obs_data_release(font_obj);
};

static void missing_file_callback(void *src, const char *new_path, void *data)
{
	TextSource *s = reinterpret_cast<TextSource *>(src);

	obs_source_t *source = s->source;
	obs_data_t *settings = obs_source_get_settings(source);
	obs_data_set_string(settings, S_FILE, new_path);
	obs_source_update(source, settings);
	obs_data_release(settings);

	UNUSED_PARAMETER(data);
}

bool obs_module_load(void)
{
	obs_source_info si = {};
	si.id = "text_gdiplusml";
	si.type = OBS_SOURCE_TYPE_INPUT;
	si.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW |
			  OBS_SOURCE_CAP_OBSOLETE | OBS_SOURCE_SRGB;
	si.get_properties = get_properties;
	si.icon_type = OBS_ICON_TYPE_TEXT;

	si.get_name = [](void *) { return obs_module_text("TextGDIPlusML"); };
	si.create = [](obs_data_t *settings, obs_source_t *source) {
		return (void *)new TextSource(source, settings);
	};
	si.destroy = [](void *data) {
		delete reinterpret_cast<TextSource *>(data);
	};
	si.get_width = [](void *data) {
		return reinterpret_cast<TextSource *>(data)->cx;
	};
	si.get_height = [](void *data) {
		return reinterpret_cast<TextSource *>(data)->cy;
	};
	si.get_defaults = [](obs_data_t *settings) { defaults(settings, 1); };
	si.update = [](void *data, obs_data_t *settings) {
		reinterpret_cast<TextSource *>(data)->Update(settings);
	};
	si.video_tick = [](void *data, float seconds) {
		reinterpret_cast<TextSource *>(data)->Tick(seconds);
	};
	si.video_render = [](void *data, gs_effect_t *) {
		reinterpret_cast<TextSource *>(data)->Render();
	};
	si.missing_files = [](void *data) {
		TextSource *s = reinterpret_cast<TextSource *>(data);
		obs_missing_files_t *files = obs_missing_files_create();

		obs_source_t *source = s->source;
		obs_data_t *settings = obs_source_get_settings(source);

		bool read = obs_data_get_bool(settings, S_USE_FILE);
		const char *path = obs_data_get_string(settings, S_FILE);

		if (read && strcmp(path, "") != 0) {
			if (!os_file_exists(path)) {
				obs_missing_file_t *file =
					obs_missing_file_create(
						path, missing_file_callback,
						OBS_MISSING_FILE_SOURCE,
						s->source, NULL);

				obs_missing_files_add_file(files, file);
			}
		}

		obs_data_release(settings);

		return files;
	};

	obs_source_info si_v2 = si;
	si_v2.version = 2;
	si_v2.output_flags &= ~OBS_SOURCE_CAP_OBSOLETE;
	si_v2.get_defaults = [](obs_data_t *settings) {
		defaults(settings, 2);
	};

	obs_register_source(&si);
	obs_register_source(&si_v2);

	const GdiplusStartupInput gdip_input;
	GdiplusStartup(&gdip_token, &gdip_input, nullptr);
	return true;
}

void obs_module_unload(void)
{
	GdiplusShutdown(gdip_token);
}



