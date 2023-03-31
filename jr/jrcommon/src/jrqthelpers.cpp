#include "jrqthelpers.hpp"

#include <../qt-wrappers.hpp>







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
void setThemeID(QWidget *widget, const QString &themeID) {
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





//---------------------------------------------------------------------------
QString splitOffRightWord(QString &str, QString splitPattern) {
	// if pattern not found, return original
	// otherwise return the split word and set str to remainder, trimming both
	int pos = str.lastIndexOf(splitPattern);
	if (pos == -1) {
		return "";
	}
	QString splitPart = str.sliced(pos+1);
	str = str.sliced(0,pos);
	//
	splitPart = splitPart.trimmed();
	str = str.trimmed();
	return splitPart;
}

QString splitOffLeftWord(QString &str, QString splitPattern) {
	// if pattern not found, return original
	// otherwise return the split word and set str to remainder, trimming both
	int pos = str.indexOf(splitPattern);
	if (pos == -1) {
		return "";
	}
	QString splitPart = str.sliced(0, pos);
	str = str.sliced(pos+1);
	//
	splitPart = splitPart.trimmed();
	str = str.trimmed();
	return splitPart;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
QString sanitizeMessageString(const QString &str) {
	return str.toHtmlEscaped();
}
//---------------------------------------------------------------------------
