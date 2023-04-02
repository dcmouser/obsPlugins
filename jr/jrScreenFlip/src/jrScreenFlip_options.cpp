#include "jrScreenFlip.hpp"
#include "jrScreenFlip_options.hpp"


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



OptionsDialog::OptionsDialog(QMainWindow* parent, jrScreenFlip* inpluginp)
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
	//
	label = new QLabel(obs_module_text("Enabled"));
	mainLayout->addWidget(label, idx, 0, Qt::AlignLeft);
	checkbox_enable = new QCheckBox;
	mainLayout->addWidget(checkbox_enable, idx, 1, Qt::AlignLeft);
	++idx;
	//
	label = new QLabel(obs_module_text(Setting_OnlyDuringStreamRec_Text));
	mainLayout->addWidget(label, idx, 0, Qt::AlignLeft);
	checkbox_onlyStreamingrecording = new QCheckBox;
	mainLayout->addWidget(checkbox_onlyStreamingrecording, idx, 1, Qt::AlignLeft);
	++idx;

	//
	label = new QLabel(obs_module_text(Setting_SceneFilter_Text));
	mainLayout->addWidget(label, idx, 0, Qt::AlignLeft);
	textEdit_sceneFilter = new QTextEdit;
	mainLayout->addWidget(textEdit_sceneFilter, idx, 1, Qt::AlignLeft);
	textEdit_sceneFilter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	textEdit_sceneFilter->setAcceptRichText(false);
	++idx;
	//
	mainLayout->setColumnStretch(1, 1);




	// bottom button row
	QHBoxLayout *bottomLayout = new QHBoxLayout;
	//
	QPushButton *cancelButton = new QPushButton(obs_module_text("Cancel"));
	bottomLayout->addWidget(cancelButton, 0, Qt::AlignLeft);
	connect(cancelButton, &QPushButton::clicked, [this]() { onClickCancel(); close(); });
	//
	QPushButton *closeApplyButton = new QPushButton(obs_module_text("Apply and Close"));
	bottomLayout->addWidget(closeApplyButton, 0, Qt::AlignRight);
	connect(closeApplyButton, &QPushButton::clicked, [this]() { onClickApply(); close(); });

	// entire layout
	QVBoxLayout* vlayout = new QVBoxLayout;
	vlayout->addLayout(mainLayout);
	vlayout->addLayout(bottomLayout);
	setLayout(vlayout);

	// dialog
	setWindowTitle(obs_module_text(PLUGIN_OPTIONS_LABEL));
	setMinimumSize(160, 240);
	setSizeGripEnabled(true);
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
void OptionsDialog::setOptionEnabled(bool val) {
	checkbox_enable->setCheckState(val ? Qt::Checked : Qt::Unchecked);
}

void OptionsDialog::setOptionOnlyStreamingrecording(bool val) {
	checkbox_onlyStreamingrecording->setCheckState(val ? Qt::Checked : Qt::Unchecked);
}

void OptionsDialog::setOptionSceneFilterNewlined(std::string str) {
	textEdit_sceneFilter->setPlainText(str.c_str());
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void OptionsDialog::onClickApply() {
	pluginp->setOptionEnabled(checkbox_enable->checkState() == Qt::Checked);
	pluginp->setOptionOnlyStreamingrecording(checkbox_onlyStreamingrecording->checkState() == Qt::Checked);
	pluginp->setOptionSceneFilterNewlined(textEdit_sceneFilter->toPlainText().toStdString());
	pluginp->optionsFinishedChanging();
}
//---------------------------------------------------------------------------
