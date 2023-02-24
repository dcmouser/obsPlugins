#include "jryoutubeid.hpp"
#include "jryoutubeid_options.hpp"
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


OptionsDialog::OptionsDialog(QMainWindow* parent, JrYouTubeId* inpluginp)
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

	label = new QLabel(obs_module_text("Chat utility"));
	mainLayout->addWidget(label, idx, 0, Qt::AlignLeft);
	editChatUtilityCommandline = new QLineEdit;
//	mainLayout->addWidget(editChatUtilityCommandline, idx, 1, Qt::AlignLeft);
	mainLayout->addWidget(editChatUtilityCommandline, idx, 1);
	editChatUtilityCommandline->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	++idx;

	//
	label = new QLabel(obs_module_text("Start minimized"));
	mainLayout->addWidget(label, idx, 0, Qt::AlignLeft);
	checkboxStartMinimized = new QCheckBox;
	mainLayout->addWidget(checkboxStartMinimized, idx, 1, Qt::AlignLeft);
	++idx;

	//
	mainLayout->setColumnStretch(1, 1);

	// entire layout
	QVBoxLayout* vlayout = new QVBoxLayout;
	vlayout->addLayout(mainLayout);

	if (true) {
		// bottom button row
		QHBoxLayout* bottomLayout = new QHBoxLayout;
		//
		QPushButton* cancelButton = new QPushButton(obs_module_text("Cancel"));
		bottomLayout->addWidget(cancelButton, 0, Qt::AlignLeft);
		connect(cancelButton, &QPushButton::clicked, [this]() { onClickCancel(); close(); });
		//
		QPushButton* closeApplyButton = new QPushButton(obs_module_text("Apply and Close"));
		bottomLayout->addWidget(closeApplyButton, 0, Qt::AlignRight);
		connect(closeApplyButton, &QPushButton::clicked, [this]() { onClickApply(); close(); });
		//
		vlayout->addStretch();
		vlayout->addLayout(bottomLayout);
	}


	setLayout(vlayout);

	// dialog
	setWindowTitle(obs_module_text(PLUGIN_OPTIONS_LABEL));
	setMinimumSize(640, 180);
	setSizeGripEnabled(true);
}
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
void OptionsDialog::onClickApply() {
	pluginp->setOptionChatUtilityCommandline(editChatUtilityCommandline->text());
	pluginp->setOptionStartMinimized(checkboxStartMinimized->checkState() == Qt::Checked);
	pluginp->optionsFinishedChanging();
}

void OptionsDialog::setOptionChatUtilityCommandline(QString chatUtilityCommandLine) {
	editChatUtilityCommandline->setText(chatUtilityCommandLine);
}

void OptionsDialog::setOptionStartMinimized(bool val) {
	checkboxStartMinimized->setCheckState(val ? Qt::Checked : Qt::Unchecked);
}
//---------------------------------------------------------------------------
