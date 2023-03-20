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
#include <string>


//---------------------------------------------------------------------------
#define DefMaxLenVideoIdBuf 80
#define DefUserDataFileExtension "_jrytuserdata.json"
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
struct UserData {
public:
	time_t firstMessageTime_Session;
	time_t lastMessageTime_Session;
	int messageCount_Session;
	int characterCount_Session;
	//
	time_t lastSeenTime;
	char lastSeenVideoIdBuf[DefMaxLenVideoIdBuf];
	//
	// these might be persisted over time
	time_t firstMessageTime_AllTime;
	time_t lastMessageTime_AllTime;
	int messageCount_AllTime;
	int characterCount_AllTime;
	int sessionCount_AllTime;
	int sessionTime_AllTime;
	char lastSessionVideoIdBuf_AllTime[DefMaxLenVideoIdBuf];
	//

};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
typedef std::map<QString, QString> TStringStringMap;
typedef std::map<QString, int> TStringIntMap;
typedef std::list<QString> TStringList;
typedef std::pair<QString, int> TStringIntPair;
//
typedef std::map<QString, UserData> TStringUserMap;
typedef std::pair<QString, UserData> TStringUserPair;
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
class ChatStats {
protected:
	TStringUserMap userDataMap;
	std::string sessionVideoId;
	std::string dataFilePath;
	QString optionExcludedUsers;
public:
	ChatStats();
	virtual ~ChatStats() { ; };
public:
	bool startup(const std::string& baseFilePath);
	void shutdown();
	void setVideoId(const std::string& in_sessionVideoId);
	void resetStatsOnNewSession();
	void resetUserStatsOnNewSession(UserData& userData);
public:
	bool scanLine(const QString &author, const QString &messageSimplePlaintext);
	QString getSessionUsersAsString();
	QString getAllUsersAsString();
protected:
	void updateUserStats(const QString &author, const QString &messageSimplePlaintext);
	time_t getNowTime();
public:
	bool savePersistentUserData(const std::string &dataFilePath);
	bool loadPersistentUserData(const std::string &dataFilePath);
	void fillJsonArrayWithUserData(QJsonArray& jsonUserDataArray);
	void fillJsonArrayWithActiveUserData(QJsonArray& jsonUserDataArray);
	void addUserFromJsonObject(const QJsonObject& userObject);
	void fillJsonObjectWithUsersData(const QString& userName, const UserData& userData, QJsonObject& jsonData);
public:
	TStringUserMap* getUserMap() { return &userDataMap; };
};
//---------------------------------------------------------------------------
