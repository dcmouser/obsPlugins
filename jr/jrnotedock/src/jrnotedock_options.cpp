#include "jrnotedock.hpp"
#include "jrnotedock_options.hpp"
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


OptionsDialog::OptionsDialog(QMainWindow* parent, jrNoteDock* inpluginp)
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
	mainLayout = new QGridLayout();
	//
	//mainLayout->setColumnStretch(1, 1);
	//
	
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
	setMinimumSize(160, 240);
	setSizeGripEnabled(true);
}
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
void OptionsDialog::onClickApply() {
	pluginp->optionsFinishedChanging();
}
//---------------------------------------------------------------------------
