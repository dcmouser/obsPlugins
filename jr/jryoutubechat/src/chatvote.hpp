#pragma once

#include <obs.hpp>
#include <util/platform.h>
#include <obs-frontend-api.h>
#include <../obs-app.hpp>
#include <QDockWidget>
#include <QPointer>
#include <QWidget>
#include <QTimer>
#include <QLabel>
#include <QList>
#include <QLineEdit>
#include <QListWidget>
#include <QProcess>
#include <QDesktopServices>
#include <QStyledItemDelegate>

#include <map>
#include <list>


//---------------------------------------------------------------------------
typedef std::map<QString, QString> TStringStringMap;
typedef std::map<QString, int> TStringIntMap;
typedef std::list<QString> TStringList;
typedef std::pair<QString, int> TStringIntPair;
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
#define DefBlankUserVote ""
#define DefForceVoteStringsLowercase true
//---------------------------------------------------------------------------





class ChatVote {
protected:
	QRegularExpression votePatternRe;
	bool isOpenAndRunning = false;
	time_t startTime;
	int voteListItemIndex = -1;
protected:
	QListWidget* msgList = NULL;
	QListWidgetItem* voteListItemp = NULL;
protected:
	TStringStringMap userVotes;
public:
	ChatVote();
	virtual ~ChatVote() { ; };
public:
	void setMsgList(QListWidget* p) {msgList=p;}
public:
	void startNew(QListWidgetItem* itemp);
	void stop();
	bool reopen(bool flagClear);
	void clearAndRelease();
public:
	bool isOpen() { return isOpenAndRunning; }
	QListWidgetItem* getVoteListItemp() { return voteListItemp; }
	void setVoteListItemp(QListWidgetItem* itemp) { voteListItemp = itemp; }
public:
	bool scanLineForVote(const QString &author, QString messageSimplePlaintext);
	void buildResults(QString &htmlResults, QString &plainResults, int &rowcount, int &maxrowwidth, bool &isOpen);
	void addUserVote(const QString &author, const QString &choiceStr);
};
