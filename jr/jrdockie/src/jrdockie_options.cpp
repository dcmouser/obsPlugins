#include "jrdockie.hpp"
#include "jrdockie_options.hpp"
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


OptionsDialog::OptionsDialog(QMainWindow* parent, JrDockie* inpluginp)
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
	mainLayout = new QGridLayout();


	// entire layout
	QVBoxLayout* vlayout = new QVBoxLayout;
	vlayout->addLayout(mainLayout);

	// options
	label = new QLabel(obs_module_text("Name of toolbar menu to place the JrDockie submenu inside of? (default: Docks)\nNote: Leave blank to add a new top-level menu."));
	mainLayout->addWidget(label, idx, 0, Qt::AlignLeft);
	++idx;
	editTopMenuLabel = new QLineEdit;
	mainLayout->addWidget(editTopMenuLabel, idx, 0, 1, 2 );
	++idx;



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
	QString windowTitle = QString(PLUGIN_OPTIONS_LABEL) + " v" + QString(PLUGIN_VERSION);
	setWindowTitle(windowTitle);
	setMinimumSize(480, 140);
	setSizeGripEnabled(true);
}
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
void OptionsDialog::onClickApply() {
	// hide menu before we change the type, to support changing where menu is
	pluginp->hideDockMenuWidgetp();
	//
	pluginp->setOptionTopLevelDockLabel(editTopMenuLabel->text());
	pluginp->optionsFinishedChanging();
}
//---------------------------------------------------------------------------
