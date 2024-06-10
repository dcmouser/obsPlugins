#include "jryoutubechat.hpp"
#include "jryoutubechat_options.hpp"
//
#include "../../jrcommon/src/jrhelpers.hpp"
#include "../../jrcommon/src/jrqthelpers.hpp"
#include "../../jrcommon/src/jrobshelpers.hpp"



#include <obs-module.h>
#include <obs.h>
#include <string>

#include <../obs-frontend-api/obs-frontend-api.h>
#include <../qt-wrappers.hpp>
#include <QObject>


#include <csignal>
#include <signal.h>


#include <windows.h>
#include <shellapi.h>


#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>




















//---------------------------------------------------------------------------
// this can get called in 2 kinds of places:
// first, when processing a chat entry directly from youtube stream, which is in full json format, and needs sanitization
// second on various manual creation methods
//
QListWidgetItem* JrYouTubeChat::addYouTubeItemViaJson(const QString& str, int index, bool flagTriggerWsBroadcast, bool flagNeedsSanitizing) {
	// parse json

	QJsonParseError jerror;
	QJsonDocument itemJsonDoc;
	JrYtWidgetListItemTypeEnum typeEnum = JrYtWidgetListItemTypeEnum_Normal;
	//mydebug("ATTN: JR - IN addYouTubeItemViaJson: %s.", str.toStdString().c_str());
	//
	if (str.length() > 0 && str[0] != '{') {
		//mydebug("ATTN: JR - IN addYouTubeItemViaJson stage 2");
		// not json just a simple message; to keep uniformity of processing we will just convert it to json doc with just a message string
		QJsonObject chatItemObj;
		chatItemObj["message"] = str;
		itemJsonDoc = QJsonDocument(chatItemObj);
		if (str[0]==']') {
			// this makes it an info message
			typeEnum = JrYtWidgetListItemTypeEnum_Info;
		}
	}
	else {
		// parse json
		//mydebug("ATTN: JR - IN addYouTubeItemViaJson stage 3");
		itemJsonDoc = QJsonDocument::fromJson(str.toUtf8(), &jerror);
	}

	//mydebug("ATTN: JR - IN addYouTubeItemViaJson stage 5");

	// ok we have our json doc -- is it valid?
	if (itemJsonDoc.isNull()) {
		// error parsing, so store string raw
		//mydebug("ATTN: JR - IN addYouTubeItemViaJson stage 6");
		QJsonObject chatItemObj;
		chatItemObj["message"] = ("JSONERR: " + jerror.errorString() + "\n" + str);
		itemJsonDoc = QJsonDocument(chatItemObj);
		// drop down
	}

	// ok lets process the json object decide what to save, what we can precompute, sanitize, etc.
	QJsonObject itemJson = itemJsonDoc.object();

	//mydebug("ATTN: JR - IN addYouTubeItemViaJson stage 7");

	// ok now we will extract message in different forms and precompute some variations and properties
	QString messageHtml;
	QString messageSimplePlaintext;
	QString authorName;
	QString authorImageUrl;
	bool bretv = calcMessageDataFromYouTubeItemJson(messageHtml, messageSimplePlaintext, authorName, authorImageUrl, itemJson, flagNeedsSanitizing);

	//mydebug("ATTN: JR - IN addYouTubeItemViaJson stage 8: %d", (int)bretv);

	// before we add it lets see if some other spin-off handles it
	if (checkScanStatementBeforeAdding(authorName, messageSimplePlaintext, false)) {
		//mydebug("ATTN: JR - IN addYouTubeItemViaJson SKIPPING because checkScanStatementBeforeAdding returns true.");
		return NULL;
	}

	//mydebug("ATTN: JR - IN addYouTubeItemViaJson stage 9.");

	// build simple label for listview
	QString authorNameUpper = authorName.toUpper();
	QString label;
	if (optionSetStyle) {
		if (authorName != "") {
			label = authorNameUpper + ":\n" + messageSimplePlaintext;
		}
		else {
			label = messageSimplePlaintext;
		}
	}
	else {
		QString timeStr = itemJson.value("elapsedTime").toString();
		if (authorName != "") {
			label = timeStr + " - " + authorNameUpper + ": " + messageSimplePlaintext;
		}
		else {
			label = timeStr + " - " + messageSimplePlaintext;
		}
	}

	// add extended computed info as json raw user data
	QJsonObject msgObj;
	msgObj["messageHtml"] = messageHtml;
	msgObj["messageSimplePlaintext"] = messageSimplePlaintext;
	msgObj["plainTextLen"] = messageSimplePlaintext.length();
	msgObj["authorName"] = authorName;
	msgObj["authorImageUrl"] = authorImageUrl;

	//mydebug("ATTN: JR - IN addYouTubeItemViaJson stage 10: %s (author:%s) [%s].", messageSimplePlaintext.toStdString().c_str(), authorName.toStdString().c_str(), authorNameUpper.toStdString().c_str());

	QListWidgetItem* itemp = addGenericMessageListItem(label, msgObj, typeEnum, index, flagTriggerWsBroadcast);
	return itemp;
}


void JrYouTubeChat::buildGenericMessageDataObjectForSimpleStringLabel(QString label, QJsonObject& msgObj) {
	msgObj["messageHtml"] = label;
	msgObj["messageSimplePlaintext"] = label;
	msgObj["plainTextLen"] = label.length();
	msgObj["authorName"] = "";
	msgObj["authorImageUrl"] = "";
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
bool JrYouTubeChat::checkScanStatementBeforeAdding(const QString &authorName, const QString &messageSimplePlaintext, bool flagManuallyAdded) {
	// return true if this was a vote and is consumed as a vote and shouldn't be added to list
	// generic stats
	if (authorName != "") {
		// avoid manual on stats?
		if (!flagManuallyAdded) {
			if (DefUpdateStatsEvenWhenNotStreaming || isStreaming) {
				bool bretv = chatStats.scanLine(authorName, messageSimplePlaintext);
				if (bretv) {
					// stop here
					return true;
				}
			}
		}
	}
	// vote?
	if (chatVote.isOpen()) {
		bool isVote = chatVote.scanLineForVote(authorName, messageSimplePlaintext);
		if (isVote) {
			// we added it as vote, so don't add it as item; but update vote stats
			voteUpdateResults(true);
			return true;
		}
	}
	return false;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
bool JrYouTubeChat::calcMessageDataFromYouTubeItemJson(QString& messageHtml, QString& messageSimplePlaintext, QString& authorName, QString &authorImageUrl, QJsonObject& itemJson, bool flagNeedsSanitizing) {
	// we need to walk through messageex and build (or strip)

	// clear all first
	messageHtml = "";
	messageSimplePlaintext = "";
	authorName = "";
	authorImageUrl = "";

	bool optionUnicodeEmoticonForPlainText = true;
	bool optionResolveEmoticonsForHtml = true;
	//
	//messageSimplePlaintext = str = itemJson.value("message").toString();;
	//
	auto messageExObj = itemJson.value("messageEx");
	auto messageExArray = messageExObj.toArray();
	if (messageExArray.isEmpty()) {
		messageSimplePlaintext = messageHtml = itemJson.value("message").toString();
		return false;
	}

	// loop array of messageex and built it
	QString str;
	foreach(const QJsonValue &val, messageExArray) {
		if (val.isString()) {
			// text string, add it
			str = val.toString();
			// plaintext does not sanitize since it is not rendered as html
			messageSimplePlaintext += str;
			// html sanitizes
			if (flagNeedsSanitizing) {
				str = sanitizeMessageString(str);
			}
			messageHtml += str;
		}
		else {
			// its a dict with url item
			if (optionResolveEmoticonsForHtml) {
				QString emoticonUrl = val["url"].toString();
				messageHtml += " <img src=\"" + emoticonUrl + "\" class=\"jrYtEmoticon\">";
				if (optionUnicodeEmoticonForPlainText) {
					QString emoticonText = val["id"].toString();
					messageSimplePlaintext += emoticonText;
				}
			} else {
				QString emoticonText = val["id"].toString();
				messageHtml += emoticonText;
				if (optionUnicodeEmoticonForPlainText) {
					messageSimplePlaintext += emoticonText;
				}
			}
		}
	}

	// now author info
	QJsonObject authorJson = itemJson.value("author").toObject();
	if (!authorJson.isEmpty()) {
		authorName = authorJson.value("name").toString();
		authorImageUrl = authorJson.value("imageUrl").toString();
	}

	// success
	return true;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
QListWidgetItem* JrYouTubeChat::addGenericMessageListItem(const QString& label, QJsonObject &msgObj, JrYtWidgetListItemTypeEnum typeEnum, int index, bool flagTriggerWsBroadcast) {
	// called by youtube item create and any simpler ones
	//
	// add index to msgObj before we add it in
	int itemIndex = (index != -1) ? index : getLastListIndex()+1;
	msgObj["index"] = itemIndex;

	// ATTN: 5/7/24 we were getting failure to add items, and i think it is because of newlines in label
	QString labelDisplay = label;
	if (false) {
		labelDisplay.replace("\n", "<br/>");
	}
	else if (false) {
		labelDisplay.replace("\n", " ");
	} else {
		// nothing
	}

	// 
	//mydebug("ATTN: JR - IN addGenericMessageListItem label = '%s' (%s).", label.toStdString().c_str(), labelDisplay.toStdString().c_str());

	// create new list item
	QListWidgetItem* itemp = makeNewListWidgetItem(labelDisplay, typeEnum);

	// add the data msgObj as a json object wrapped in variant to the list item
	setUserRoleDataForItem(itemp, msgObj);

	// now add item to listview
	if (index == -1) {
		msgList->addItem(itemp);
		index = getLastListIndex();
	}
	else {
		msgList->insertItem(index, itemp);
	}

	// go to it if on last item
	if (index == getLastListIndex()) {
		gotoNewLastIfOnNextToLast();
	}

	// caller should do this now
	if (flagTriggerWsBroadcast) {
		// trigger websocket event
		triggerWsNewMessageAddedToListEvent(itemp, index);
	}

	return itemp;
}
//---------------------------------------------------------------------------















//---------------------------------------------------------------------------
QListWidgetItem* JrYouTubeChat::doMsgListAddDebugStr(const QString& str, bool flagTriggerWsBroadcast) {
	return doMsgListAddItemSimpleStr(str, JrYtWidgetListItemTypeEnum_Info, -1, flagTriggerWsBroadcast);
}


QListWidgetItem* JrYouTubeChat::doMsgListAddItemSimpleStr(const QString& label, JrYtWidgetListItemTypeEnum typeEnum, int index, bool flagTriggerWsBroadcast) {
	QJsonObject msgObj;
	buildGenericMessageDataObjectForSimpleStringLabel(label, msgObj);
	return addGenericMessageListItem(label, msgObj, typeEnum, index, flagTriggerWsBroadcast);
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
QListWidgetItem* JrYouTubeChat::fillListWithManualItem(QString str, int index, bool flagTriggerWsBroadcast) {
	// str is in syntax [*][AUTHORNAME:] MSG [| OPTIONAL IMAGE URL]
	QJsonObject msgObj;

	// first char * means starred
	bool starred = false;
	if (str.length() > 0 && str[0] == '*') {
		starred = true;
		str = str.sliced(1);
	}

	// ok first lets get author name
	QString authorImageUrl = splitOffRightWord(str, "|");
	if (authorImageUrl == "") {
		// default author image
		authorImageUrl = optionDefaultAvatarUrl;
	}
	// author name if any
	QString authorName = splitOffLeftWord(str, ":");

	// see if its a vote, if so don't add it
	if (checkScanStatementBeforeAdding(authorName, str, true)) {
		return NULL;
	}

	// label
	QString label;
	QString authorNameUpper = authorName.toUpper();
	if (authorName != "") {
		label = authorNameUpper + ":\n" + str;
	} else {
		label = str;
	}

	// strip comments for message
	if (str.contains("//")) {
		size_t pos = str.indexOf("//");
		QString notes = str.mid(pos+2);
		str = str.mid(0, pos);
		//
		msgObj["notes"] = notes;
	}

	// newline \n replace (this will get further replaced during display)
	str.replace("\\n", "\n");

	// main fields
	msgObj["messageHtml"] = str;
	msgObj["messageSimplePlaintext"] = str;
	msgObj["plainTextLen"] = label.length();
	msgObj["authorName"] = authorName;
	msgObj["authorImageUrl"] = authorImageUrl;

	// extras
	if (starred) {
		msgObj["starred"] = starred;
	}

	return addGenericMessageListItem(label, msgObj, JrYtWidgetListItemTypeEnum_ManuallyAdded, index, flagTriggerWsBroadcast);
}
//---------------------------------------------------------------------------










//---------------------------------------------------------------------------
void JrYouTubeChat::voteStartNew() {
	// create a new poll

	// object data
	QJsonObject msgObj;
	msgObj["authorImageUrl"] = "avatars/vote.png";
	msgObj["showPlainInList"] = true;

	QListWidgetItem* itemp = addGenericMessageListItem("NEW VOTE POLL", msgObj, JrYtWidgetListItemTypeEnum_Poll, -1, false);
	// start new vote and remember listitem
	chatVote.startNew(itemp);
	if (optionInitializeVoteWithRecentTalkers) {
		initializeVoteWithRecentTalkers();
	}

	// this will push changed into this item via updateVoteItemWithTextAndLabel
	voteUpdateResults(false);
	// we need to push it via trigger
	gotoItemByPointer(itemp, true, false);
	triggerWsNewMessageAddedToListEvent(itemp, getLastListIndex());
}


void JrYouTubeChat::initializeVoteWithRecentTalkers() {
	// add all recent talkers to poll with blank results
	long cutoffSeconds = optionRecentTalkerTimeMins * 60;
	time_t nowTime = getNowTime();
	TStringUserMap *userDataMapp = chatStats.getUserMap();
	for (auto& userDataPair: *userDataMapp) {
		if (userDataPair.second.messageCount_Session > 0) {
			// they have talked in this session, now how long ago was it
			time_t elapsed = nowTime - userDataPair.second.lastMessageTime_Session;
			if (elapsed < cutoffSeconds) {
				// we got a recent one
				chatVote.addUserVote(userDataPair.first, DefBlankUserVote);
			}
		}
	}
}




void JrYouTubeChat::updateVoteItemWithTextAndLabel(QListWidgetItem* itemp, QString htmlResults, QString plainResults, int rowcount, int maxrowwidth, bool isOpen) {
	// note we are MODIFYING an item
	//mydebug("stage2: %s",obs_data_get_json(request_data));

	// get existing json userdata; initialize it if not found
	QJsonObject msgObj = getUserRoleJsonDataObjForItem(itemp);

	// modifications
	// set virtual author label based on state
	QString authorName;
	if (isOpen) {
		 authorName = QString("VOTING OPEN");
	}
	else {
		 authorName = QString("VOTE COMPLETE");
	}
	msgObj["authorName"] = authorName;
	// other fields
	msgObj["messageSimplePlaintext"] = plainResults;
	// for vote, the estimate of plaintextLen is approximated to give some idea of size
	msgObj["plainTextLen"] = rowcount * 100;
	msgObj["messageHtml"] = htmlResults;
	msgObj["rowcount"] = rowcount;
	msgObj["maxrowwidth"] = maxrowwidth;

	// start time
	if (!msgObj.contains("startTime")) {
		msgObj["startTime"] = getNowTime();
	}
	// end time
	if (!msgObj.contains("endTime") && !isOpen) {
		msgObj["endTime"] = getNowTime();
	}

	// resave user data
	setUserRoleDataForItem(itemp, msgObj);

	// set new label simple display
	QString label = authorName + ":\n" + plainResults;
	itemp->setText(label);
}
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
void JrYouTubeChat::requestWsModifyStarState(obs_data_t* request_data, obs_data_t* response_data) {
	// get info
	int index = obs_data_get_int(request_data, "itemid");
	bool starState = obs_data_get_bool(request_data, "state");
	QListWidgetItem* itemp = getItemByIndex(index);
	if (!itemp) {
		return;
	}

	// get existing json userdata; initialize it if not found
	QJsonObject itemJson = getUserRoleJsonDataObjForItem(itemp);

	// modifications
	itemJson["starred"] = starState;

	// resave it
	setUserRoleDataForItem(itemp, itemJson);
}
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
QJsonObject JrYouTubeChat::getUserRoleJsonDataObjForItem(QListWidgetItem* itemp) {
	QVariant jsonItemVar = itemp->data(Qt::UserRole);
	QString str;
	if (!jsonItemVar.isValid()) {
		str = "{}";
	}
	else {
		str = jsonItemVar.toString();
	}
	// decode it again like in original parsing
	QJsonDocument itemJsonDoc = QJsonDocument::fromJson(str.toUtf8());
	QJsonObject itemJson = itemJsonDoc.object();
	return itemJson;
}


void JrYouTubeChat::setUserRoleDataForItem(QListWidgetItem* itemp, const QJsonObject& itemJson) {

	if (true) {
		QJsonDocument itemJsonDoc = QJsonDocument(itemJson);
		QVariant jsonQVariant(itemJsonDoc.toJson());
		itemp->setData(Qt::UserRole, jsonQVariant);
	}
	else {
		QVariant jsonQVariant(itemJson);
		itemp->setData(Qt::UserRole, jsonQVariant);
	}

}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
void JrYouTubeChat::requestWsAllMessagesInListEvent(obs_data_t *response_data) {
	// ATTN: todo there iss a lot of code repetition in here from the single result reply, we need to merge
	// a websocket client can call this function to be told the entire list of messages currently in list (could be a big reply)
	// returns obs_data_t*
	//mydebug("In requestWsAllMessagesInListEvent.");

	// first build a big json document
	QJsonDocument jsonDoc;
	QJsonArray jsonArray;

	// loop all items
	QListWidgetItem* itemp;

	for (int i = 0; i < msgList->count(); ++i) {
		itemp = msgList->item(i);
		// get existing json userdata
		QJsonObject itemJson = getUserRoleJsonDataObjForItem(itemp);
		// add any dynamic props
		if (itemp == selectedListItem) {
			// we are updating the item that is being highlighted
			itemJson["highlighted"] = true;
		}
		// add it to return array
		jsonArray.append(itemJson);
	}

	// now convert json to obs_data_t *
	jsonDoc.setArray(jsonArray);
	obs_data_set_string(response_data, "dataList", jsonDoc.toJson());
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void JrYouTubeChat::requestWsSelectedMessageInfoEvent(obs_data_t *response_data, QListWidgetItem* itemp, int index, bool forInternalUse, bool messageChanged) {
	// a websocket client can call this function to be told what the currrently selected message index is (useful on startup of client so it doesnt have to wait for next broadcast)
	//mydebug("In requestWsSelectedMessageInfoEvent.");

	if (!msgList || !itemp) {
		// clear selection
		QJsonObject itemJson;
		itemJson["index"] = -1;
		QJsonDocument jsonDoc(itemJson);
		obs_data_set_string(response_data, "data", jsonDoc.toJson());
		return;
	}

	// ATTN: we need to make sure itemp stays valid
	QString msgHtml,authorName,authorImageUrl;
	bool starred = false;
	int rowcount = 0;
	// fill it from current selection
	int plainTextLen = 0;

	// get existing json userdata
	QJsonObject itemJson = getUserRoleJsonDataObjForItem(itemp);
	// add info
	if (itemp == selectedListItem) {
		// we are updating the item that is being highlighted
		itemJson["highlighted"] = true;
	}
	if (messageChanged) {
		itemJson["messageChanged"] = true;
	}
	// build reply
	QJsonDocument jsonDoc(itemJson);
	obs_data_set_string(response_data, "data", jsonDoc.toJson());
	//mydebug("requestWsSelectedMessageInfoEvent sending: %s.", jsonDoc.toJson().toStdString().c_str());
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
QListWidgetItem* JrYouTubeChat::makeNewListWidgetItem(const QString &str, JrYtWidgetListItemTypeEnum typeEnum) {
	QListWidgetItem* qp = new QListWidgetItem(str, NULL, calcTypeEnumVal(typeEnum));
	// special override details?
	if (true) {
		switch (typeEnum) {
		case JrYtWidgetListItemTypeEnum_Info:
			if (DefSetCustomSmallInfoFont) {
				qp->setFont(liFontInfo);
			}
			//qp->setFlags(qp->flags() & ~Qt::ItemIsSelectable);
			//qp->setBackground(liBrushInfo);
			//qp->setCheckState(Qt::Checked);
			break;
		case JrYtWidgetListItemTypeEnum_ManuallyAdded:
			//qp->setFlags(qp->flags() &~ Qt::ItemIsSelectable);
			//qp->setBackground(liBrushManual);
			//qp->setCheckState(Qt::Checked);
			break;
		case JrYtWidgetListItemTypeEnum_Normal:
			//qp->setBackground(liBrushNormal);
			break;
		case JrYtWidgetListItemTypeEnum_Poll:
			//qp->setBackground(liBrushNormal);
			break;
		default:
			break;
		}
	}
	return qp;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
void JrYouTubeChat::reIndexItems() {
	// needed after we insert new items at top
	// loop all items
	QListWidgetItem* itemp;

	for (int i = 0; i < msgList->count(); ++i) {
		itemp = msgList->item(i);
		// get existing json userdata
		QJsonObject itemJson = getUserRoleJsonDataObjForItem(itemp);
		itemJson["index"] = i;
		// resave it
		setUserRoleDataForItem(itemp, itemJson);
	}
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
time_t JrYouTubeChat::getNowTime() {
	// get current time
	time_t t;
	time (&t);
	return t;
}
//---------------------------------------------------------------------------


