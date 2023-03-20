#include "jryoutubechat.hpp"
#include "jryoutubechat_options.hpp"
#include "../../jrcommon/src/jrobshelpers.hpp"

#include <obs-module.h>
#include <obs-frontend-api.h>

#include <QCompleter>

#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QSpinBox>
#include <QTableView>
#include <QtWidgets/QColorDialog>


OptionsDialog::OptionsDialog(QMainWindow* parent, JrYouTubeChat* inpluginp)
	: JrPluginOptionsDialog(parent)
{
	pluginp = inpluginp;
	buildUi();
}



OptionsDialog::~OptionsDialog()
{
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
void OptionsDialog::buildUi() {
	int idx = 0;
	QLabel* label;

	// main layout of controls
	mainLayout = new QGridLayout;

	label = new QLabel(obs_module_text("Chat utility script commandline"));
	mainLayout->addWidget(label, idx, 0, Qt::AlignLeft);
	editChatUtilityCommandline = new QLineEdit;
	++idx;
	mainLayout->addWidget(editChatUtilityCommandline, idx, 0, 1,2 );
	editChatUtilityCommandline->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	++idx;

	//
	label = new QLabel(obs_module_text("Start embedded"));
	mainLayout->addWidget(label, idx, 0, Qt::AlignLeft);
	checkboxStartEmbedded = new QCheckBox;
	mainLayout->addWidget(checkboxStartEmbedded, idx, 1, Qt::AlignLeft);
	++idx;

	//
	label = new QLabel(obs_module_text("Show emoticons"));
	mainLayout->addWidget(label, idx, 0, Qt::AlignLeft);
	checkboxShowEmoticons = new QCheckBox;
	mainLayout->addWidget(checkboxShowEmoticons, idx, 1, Qt::AlignLeft);
	++idx;

	//
	label = new QLabel(obs_module_text("Large styled (needs restart)"));
	mainLayout->addWidget(label, idx, 0, Qt::AlignLeft);
	checkboxSetStyle = new QCheckBox;
	mainLayout->addWidget(checkboxSetStyle, idx, 1, Qt::AlignLeft);
	++idx;

	//
	label = new QLabel(obs_module_text("Listview font size (only when styled)"));
	mainLayout->addWidget(label, idx, 0, Qt::AlignLeft);
	spinBoxFontSize = new QSpinBox;
	spinBoxFontSize->setMinimum(8);
	spinBoxFontSize->setMaximum(100);
	mainLayout->addWidget(spinBoxFontSize, idx, 1, Qt::AlignLeft);
	++idx;

	//
	label = new QLabel(obs_module_text("Default avatar url"));
	mainLayout->addWidget(label, idx, 0, Qt::AlignLeft);
	editDefaultAvatarUrl = new QLineEdit;
	mainLayout->addWidget(editDefaultAvatarUrl, idx, 1);
	editDefaultAvatarUrl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	++idx;

	//
	label = new QLabel(obs_module_text("Enable/disable DSK by name"));
	mainLayout->addWidget(label, idx, 0, Qt::AlignLeft);
	editAutoEnableDsk = new QLineEdit;
	mainLayout->addWidget(editAutoEnableDsk, idx, 1);
	editAutoEnableDsk->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	++idx;
	//
	label = new QLabel(obs_module_text("... DSK Scene"));
	mainLayout->addWidget(label, idx, 0, Qt::AlignLeft);
	editAutoEnableDskScene = new QLineEdit;
	mainLayout->addWidget(editAutoEnableDskScene, idx, 1);
	editAutoEnableDskScene->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	++idx;
	//
	label = new QLabel(obs_module_text("Scenes to ignore DSK\nEach on it's own line\nSpecify full scene names"));
	mainLayout->addWidget(label, idx, 0, Qt::AlignLeft);
	textEdit_ignoreSceneList = new QTextEdit;
	mainLayout->addWidget(textEdit_ignoreSceneList, idx, 1, Qt::AlignLeft);
	textEdit_ignoreSceneList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	textEdit_ignoreSceneList->setAcceptRichText(false);
	++idx;

	//
	label = new QLabel(obs_module_text("Auto advance+show for scenes below"));
	mainLayout->addWidget(label, idx, 0, Qt::AlignLeft);
	checkboxUseAutoSceneList = new QCheckBox;
	mainLayout->addWidget(checkboxUseAutoSceneList, idx, 1);
	++idx;
	//
	label = new QLabel(obs_module_text("Auto-advance Scene List\nEach on it's own line\nSpecify full scene names"));
	mainLayout->addWidget(label, idx, 0, Qt::AlignLeft);
	textEdit_autoSceneList = new QTextEdit;
	mainLayout->addWidget(textEdit_autoSceneList, idx, 1, Qt::AlignLeft);
	textEdit_autoSceneList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	textEdit_autoSceneList->setAcceptRichText(false);
	++idx;

	//
	label = new QLabel(obs_module_text("Auto-advance show time (ms):"));
	mainLayout->addWidget(label, idx, 0, Qt::AlignLeft);
	spinBoxTimeMsShow = new QSpinBox;
	spinBoxTimeMsShow->setMinimum(1000);
	spinBoxTimeMsShow->setMaximum(30000);
	spinBoxTimeMsShow->setSingleStep(100);
	mainLayout->addWidget(spinBoxTimeMsShow, idx, 1, Qt::AlignLeft);
	++idx;
	//
	label = new QLabel(obs_module_text("Auto-advance off time (ms):"));
	mainLayout->addWidget(label, idx, 0, Qt::AlignLeft);
	spinBoxTimeMsOff = new QSpinBox;
	spinBoxTimeMsOff->setMinimum(0);
	spinBoxTimeMsOff->setMaximum(30000);
	spinBoxTimeMsOff->setSingleStep(100);
	mainLayout->addWidget(spinBoxTimeMsOff, idx, 1, Qt::AlignLeft);
	++idx;

	//
	label = new QLabel(obs_module_text("Initial/Manual comment lines\n(author:message|[optionalavatarurl] (use // for comments)"));
	mainLayout->addWidget(label, idx, 0, Qt::AlignLeft);
	textEditManualLines = new QTextEdit;
	++idx;
	mainLayout->addWidget(textEditManualLines, idx, 0, 1,2 );
	textEditManualLines->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	//textEditManualLines->setWordWrapMode(QTextOption::NoWrap);
	textEditManualLines->setAcceptRichText(false);
	++idx;

	//
	mainLayout->setColumnStretch(1, 1);

	// entire layout
	QVBoxLayout* vlayout = new QVBoxLayout;
	vlayout->addLayout(mainLayout);

	if (true) {
		// button row
		QHBoxLayout* buttonLayout = new QHBoxLayout;
		//
		QPushButton* cancelButton = new QPushButton(obs_module_text("Cancel"));
		buttonLayout->addWidget(cancelButton, 0, Qt::AlignLeft);
		connect(cancelButton, &QPushButton::clicked, [this]() { onClickCancel(); close(); });
		//
		QPushButton* closeApplyButton = new QPushButton(obs_module_text("Apply and Close"));
		buttonLayout->addWidget(closeApplyButton, 0, Qt::AlignRight);
		connect(closeApplyButton, &QPushButton::clicked, [this]() { onClickApply(); close(); });
		//
		vlayout->addStretch();
		vlayout->addLayout(buttonLayout);
	}


	setLayout(vlayout);

	// dialog
	setWindowTitle(obs_module_text(PLUGIN_OPTIONS_LABEL));
	setMinimumSize(640, 480);
	setSizeGripEnabled(true);
}
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
void OptionsDialog::onClickApply() {
	pluginp->setOptionChatUtilityCommandline(editChatUtilityCommandline->text());
	pluginp->setOptionStartEmbedded(checkboxStartEmbedded->checkState() == Qt::Checked);
	pluginp->setOptionShowEmoticons(checkboxShowEmoticons->checkState() == Qt::Checked);
	pluginp->setOptionSetStyle(checkboxSetStyle->checkState() == Qt::Checked);
	pluginp->setOptionFontSize(spinBoxFontSize->value());
	pluginp->setOptionDefaultAvatarUrl(editDefaultAvatarUrl->text());
	pluginp->setOptionManualLines(textEditManualLines->toPlainText());
	pluginp->setOptionAutoEnableDsk(editAutoEnableDsk->text());
	pluginp->setOptionAutoEnableDskScene(editAutoEnableDskScene->text());
	pluginp->setOptionIgnoreScenesList(textEdit_ignoreSceneList->toPlainText());
	pluginp->setOptionEnableAutoAdvanceScenesList(checkboxUseAutoSceneList->checkState() == Qt::Checked);
	pluginp->setOptionAutoAdvanceScenesList(textEdit_autoSceneList->toPlainText());
	pluginp->setOptionAutoTimeShow(spinBoxTimeMsShow->value());
	pluginp->setOptionAutoTimeOff(spinBoxTimeMsOff->value());


	pluginp->optionsFinishedChanging();
}
//---------------------------------------------------------------------------
