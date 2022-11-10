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

class jrNoteDock;



class OptionsDialog : public JrPluginOptionsDialog {
	//Q_OBJECT
	jrNoteDock* pluginp = NULL;
	QGridLayout *mainLayout;
public:
	OptionsDialog(QMainWindow *parent, jrNoteDock* inpluginp);
	~OptionsDialog();
public:
	virtual void onClickApply();
	virtual void buildUi();
};
