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

#include "plugininfo.hpp"

#include "../../jrcommon/src/jrobsplugin.hpp"

#include "../../plugins/obs-websocket/lib/obs-websocket-api.h"

#include "chatvote.hpp"
#include "chatStats.hpp"

#include <chrono>
#include <time.h>




//---------------------------------------------------------------------------
#define blog(level, msg, ...) blog(level, "[" PLUGIN_NAME "] " msg, ##__VA_ARGS__)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
enum JrYtWidgetListItemTypeEnum {JrYtWidgetListItemTypeEnum_Normal, JrYtWidgetListItemTypeEnum_Info, JrYtWidgetListItemTypeEnum_ManuallyAdded, JrYtWidgetListItemTypeEnum_Pending, JrYtWidgetListItemTypeEnum_Poll};
enum JrYtAutoAdvanceStageEnum {JrYtAutoAdvanceStageEnum_Show, JrYtAutoAdvanceStageEnum_Hide, JrYtAutoAdvanceStageEnum_LastCheck};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#define Def_MaxSleepMsOnRestartWaitForClose 2000
#define Def_MaxSleepMsOnExitWaitForClose 2000
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// attempting to fix a bug in qt listitem sizing
#define DefUseCustomItemDelegate true
// above interferes with:
#define DefSetCustomSmallInfoFont true
// only update stats when really streaming
#define DefUpdateStatsEvenWhenNotStreaming true
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// forward declarations
class QGridLayout;
class QCloseEvent;
//
class OptionsDialog;
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
void do_frontend_save(obs_data_t* save_data, bool saving, void* data);

class JrYouTubeChat : public QDockWidget, public jrObsPlugin {
	// produces link error:
	//Q_OBJECT
protected:
	ChatVote chatVote;
	ChatStats chatStats;
protected:
	QVBoxLayout* mainLayout;
	QProcess process;
protected:
	obs_websocket_vendor vendor = NULL;
protected:
	// from scene notes plugin
	QListWidget* msgList = NULL;
	QListWidgetItem* selectedListItem = NULL;
protected:
	QLineEdit* editYouTubeId;
	QString youTubeIdQstrFromLastLoad;
	QString youTubeIdQstrUsedByChatUtil;
	QString chatUtilityCommandLine;
	//
	QPushButton* chatUtilityLaunchButton = NULL;
	QPushButton* stopUtilityButton = NULL;
	QPushButton* clearOverlayButton = NULL;
	QPushButton* toggleAutoAdvanceButton = NULL;
protected:
	bool isStreaming = false;
protected:
	int optionListItemInfoFontSize = 10;
protected:
	bool optionStartEmbedded = true;
	bool optionShowEmoticons = true;
	bool optionSetStyle = true;
	int optionFontSize = 20;
	QString optionDefaultAvatarUrl = "";
	QString optionManualLines = "";
	QString optionAutoEnableDsk = "";
	QString optionAutoEnableDskScene = "";
	QString optionIgnoreScenesList = "";
	//
	int optionAutoDelayBetweenLastItemChecks = 1000;
	int optionAutoTimeOff = 2000;
	int optionAutoTimeShow = 5000;
	int optionAutoTimeShowExtraLastItem = 10000; // note autocomputed as optionAutoTimeShow * 2
	bool optionAutoEngaged = false;
	//
	QString optionAutoAdvanceScenesList = "";
	bool optionEnableAutoAdvanceScenesList = true;
	//
	bool optionInitializeVoteWithRecentTalkers = true;
	int optionRecentTalkerTimeMins = 10;
protected:
	QTimer autoTimer;
	int autoAdvanceStage = 0;
	clock_t startTimeLastItem = 0;
protected:
	bool dirtyChanges = false;
	qint64 chatExePid = 0;
	bool ourDskLastState = false;
	bool lastSceneWasInIgnoreList = false;
	bool shouldTurnOffAutoOnAutoSceneLeave = false;
protected:
	QFont liFontInfo;
	QBrush liBrushInfo;
	QBrush liBrushManual;
	QBrush liBrushNormal;
protected:
	size_t hotkeyId_moveNext = -1;
	size_t hotkeyId_movePrev = -1;
	size_t hotkeyId_activateCurrent = -1;
	size_t hotkeyId_moveNextAndActivate = -1;
	size_t hotkeyId_clear = -1;
	size_t hotkeyId_cycleTab = -1;
	size_t hotkeyId_toggleAutoAdvance = -1;
	size_t hotkeyId_goLast = -1;
	size_t hotkeyId_voteStart = -1;
	size_t hotkeyId_voteStop = -1;
	size_t hotkeyId_voteRestart = -1;
	size_t hotkeyId_voteReopen = -1;
	size_t hotkeyId_voteGolast = -1;
public:
	void destructStuff();
public:
	JrYouTubeChat(QWidget* parent = nullptr);
	~JrYouTubeChat();
public:
	virtual void postStartup();
	virtual void initialShutdown();
public:
	void setOptionChatUtilityCommandline(QString str) {chatUtilityCommandLine = str;}
	void setOptionStartEmbedded(bool val) {optionStartEmbedded = val;};
	void setOptionShowEmoticons(bool val) {optionShowEmoticons = val;}
	void setOptionSetStyle(bool val) {optionSetStyle = val;}
	void setOptionFontSize(int val) {optionFontSize = val;}
	void setOptionDefaultAvatarUrl(QString str) {optionDefaultAvatarUrl = str;}
	void setOptionManualLines(QString str) {optionManualLines = str;}
	void setOptionAutoEnableDsk(QString str) {optionAutoEnableDsk = str;}
	void setOptionAutoEnableDskScene(QString str) {optionAutoEnableDskScene = str;}
	void setOptionIgnoreScenesList(QString str) {optionIgnoreScenesList = str;}
	void setOptionAutoTimeShow(int val) {optionAutoTimeShow = val;}
	void setOptionAutoTimeOff(int val) {optionAutoTimeOff = val;}
	void setOptionAutoAdvanceScenesList(QString str) { optionAutoAdvanceScenesList = str; };
	void setOptionEnableAutoAdvanceScenesList(bool val) { optionEnableAutoAdvanceScenesList = val; };
public:
	static void ObsFrontendEvent(enum obs_frontend_event event, void* ptr);
	static void ObsHotkeyCallback(void* data, obs_hotkey_id id, obs_hotkey_t* key, bool pressed);
protected:
	virtual void handleObsFrontendEvent(enum obs_frontend_event event);
	virtual void handleObsHotkeyPress(obs_hotkey_id id, obs_hotkey_t* key);
protected:
	virtual JrPluginOptionsDialog* createNewOptionsDialog();
	virtual void setSettingsOnOptionsDialog(JrPluginOptionsDialog* optionDialog);
protected:
	virtual void registerCallbacksAndHotkeys();
	virtual void unregisterCallbacksAndHotkeys();
public:
	void setDerivedSettingsOnOptionsDialog(OptionsDialog* optionDialog);
protected:
	virtual void loadStuff(obs_data_t* settings);
	virtual void saveStuff(obs_data_t* settings);
protected:
	void buildUi();
protected:
	void Update();
protected:
	virtual void closeEvent(QCloseEvent* event) override;
protected:
	virtual const char* getPluginName() { return PLUGIN_NAME; };
	virtual const char* getPluginLabel() { return PLUGIN_LABEL; };
	virtual const char* getPluginConfigFileName() { return PLUGIN_CONFIGFILENAME; };
	virtual const char* getPluginOptionsLabel() { return PLUGIN_OPTIONS_LABEL; };
protected:
	virtual void showEvent(QShowEvent* event) override;
	virtual void hideEvent(QHideEvent* event) override;

public slots:
	void Reset();
	void goDistribute();
	void goSetBrowserChatIds();
	void goLaunchChatUtility();
	void goOpenYtWebPage();
public:
	void grabVideoIdFromObsSelectedBroadcast();
	void sendYoutubeIdToBrowserChatSources(const QString videoid);
	void launchChatMonitorUtility(QString videoid);
	void receiveYoutubeIdSelectedSignal(QString videoid);
public:
	void prepareForDelete();
	void closeRunningProcess();
	void stopRunningProcessClick();
	void stopRunningProcess();
public:
	void processStdInputFromProcess();
	bool isProcessRunning();
	void waitForProcessFinish(int maxms);
public:
	void autoStartChatUtility();
public:
	void wsSetupWebsocketStuff();
	void wsShutdownWebsocketStuff();
	void requestWsSelectedMessageInfoEvent(obs_data_t *response_data, QListWidgetItem* itemp, int index, bool forInternalUse, bool messageChanged);
	void requestWsAllMessagesInListEvent(obs_data_t *response_data);
	void requestWsHandleMessageSelectedByClient(obs_data_t *request_data, obs_data_t* response_data);
	void requestWsHandleCommandByClient(obs_data_t *request_data, obs_data_t* response_data);
	void requestWsHandleGetState(obs_data_t *request_data, obs_data_t* response_data);
	void requestWsModifyStarState(obs_data_t* request_data, obs_data_t* response_data);
	void triggerWsSelectedMessageChangeEvent();
	void triggerWsClearMessageListEvent();
	void triggerWsNewMessageAddedToListEvent(QListWidgetItem* itemp, int index);
	void triggerWsFullListChangeEvent();
	void triggerWsStateChangeEvent_AutoToggle();
protected:
	void clearMessageList();
	void doMessageSelectClick(QListWidgetItem* item);
	void doMessageSelect(QListWidgetItem* item, bool toggleOffIfOn);
	void clearSelectedItem() { selectedListItem = NULL; };
	void clickClearSelectedItem();
	void clearSelectedItemTriggerUpdate();
	void userDoesActionStopAutoTimer();
public:
	virtual void onModulePostLoad();
	virtual void onModuleUnload();
public:
	QListWidgetItem* addYouTubeItemViaJson(const QString& str, int index, bool flagTriggerWsBroadcast, bool flagNeedsSanitizing);
	QListWidgetItem* addGenericMessageListItem(const QString &label, QJsonObject &msgObj, JrYtWidgetListItemTypeEnum typeEnum, int index, bool flagTriggerWsBroadcast);
	bool calcMessageDataFromYouTubeItemJson(QString& messageHtml, QString& messageSimplePlaintext, QString& authorName, QString &authorImageUrl, QJsonObject& itemJson, bool flagNeedsSanitizing);
public:
	bool checkScanStatementBeforeAdding(const QString& authorName, const QString& messageSimplePlaintext, bool flagManuallyAdded);
public:
	QListWidgetItem* doMsgListAddDebugStr(const QString& str, bool flagTriggerWsBroadcast);
	QListWidgetItem* doMsgListAddItemSimpleStr(const QString& str, JrYtWidgetListItemTypeEnum typeEnum, int index, bool flagTriggerWsBroadcast);
	void buildGenericMessageDataObjectForSimpleStringLabel(QString label, QJsonObject& msgObj);
public:
	QJsonObject getUserRoleJsonDataObjForItem(QListWidgetItem* itemp);
	void setUserRoleDataForItem(QListWidgetItem* itemp, const QJsonObject &itemJson);
public:
	void fillListWithManualItems();
	QListWidgetItem* fillListWithManualItem(QString str, int index, bool flagTriggerWsBroadcast);
public:
	QListWidgetItem* makeNewListWidgetItem(const QString& str, JrYtWidgetListItemTypeEnum typeEnum);
	QString buildMessageFromChatItemJson(QJsonObject& itemJson, bool resolveEmoticons, bool useEmoticonText, bool forInternalUse, bool flagNeedsSanitizing);

public:
	virtual void optionsFinishedChanging();
public:
	void updateDskStateAfterCheckingIgnoreList();
	void updateDskState(bool showDsk);
public:
	void deleteInitialManualItems();
	void refillManualItems();
	int calcTypeEnumVal(JrYtWidgetListItemTypeEnum etype) { return QListWidgetItem::UserType + etype; }
public:
	// hotkey helpers
	bool moveSelection(int offset, bool trigger);
	void cycleParentDockTab();
public:
	void toggleAutoAdvance();
	void stopAutoAdvance();
	void startAutoAdvance();
	void autoTimerTrigger();
	void setNextAutoTimer();
	void updateAutoButton();
	void gotoFirstMessage();
	void gotoLastMessage();
	void gotoMessageByIndex(int rowIndex);
	void gotoNewLastIfOnNextToLast();
	bool isOnLastRow();
	bool isOnLastNonInfoRow();
	void gotoItemByPointer(QListWidgetItem* itemp, bool flagTrigger, bool flagToggle);
public:
	void updateButtonDisableds();
public:
	int getLastListIndex();
	int getSelectedIndex();
	int getItemIndex(QListWidgetItem* targetItemp);
	bool checkSelectedItemStillGood();
//signals:
public slots:
	void toggleAutoSignal() {toggleAutoAdvance();};
public:
	QListWidgetItem* getItemByIndex(int index);
public:
	void voteStartNew();
	void voteStop();
	void voteReopen(bool flagClear);
	void voteUpdateResults(bool pushChanges);
	void voteGotoLastOrCurrent();
	void initializeVoteWithRecentTalkers();
	bool isVoteShowing();
	bool isVoteOpen();
public:
	void updateVoteItemWithTextAndLabel(QListWidgetItem* itemp, QString htmlResults, QString plainResults, int rowcount, int maxrowwidth, bool isOpen);
	void pushChangeToItem(QListWidgetItem* item);
protected:
	void testVoting();
protected:
	bool currentSceneIsInIgnoreList();
	bool currentSceneIsInAutoAdvanceList();
	void onSceneChange();
protected:
	void reIndexItems();
protected:
	time_t getNowTime();
protected:
	void turnGoLastAndEnableAutoAdvance();
	void turnOffPreviousAutoAdvanceOnSceneLeave();
	void clearAutoEnableSceneMemory() { shouldTurnOffAutoOnAutoSceneLeave = false; };
public:
	QString getStatsActive();
	QString getStatsAll();
};
//---------------------------------------------------------------------------




