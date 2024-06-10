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


class JrPluginOptionsDialog : public QDialog {
	Q_OBJECT
	QGridLayout *mainLayout;
public:
	JrPluginOptionsDialog(QMainWindow *parent);
	~JrPluginOptionsDialog();
public:
	virtual void buildUi() { ; };
	virtual void onClickCancel() { ; };
	virtual void onClickApply() { ; };
};
