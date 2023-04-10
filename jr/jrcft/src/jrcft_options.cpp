#include "jrcft.hpp"
#include "jrcft_options.hpp"
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


OptionsDialog::OptionsDialog(QMainWindow* parent, JrCft* inpluginp)
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

	// main layout of controls
	mainLayout = new QGridLayout;
	QLabel* label;

	//
	mainLayout->setColumnStretch(1, 1);

	// entire layout
	QVBoxLayout* vlayout = new QVBoxLayout;
	vlayout->addLayout(mainLayout);


	if (true) {
		label = new QLabel(obs_module_text("Restart media on Streaming/Recording/Broadcasting start?"));
		mainLayout->addWidget(label, idx, 0, Qt::AlignLeft);
		checkboxRestartMediaOnStart = new QCheckBox;
		mainLayout->addWidget(checkboxRestartMediaOnStart, idx, 1, Qt::AlignLeft);
		++idx;

		label = new QLabel(obs_module_text("Commandline to run on Streaming/Recording/Broadcasting start"));
		mainLayout->addWidget(label, idx, 0, Qt::AlignLeft);
		++idx;
		editStartRecStrCommandline = new QLineEdit;
		mainLayout->addWidget(editStartRecStrCommandline, idx, 0, 1, 2 );
		editStartRecStrCommandline->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		++idx;
	}


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
	//
	pluginp->setOptionStartRecStrCommandline(editStartRecStrCommandline->text());
	pluginp->setOptionRestartMediaOnStart(checkboxRestartMediaOnStart->checkState() == Qt::Checked);
	pluginp->optionsFinishedChanging();
}
//---------------------------------------------------------------------------
