#include "plugininfo.hpp"
#include "chatStats.hpp"
#include "../../jrcommon/src/jrobshelpers.hpp"
#include "../../jrcommon/src/jrobsplugin.hpp"
#include "jryoutubechat.hpp"

#include <obs-module.h>
#include <obs.h>
#include <string>
#include <../obs-frontend-api/obs-frontend-api.h>
#include <../qt-wrappers.hpp>

#include <QObject>
#include <QFile>
#include <QIODevice>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include <windows.h>
#include <shellapi.h>

#include <iostream>
#include <fstream>




//---------------------------------------------------------------------------
ChatStats::ChatStats() {
	optionExcludedUsers = "CO-OP FOR TWO";
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
bool ChatStats::scanLine(const QString &author, const QString &messageSimplePlaintext)  {
	if (optionExcludedUsers.contains(author)) {
		// don't include this user in stats
		return false;
	}
	updateUserStats(author, messageSimplePlaintext);
	return false;
}


QString ChatStats::getSessionUsersAsString() {
	// get data
	QJsonArray jsonUserDataArray;
	fillJsonArrayWithActiveUserData(jsonUserDataArray);
	// return it as string
	QJsonDocument doc(jsonUserDataArray);
	return doc.toJson();
}

QString ChatStats::getAllUsersAsString() {
	// get data
	QJsonArray jsonUserDataArray;
	fillJsonArrayWithUserData(jsonUserDataArray);
	// return it as string
	QJsonDocument doc(jsonUserDataArray);
	return doc.toJson();
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
bool ChatStats::startup(const std::string& baseFilePath) {
	dataFilePath = baseFilePath + DefUserDataFileExtension;
	loadPersistentUserData(dataFilePath);
	return true;
}

void ChatStats::shutdown() {
	savePersistentUserData(dataFilePath);
}

void ChatStats::setVideoId(const std::string& in_sessionVideoId) {
	sessionVideoId = in_sessionVideoId;
	resetStatsOnNewSession();
}

void ChatStats::resetStatsOnNewSession() {
	// possibly multiple sessions without restarting obs
	for (auto& userDataPair: userDataMap) {
		resetUserStatsOnNewSession(userDataPair.second);
	}
}

void ChatStats::resetUserStatsOnNewSession(UserData& userData) {
	userData.messageCount_Session = 0;
	userData.firstMessageTime_Session = 0;
	userData.lastMessageTime_Session = 0;
	userData.messageCount_Session = 0;
	userData.characterCount_Session = 0;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void ChatStats::updateUserStats(const QString& author, const QString& messageSimplePlaintext) {
	// init
	time_t nowTime = getNowTime();
	int msgLen = messageSimplePlaintext.length();

	// 1. get stats object for this user or create new one
	UserData& userData = userDataMap[author];

	// 2. update it
	// first session of all time init
	if (userData.sessionCount_AllTime == 0) {
		// happens only with first session of all time we've seen this user (assuming persistence)
		++userData.sessionCount_AllTime;
	}
	// first message of session inits
	if (userData.messageCount_Session==0) {
		// happens once per user per session
		// keep track of last session time they joined so we can refer to it any time this session
		userData.lastSeenTime = userData.lastMessageTime_AllTime;
		strcpy(userData.lastSeenVideoIdBuf, userData.lastSessionVideoIdBuf_AllTime);
		// new values store
		strcpy(userData.lastSessionVideoIdBuf_AllTime, sessionVideoId.c_str());
		userData.firstMessageTime_Session = nowTime;
	}

	// update stats
	++userData.messageCount_Session;
	++userData.messageCount_AllTime;
	userData.characterCount_Session += msgLen;
	userData.characterCount_AllTime += msgLen;
	userData.lastMessageTime_Session = nowTime;
	userData.lastMessageTime_AllTime = nowTime;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
time_t ChatStats::getNowTime() {
	time_t t;
	time (&t);
	return t;
}
//---------------------------------------------------------------------------













//---------------------------------------------------------------------------
bool ChatStats::savePersistentUserData(const std::string& dataFilePath) {
	mydebug("Saving chat stats to %s.", dataFilePath.c_str());

	// open file
	std::string dataFilePathTemp = dataFilePath + ".tmp";
	QFile file( dataFilePathTemp.c_str() );
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
		return false;
	}

	// build json data structure array
	QJsonArray jsonUserDataArray;
	fillJsonArrayWithUserData(jsonUserDataArray);

	// assemble into larger object
	QJsonObject jsonData;
	jsonData["UserDataArray"] = jsonUserDataArray;
	QJsonDocument doc(jsonData);

	// write data
	QByteArray bytes = doc.toJson( QJsonDocument::Indented );
	QTextStream iStream( &file );
        iStream << bytes;
        file.close();

	if (file.error() == QFileDevice::NoError) {
		// success
		std::string dataFilePathBak = dataFilePath + ".bak";
		// delete bak
		if (QFile::exists(dataFilePathBak.c_str())) {
			QFile::remove(dataFilePathBak.c_str());
		}
		// rename old to bak
		QFile::rename(dataFilePath.c_str(), dataFilePathBak.c_str());
		// copy tmp to new
		file.copy(dataFilePath.c_str());
	}

	// error
	return false;
}


bool ChatStats::loadPersistentUserData(const std::string& dataFilePath) {
	mydebug("Loading chat stats from %s.", dataFilePath.c_str());

	// open file
	QFile file( dataFilePath.c_str() );
	if (!file.open(QIODevice::ReadOnly)) {
		// not found
		return false;
	}

	// read file
        QByteArray bytes = file.readAll();
        file.close();

	// parse it
        QJsonParseError jsonError;
        QJsonDocument doc = QJsonDocument::fromJson( bytes, &jsonError );
        if( jsonError.error != QJsonParseError::NoError )
        {
            mydebug("fromJson failed: %s.", jsonError.errorString().toStdString().c_str());
            return false;
        }
	QJsonObject jsonData = doc.object();
	QJsonArray jsonUserDataArray = jsonData["UserDataArray"].toArray();

	// clear existing data of all entries
	userDataMap.clear();

	// now parse it into our map
	foreach (const QJsonValue &value, jsonUserDataArray) {
		QJsonObject obj = value.toObject();
		addUserFromJsonObject(obj);
	}

	// clear session stats
	resetStatsOnNewSession();
	return false;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void ChatStats::fillJsonArrayWithUserData(QJsonArray& jsonUserDataArray) {
	for (auto& userDataPair: userDataMap) {
		QJsonObject jsonUserObj;
		fillJsonObjectWithUsersData(userDataPair.first, userDataPair.second, jsonUserObj);
		jsonUserDataArray.push_back(jsonUserObj);
	}
}

void ChatStats::fillJsonArrayWithActiveUserData(QJsonArray& jsonUserDataArray) {
	for (auto& userDataPair: userDataMap) {
		if (userDataPair.second.messageCount_Session > 0) {
			// they talked this session so include them
			QJsonObject jsonUserObj;
			fillJsonObjectWithUsersData(userDataPair.first, userDataPair.second, jsonUserObj);
			jsonUserDataArray.push_back(jsonUserObj);
		}
	}
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void ChatStats::fillJsonObjectWithUsersData(const QString &userName, const UserData& userData, QJsonObject &jsonUserObj) {
	jsonUserObj["name"] = userName;
	jsonUserObj["firstMessageTime_AllTime"] = userData.firstMessageTime_AllTime;
	jsonUserObj["lastMessageTime_AllTime"] = userData.lastMessageTime_AllTime;
	jsonUserObj["messageCount_AllTime"] = userData.messageCount_AllTime;
	jsonUserObj["characterCount_AllTime"] = userData.characterCount_AllTime;
	jsonUserObj["sessionCount_AllTime"] = userData.sessionCount_AllTime;
	jsonUserObj["lastSessionVideoId_AllTime"] = QString(userData.lastSessionVideoIdBuf_AllTime);
	int sessionTime_AllTimeThisIncluded = userData.sessionTime_AllTime + (userData.lastMessageTime_Session - userData.firstMessageTime_Session) / 60;
	jsonUserObj["sessionTime_AllTime"] = sessionTime_AllTimeThisIncluded;
	// save session data too
	jsonUserObj["firstMessageTime_Session"] = userData.firstMessageTime_Session;
	jsonUserObj["lastMessageTime_Session"] = userData.lastMessageTime_Session;
	jsonUserObj["messageCount_Session"] = userData.messageCount_Session;
	jsonUserObj["characterCount_Session"] = userData.characterCount_Session;
}


void ChatStats::addUserFromJsonObject(const QJsonObject& userObject) {
	// get username as key
	QString userName = userObject["name"].toString();
	UserData userData;
	// fill persistent userData
	userData.firstMessageTime_AllTime = userObject["firstMessageTime_AllTime"].toInt();
	userData.lastMessageTime_AllTime = userObject["lastMessageTime_AllTime"].toInt();
	userData.messageCount_AllTime = userObject["messageCount_AllTime"].toInt();
	userData.characterCount_AllTime = userObject["characterCount_AllTime"].toInt();
	userData.sessionCount_AllTime = userObject["sessionCount_AllTime"].toInt();
	userData.sessionTime_AllTime = userObject["sessionTime_AllTime"].toInt();
	strcpy(userData.lastSessionVideoIdBuf_AllTime, userObject["lastSessionVideoId_AllTime"].toString().toStdString().c_str());
	// note we ignore session data
	// add it
	TStringUserPair userDataPair(userName, userData);
	userDataMap.insert(userDataPair);
}
//---------------------------------------------------------------------------
