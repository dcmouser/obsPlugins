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

//#include "obs-data.h"
//#include "../deps/jansson/src/jansson.h"
//#include "../deps/jansson/src/jansson.h"
//#include "../deps/jansson/src/jansson_private.h"
//#include "../deps/jansson/src/utf.h"


//---------------------------------------------------------------------------
#define blog(level, msg, ...) blog(level, "[" PLUGIN_NAME "] " msg, ##__VA_ARGS__)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
enum JrYtWidgetListItemTypeEnum {JrYtWidgetListItemTypeEnum_Normal, JrYtWidgetListItemTypeEnum_Info, JrYtWidgetListItemTypeEnum_ManuallyAdded, JrYtWidgetListItemTypeEnum_Pending};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#define DefAutoTimerMsShowDuration	5000
#define DefAutoTimerMsClearWaitDuration	2000
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#define Def_MaxSleepMsOnRestartWaitForClose 2000
#define Def_MaxSleepMsOnExitWaitForClose 2000
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
	QVBoxLayout* mainLayout;
	QProcess process;
protected:
	obs_websocket_vendor vendor = NULL;
protected:
	// from scene notes plugin
	QListWidget* msgList;
	QLineEdit* editYouTubeId;
	QString youTubeIdQstrFromLastLoad;
	QString youTubeIdQstrUsedByChatUtil;
	QString chatUtilityCommandLine;
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
protected:
	QPushButton* toggleAutoAdvanceButton = NULL;
	QTimer autoTimer;
	int optionAutoTimeOff = 2000;
	int optionAutoTimeShow = 5000;
	bool optionAutoEngaged = false;
	int autoAdvanceStage = 0;
protected:
	bool dirtyChanges = false;
	qint64 chatExePid = 0;
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
protected:
	QListWidgetItem* selectedListItem = NULL;
public:
	void destructStuff();
public:
	JrYouTubeChat(QWidget* parent = nullptr);
	~JrYouTubeChat();
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
	void setOptionAutoTimeShow(int val) {optionAutoTimeShow = val;}
	void setOptionAutoTimeOff(int val) {optionAutoTimeOff = val;}
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
	void clearMessageList();
	void wsSetupWebsocketStuff();
	void wsShutdownWebsocketStuff();
	void triggerWsSelectedMessageChangeEvent();
	void requestWsSelectedMessageInfoEvent(obs_data_t *response_data);
	void doMessageSelectClick(QListWidgetItem* item);
	void doMessageSelect(QListWidgetItem* item);
	void clearSelectedItem() { selectedListItem = NULL; };
	void clickClearSelectedItem();
	void clearSelectedItemTriggerUpdate();
	void userDoesActionStopAutoTimer();
public:
	virtual void onModulePostLoad();
	virtual void onModuleUnload();
public:
	void addItemLabel(const QString& str, JrYtWidgetListItemTypeEnum typeEnum, int index);
	void addItemJson(const QString& str, JrYtWidgetListItemTypeEnum typeEnum, int index);
	void doMsgListAddItemStr(const QString& str, JrYtWidgetListItemTypeEnum typeEnum, int index);
	void doMsgListAddDebugStr(const QString& str);
public:
	void fillListWithManualItems();
	void fillListWithManualItem(QString str, int index);
public:
	QListWidgetItem* makeNewListWidgetItem(const QString& str, JrYtWidgetListItemTypeEnum typeEnum);
	QString buildMessageFromChatItemJson(QJsonObject& itemJson, bool resolveEmoticons, bool useEmoticonText, bool forInternalUse);
public:
	virtual void optionsFinishedChanging();
public:
	QString splitOffRightWord(QString& str, QString splitPattern);
	QString splitOffLeftWord(QString& str, QString splitPattern);
public:
	void updateDskState(bool val);
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
	void gotoLastMessage();
	void gotoNewLastIfOnNextToLast();
};
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
// UNUSED
// see https://forum.qt.io/topic/65588/wordwrap-true-problem-in-listview/9
class MyQtWordWrappedListItemDelegate : public QStyledItemDelegate {
 public:
  MyQtWordWrappedListItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {}

  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {    
    QFontMetrics fm(option.font);
    const QAbstractItemModel* model = index.model();
    QString extraString = ".....";
    QString Text = model->data(index, Qt::DisplayRole).toString() + extraString;
    QRect itemRect = QRect(option.rect);
    itemRect.setWidth(itemRect.width() - 30);
    QRect neededsize = fm.boundingRect( itemRect, Qt::TextWordWrap,Text );
    if (itemRect.height() > neededsize.height()) {
	    itemRect.setHeight(neededsize.height());
    }
    int extraWidth = 0;
    int extraHeight = 20;
    return QSize(itemRect.width()+extraWidth, itemRect.height()+extraHeight);
  }
};
//---------------------------------------------------------------------------
