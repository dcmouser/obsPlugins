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

class JrCft;



class OptionsDialog : public JrPluginOptionsDialog {
	//Q_OBJECT
	JrCft* pluginp = NULL;
	QGridLayout *mainLayout;
	//
	QLineEdit* editStartRecStrCommandline;
	QCheckBox* checkboxRestartMediaOnStart;
public:
	OptionsDialog(QMainWindow *parent, JrCft* inpluginp);
	~OptionsDialog();
public:
	virtual void onClickApply();
	virtual void buildUi();
public:
	void setOptionStartRecStrCommandline(QString str) {editStartRecStrCommandline->setText(str);}
	void setOptionRestartMediaOnStart(bool val) { checkboxRestartMediaOnStart->setCheckState(val ? Qt::Checked : Qt::Unchecked); };
};
