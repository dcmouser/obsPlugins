#pragma once

#include <obs.hpp>
#include <util/platform.h>
#include <obs-frontend-api.h>

#include <QDockWidget>
#include <QPointer>
#include <QWidget>
#include <QTimer>
#include <QLabel>
#include <QList>

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QTextEdit>
#include <QMainWindow>
#include <QDialog>
#include <QGridLayout>
#include <QSpinBox>

#include "../../jrcommon/src/jrobsplugin_options.hpp"

class jrStats;



class OptionsDialog : public JrPluginOptionsDialog {
	//Q_OBJECT
	jrStats* statsp = NULL;
	//
	QGridLayout *mainLayout;
	//
	QTextEdit* textEdit_breakPatternString;
	QSpinBox* spintEdit_fontSize_headline;
	QSpinBox* spintEdit_fontSize_normal;
public:
	OptionsDialog(QMainWindow *parent, jrStats* instatsp);
	~OptionsDialog();
public:
	virtual void onClickApply();
	virtual void buildUi();
public:
	void setBreakPatternStringNewlined(std::string str);
};
