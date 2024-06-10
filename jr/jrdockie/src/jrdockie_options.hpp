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
#include <QLineEdit>
#include <QMainWindow>
#include <QDialog>
#include <QGridLayout>
#include <QSpinBox>

#include "../../jrcommon/src/jrobsplugin_options.hpp"

class JrDockie;



class OptionsDialog : public JrPluginOptionsDialog {
	Q_OBJECT
	JrDockie* pluginp = NULL;
	QGridLayout *mainLayout = NULL;
	QLineEdit* editTopMenuLabel = NULL;
public:
	OptionsDialog(QMainWindow *parent, JrDockie* inpluginp);
	~OptionsDialog();
public:
	virtual void onClickApply();
	virtual void buildUi();
public:
	void setOptionTopMenuLabel(QString qstr) {editTopMenuLabel->setText(qstr);}
};
