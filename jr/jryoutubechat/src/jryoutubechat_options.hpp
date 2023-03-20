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

class JrYouTubeChat;



class OptionsDialog : public JrPluginOptionsDialog {
	//Q_OBJECT
	JrYouTubeChat* pluginp = NULL;
	QGridLayout *mainLayout;
	QLineEdit* editChatUtilityCommandline;
	QCheckBox* checkboxStartEmbedded;
	QCheckBox* checkboxShowEmoticons;
	QCheckBox* checkboxSetStyle;
	QLineEdit* editDefaultAvatarUrl;
	QTextEdit* textEditManualLines;
	QSpinBox* spinBoxFontSize;
	QLineEdit* editAutoEnableDsk;
	QLineEdit* editAutoEnableDskScene;
	QTextEdit* textEdit_ignoreSceneList;
	QCheckBox* checkboxUseAutoSceneList;
	QTextEdit* textEdit_autoSceneList;
	//
	QSpinBox* spinBoxTimeMsShow;
	QSpinBox* spinBoxTimeMsOff;
public:
	OptionsDialog(QMainWindow *parent, JrYouTubeChat* inpluginp);
	~OptionsDialog();
public:
	virtual void onClickApply();
	virtual void buildUi();
public:
	void setOptionChatUtilityCommandline(QString str) {editChatUtilityCommandline->setText(str);}
	void setOptionStartEmbedded(bool val) { checkboxStartEmbedded->setCheckState(val ? Qt::Checked : Qt::Unchecked); };
	void setOptionShowEmoticons(bool val) { checkboxShowEmoticons->setCheckState(val ? Qt::Checked : Qt::Unchecked); };
	void setOptionSetStyle(bool val) { checkboxSetStyle->setCheckState(val ? Qt::Checked : Qt::Unchecked); };

	void setOptionFontSize(int val) { spinBoxFontSize->setValue(val); };
	void setOptionManualLines(QString str) { textEditManualLines->setPlainText(str); };
	void setOptionDefaultAvatarUrl(QString str) { editDefaultAvatarUrl->setText(str); };
	void setOptionAutoEnableDsk(QString str) {editAutoEnableDsk->setText(str);}
	void setOptionAutoEnableDskScene(QString str) {editAutoEnableDskScene->setText(str);}

	void setOptionAutoTimeShow(int val) { spinBoxTimeMsShow->setValue(val); };
	void setOptionAutoTimeOff(int val) { spinBoxTimeMsOff->setValue(val); };

	void setOptionIgnoreSceneList(QString str) { textEdit_ignoreSceneList->setText(str); };

	void setOptionEnableAutoAdvanceSceneList(bool val) { checkboxUseAutoSceneList->setCheckState(val ? Qt::Checked : Qt::Unchecked); };
	void setOptionAutoAdvanceSceneList(QString str) { textEdit_autoSceneList->setText(str); };
};
