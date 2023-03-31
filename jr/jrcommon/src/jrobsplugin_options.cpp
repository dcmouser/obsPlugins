#include "jrobsplugin_options.hpp"

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







JrPluginOptionsDialog::JrPluginOptionsDialog(QMainWindow* parent)
	: QDialog(parent), mainLayout(NULL)
{
}



JrPluginOptionsDialog::~JrPluginOptionsDialog()
{
}
//---------------------------------------------------------------------------





