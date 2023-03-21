#pragma once

#include <string>
#include <vector>

#include <obs.hpp>
//#include <../qt-wrappers.hpp>
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

//#include <../obs-frontend-api/obs-frontend-api.h>
//#include <../qt-wrappers.hpp>
//#include <../libobs/obs-module.h>



std::vector<std::string> splitString(const std::string& str, bool skipBlankLines);

std::string calcSecsAsNiceTimeString(unsigned long secs, bool flagPadLeadingZeros);
std::string getCurrentDateTimeAsNiceString();

bool doesStringMatchAnyItemsInPatternList(const std::string needle, std::vector<std::string>* haystackVectorp);

void addSpacerToLayout(QLayout* layout);
void addLineSeparatorToLayout(QLayout* layout, int paddingTop, int paddingBot);

void setThemeID(QWidget* widget, const QString& themeID);
bool WindowPositionValid(QRect rect);

QString splitOffRightWord(QString& str, QString splitPattern);
QString splitOffLeftWord(QString& str, QString splitPattern);

QString sanitizeMessageString(const QString& str);
