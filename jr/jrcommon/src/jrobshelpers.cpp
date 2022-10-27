#include "jrobshelpers.hpp"

#include <../obs-frontend-api/obs-frontend-api.h>
#include <../qt-wrappers.hpp>

#include <ctime>




//---------------------------------------------------------------------------
// see https://www.techiedelight.com/split-a-string-on-newlines-in-cpp/
std::vector<std::string> splitString(const std::string& str, bool skipBlankLines)
{
    std::vector<std::string> tokens;
    std::string astr;
 
    std::string::size_type pos = 0;
    std::string::size_type prev = 0;
    while ((pos = str.find('\n', prev)) != std::string::npos) {
	    astr = str.substr(prev, pos - prev);
	    if (astr != "" || !skipBlankLines) {
		    tokens.push_back(astr);
	    }
        prev = pos + 1;
    }
    astr = str.substr(prev);
    if (astr != "" || !skipBlankLines) {
	    tokens.push_back(astr);
    }
 
    return tokens;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
std::string calcSecsAsNiceTimeString(unsigned long secs, bool flagPadLeadingZeros) {
	unsigned long mins = (unsigned long) (secs / 60L);
	secs = secs % 60;
	unsigned long hours = (unsigned long) (mins / 60L);
	mins = mins % 60;
	//
	char str[24];
	if (hours > 0) {
		if (flagPadLeadingZeros) {
			sprintf(str, "%02lu:%02lu:%02lu", hours, mins, secs);
		} else {
			sprintf(str, "%lu:%02lu:%02lu", hours, mins, secs);

		}
	} else {
		if (flagPadLeadingZeros) {
			sprintf(str, "%02lu:%02lu", mins, secs);
		} else {
			sprintf(str, "%lu:%02lu", mins, secs);
		}
	}
	//
	return std::string(str);
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
std::string getCurrentDateTimeAsNiceString() {
	char timestr[80];
	time_t temp;
	struct tm *timeptr;
	temp = time(NULL);
	timeptr = localtime(&temp);
	strftime(timestr, 80, "%A, %d %b %Y at %I:%M %p", timeptr);
	return std::string(timestr);
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
bool doesStringMatchAnyItemsInPatternList(const std::string needle, std::vector<std::string> *haystackVectorp) {
	const char* needleCharp = needle.c_str();
	for (std::vector<std::string>::iterator t = haystackVectorp->begin(); t != haystackVectorp->end(); ++t) {
		const char* patternCharp = t->c_str();
		if (strcmp(patternCharp, "") == 0) {
			continue;
		}
		if (strstr(needleCharp, patternCharp) != NULL) {
			return true;
		}
	}
	// not found
	return false;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void addSpacerToLayout(QLayout *layout) {
	QSpacerItem* qspacer = new QSpacerItem(1,12);
	layout->addItem(qspacer);
}


void addLineSeparatorToLayout(QLayout *layout, int paddingTop, int paddingBot) {
	QFrame* line = new QFrame();
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);

	QSpacerItem* qspacerTop = new QSpacerItem(1,paddingTop);
	layout->addItem(qspacerTop);
	//
	layout->addWidget(line);
	//
	QSpacerItem* qspacerBot = new QSpacerItem(1,paddingBot);
	layout->addItem(qspacerBot);
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void setThemeID(QWidget *widget, const QString &themeID)
{
	if (widget->property("themeID").toString() != themeID) {
		widget->setProperty("themeID", themeID);

		/* force style sheet recalculation */
		QString qss = widget->styleSheet();
		widget->setStyleSheet("/* */");
		widget->setStyleSheet(qss);
	}
}

bool WindowPositionValid(QRect rect)
{
	//QCefWidget *browser;

	for (QScreen *screen : QGuiApplication::screens()) {
		if (screen->availableGeometry().intersects(rect))
			return true;
	}
	return false;
}
//---------------------------------------------------------------------------



