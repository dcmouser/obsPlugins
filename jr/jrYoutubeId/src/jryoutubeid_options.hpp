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
#include <QLineEdit>
#include <QMainWindow>
#include <QDialog>
#include <QGridLayout>
#include <QSpinBox>

#include "../../jrcommon/src/jrobsplugin_options.hpp"

class JrYouTubeId;



class OptionsDialog : public JrPluginOptionsDialog {
	//Q_OBJECT
	JrYouTubeId* pluginp = NULL;
	QGridLayout *mainLayout;
	QLineEdit* editChatUtilityCommandline;
	QCheckBox* checkboxStartMinimized;
public:
	OptionsDialog(QMainWindow *parent, JrYouTubeId* inpluginp);
	~OptionsDialog();
public:
	virtual void onClickApply();
	virtual void buildUi();
public:
	void setOptionChatUtilityCommandline(QString chatUtilityCommandLine);
	void setOptionStartMinimized(bool val);
};
