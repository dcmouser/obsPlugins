#include "jrqthelpers.hpp"

#include <../qt-wrappers.hpp>
#include <QFileDialog>






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


//---------------------------------------------------------------------------
void showModalQtDialog(const QString& title, const QString& msg) {
	QMessageBox msgBox;
	msgBox.setText(title + ": "+ msg);
	msgBox.exec();
}

void showModalQtDialogError(const QString& msg) {
	showModalQtDialog(QString("ERROR"), msg);
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
qint64 qtHelpLaunchCommandline(const QString &str, bool optionStartMinimized) {
	QProcess process;
	qint64 pid = 0;


	QString comlineq = str;
	QStringList argumentsList =  QProcess::splitCommand(comlineq);
	if (argumentsList.count() < 1) {
		//mydebug("No arguments found in commandline command, not launching.");
		return pid;
	}
	QString program = argumentsList.takeFirst();

	process.setCreateProcessArgumentsModifier(
			[optionStartMinimized](QProcess::CreateProcessArguments* args) {
				args->flags |= CREATE_NEW_CONSOLE;
				args->startupInfo->dwFlags &= ~STARTF_USESTDHANDLES;
				if (optionStartMinimized) {
					args->startupInfo->wShowWindow |= SW_SHOWMINIMIZED;
					args->startupInfo->dwFlags |= STARTF_USESHOWWINDOW;
				}
			});

	process.setProgram(program);
	process.setArguments(argumentsList);
	//
	process.startDetached(&pid);

	return pid;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// from qt-wrappers.cpp
QString SaveFile(QWidget *parent, QString title, QString path,
		 QString extensions)
{
	QString file =
		QFileDialog::getSaveFileName(parent, title, path, extensions);

	return file;
}

QString OpenFile(QWidget *parent, QString title, QString path,
		 QString extensions)
{
	QString file =
		QFileDialog::getOpenFileName(parent, title, path, extensions);

	return file;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
bool jrqtFileExists(QString filepath) {
    QFileInfo check_file(filepath);
    if (check_file.exists() && check_file.isFile()) {
        return true;
    } else {
        return false;
    }
}
//---------------------------------------------------------------------------
