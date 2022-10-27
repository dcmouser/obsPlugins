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

class jrTimestamper;

class OptionsDialog : public JrPluginOptionsDialog {
	//Q_OBJECT
	jrTimestamper* timestamperp = NULL;
	QGridLayout *mainLayout;
	QCheckBox* checkbox_enable;
	QCheckBox* checkbox_recordAllTransitions;
	QTextEdit* textEdit_breakPatternString;
public:
	OptionsDialog(QMainWindow *parent, jrTimestamper* intimestamperp);
	~OptionsDialog();
public:
	virtual void onClickApply();
	virtual void buildUi();
public:
	void setOptionEnabled(bool val);
	void setOptionLogAllSceneTransitions(bool val);
	void setBreakPatternStringNewlined(std::string str);
};
