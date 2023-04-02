#pragma once

#include <obs.hpp>
#include <util/platform.h>
#include <obs-frontend-api.h>


#include <QColorDialog>
#include <QFontDialog>
#include <QMainWindow>
#include <QMenu>
#include <QStyle>
#include <QTextList>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScreen>

void addSpacerToLayout(QLayout* layout);
void addLineSeparatorToLayout(QLayout* layout, int paddingTop, int paddingBot);

void setThemeID(QWidget* widget, const QString& themeID);
bool WindowPositionValid(QRect rect);

QString splitOffRightWord(QString& str, QString splitPattern);
QString splitOffLeftWord(QString& str, QString splitPattern);

QString sanitizeMessageString(const QString& str);

void showModalQtDialog(const QString& title, const QString& msg);
void showModalQtDialogError(const QString& msg);
