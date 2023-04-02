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


class jrScreenFlip;


class OptionsDialog : public JrPluginOptionsDialog {
	//Q_OBJECT
	jrScreenFlip* pluginp = NULL;
	QGridLayout *mainLayout;
	QCheckBox* checkbox_enable;
	QCheckBox* checkbox_onlyStreamingrecording;
	QTextEdit* textEdit_sceneFilter;
public:
	OptionsDialog(QMainWindow *parent, jrScreenFlip* inpluginp);
	~OptionsDialog();
public:
	virtual void onClickApply();
	virtual void buildUi();
public:
	void setOptionEnabled(bool val);
	void setOptionOnlyStreamingrecording(bool val);
	void setOptionSceneFilterNewlined(std::string str);
};
