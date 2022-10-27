#include "jrstats.hpp"
#include "jrstats_options.hpp"
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


OptionsDialog::OptionsDialog(QMainWindow* parent, jrStats* instatsp)
	: JrPluginOptionsDialog(parent)
{
	statsp = instatsp;
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
	label = new QLabel(obs_module_text("Scene label patterns for breaks\nEach on it's own line\nJust specify substrings"));
	mainLayout->addWidget(label, idx, 0, Qt::AlignLeft);
	textEdit_breakPatternString = new QTextEdit;
	mainLayout->addWidget(textEdit_breakPatternString, idx, 1, Qt::AlignLeft);
	++idx;
	//
	label = new QLabel(obs_module_text("Font scale normal\n(requires restart)"));
	mainLayout->addWidget(label, idx, 0, Qt::AlignLeft);
	spintEdit_fontSize_normal = new QSpinBox;
	spintEdit_fontSize_normal->setRange(1,100);
	spintEdit_fontSize_normal->setValue(statsp->getFontSizeNormal());
	mainLayout->addWidget(spintEdit_fontSize_normal, idx, 1, Qt::AlignLeft);
	++idx;
	//
	label = new QLabel(obs_module_text("Font scale headline\n(requires restart)"));
	mainLayout->addWidget(label, idx, 0, Qt::AlignLeft);
	spintEdit_fontSize_headline = new QSpinBox;
	spintEdit_fontSize_headline->setRange(1,100);
	spintEdit_fontSize_headline->setValue(statsp->getFontSizeHeadline());
	mainLayout->addWidget(spintEdit_fontSize_headline, idx, 1, Qt::AlignLeft);
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
void OptionsDialog::setBreakPatternStringNewlined(std::string str) {
	textEdit_breakPatternString->setPlainText(str.c_str());
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void OptionsDialog::onClickApply() {
	statsp->fillBreakScenePatterns(textEdit_breakPatternString->toPlainText().toStdString());
	statsp->setFontSizeNormal(spintEdit_fontSize_normal->value());
	statsp->setFontSizeHeadline(spintEdit_fontSize_headline->value());
	statsp->optionsFinishedChanging();
}
//---------------------------------------------------------------------------
