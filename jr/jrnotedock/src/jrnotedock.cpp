#include "jrnotedock.hpp"
#include "jrnotedock_options.hpp"
#include "../../jrcommon/src/jrobshelpers.hpp"

#include <obs-module.h>

#include <string>


#include <../obs-frontend-api/obs-frontend-api.h>
#include <../qt-wrappers.hpp>





//---------------------------------------------------------------------------
OBS_DECLARE_MODULE()
OBS_MODULE_AUTHOR(PLUGIN_AUTHOR);
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
#define TIMER_INTERVAL 10000
#define DefFontSize 16
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
static jrNoteDock* moduleInstance = NULL;
static bool moduleInstanceIsRegisteredAndAutoDeletedByObs = false;

bool obs_module_load() {
	blog(LOG_INFO, "plugin loaded successfully (version %s)", PLUGIN_VERSION);

	obs_frontend_push_ui_translation(obs_module_get_string);
	//
	const auto main_window = static_cast<QMainWindow *>(obs_frontend_get_main_window());
	moduleInstance = new jrNoteDock(main_window);
	//
	obs_frontend_pop_ui_translation();

	return true;
}

void obs_module_unload() {
	if (moduleInstanceIsRegisteredAndAutoDeletedByObs) {
		blog(LOG_INFO, "plugin managed by and should be auto deleted by OBS.");
		return;
	}
	blog(LOG_INFO, "plugin unloading");
	//
	if (moduleInstance != NULL) {
		delete moduleInstance;
		moduleInstance = NULL;
	}
	blog(LOG_INFO, "plugin unloaded");
}
//---------------------------------------------------------------------------





















//---------------------------------------------------------------------------
jrNoteDock::jrNoteDock(QWidget* parent)
	: jrObsPlugin(),
	QDockWidget(parent),
	timer(this)
{
	allocateInitNoteStuff();

	// this will trigger LOAD of settings
	initialStartup();

	// build the dock ui
	buildUi();

	builtNoteStuffUi();
}


jrNoteDock::~jrNoteDock()
{
	destructNoteStuff();

	finalShutdown();

	// this can get called by OBS so we null out the global static pointer if so
	if (moduleInstance == this) {
		moduleInstance = NULL;
	}
}
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------
// statics just reroute to a cast object member function call

void jrNoteDock::ObsFrontendEvent(enum obs_frontend_event event, void *ptr)
{
	jrNoteDock *pluginp = reinterpret_cast<jrNoteDock *>(ptr);
	pluginp->handleObsFrontendEvent(event);
}

void jrNoteDock::ObsHotkeyCallback(void *data, obs_hotkey_id id, obs_hotkey_t *key, bool pressed) {
	if (!pressed)
		return;
	jrNoteDock *pluginp = reinterpret_cast<jrNoteDock *>(data);
	//
	pluginp->handleObsHotkeyPress(id, key);
}
//---------------------------------------------------------------------------














//---------------------------------------------------------------------------
void jrNoteDock::handleObsFrontendEvent(enum obs_frontend_event event) {
	// let parent handle some cases
	jrObsPlugin::handleObsFrontendEvent(event);
}


void jrNoteDock::handleObsHotkeyPress(obs_hotkey_id id, obs_hotkey_t *key) {
}
//---------------------------------------------------------------------------
















//---------------------------------------------------------------------------
JrPluginOptionsDialog* jrNoteDock::createNewOptionsDialog() {
	return new OptionsDialog((QMainWindow *)obs_frontend_get_main_window(), this);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void jrNoteDock::setSettingsOnOptionsDialog(JrPluginOptionsDialog* optionDialog) { setDerivedSettingsOnOptionsDialog(dynamic_cast<OptionsDialog*>(optionDialog)); };
	//
void jrNoteDock::setDerivedSettingsOnOptionsDialog(OptionsDialog* optionDialog) {
	//
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void jrNoteDock::registerCallbacksAndHotkeys() {
	obs_frontend_add_event_callback(ObsFrontendEvent, this);
}

void jrNoteDock::unregisterCallbacksAndHotkeys() {
	obs_frontend_remove_event_callback(ObsFrontendEvent, this);
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

void jrNoteDock::loadStuff(obs_data_t *settings) {
	//
	loadNotes(settings);
}

void jrNoteDock::saveStuff(obs_data_t *settings) {
	//
	saveNotes(settings);
}
//---------------------------------------------------------------------------




























































//---------------------------------------------------------------------------
void jrNoteDock::showEvent(QShowEvent *) {
	timer.start(TIMER_INTERVAL);
}

void jrNoteDock::hideEvent(QHideEvent *) {
	timer.stop();
}




void jrNoteDock::closeEvent(QCloseEvent *event) {
	if (isVisible()) {
		config_set_string(obs_frontend_get_global_config(), "jrNoteDock", "geometry", saveGeometry().toBase64().constData());
		//config_save(obs_frontend_get_global_config());
		//config_save_safe(obs_frontend_get_global_config(), "tmp", nullptr);
	}
	QWidget::closeEvent(event);
}
//---------------------------------------------------------------------------



































































//---------------------------------------------------------------------------
void jrNoteDock::buildUi() {
	// what does this do?
	bool deleteOnClose = moduleInstanceIsRegisteredAndAutoDeletedByObs;

	// ?
	setObjectName(PLUGIN_NAME);

	setFloating(true);
	hide();

	mainLayout = new QVBoxLayout(this);

	auto *dockWidgetContents = new QWidget;
	dockWidgetContents->setLayout(mainLayout);
	setWidget(dockWidgetContents);


	// bottom buttons
	if (false) {
		QPushButton *resetButton = new QPushButton(QTStr("Reset"));
		QHBoxLayout *buttonLayout = new QHBoxLayout;
		buttonLayout->addStretch();
		buttonLayout->addWidget(resetButton);
		connect(resetButton, &QPushButton::clicked, [this]() { Reset(); });
		mainLayout->addStretch();
		mainLayout->addLayout(buttonLayout);
		}

	resize(640, 480);
	setWindowTitle(QTStr(PLUGIN_NAME));

#ifdef __APPLE__
	setWindowIcon(
		QIcon::fromTheme("obs", QIcon(":/res/images/obs_256x256.png")));
#else
	setWindowIcon(QIcon::fromTheme("obs", QIcon(":/res/images/obs.png")));
#endif

	setWindowModality(Qt::NonModal);

	// ?
	if (deleteOnClose) {
		setAttribute(Qt::WA_DeleteOnClose, true);
	}

	// timers
	QObject::connect(&timer, &QTimer::timeout, this, &jrNoteDock::Update);
	timer.setInterval(TIMER_INTERVAL);
	if (isVisible())
		timer.start();

	// initial update
	Update();


	// ATTN: move this
	if (true) {
		const char *geometry = config_get_string(obs_frontend_get_global_config(), "jrNoteDock", "geometry");
		if (geometry != NULL) {
			QByteArray byteArray =
				QByteArray::fromBase64(QByteArray(geometry));
			restoreGeometry(byteArray);
		}

		QRect windowGeometry = normalGeometry();
		if (!WindowPositionValid(windowGeometry)) {
			QRect rect =
				QGuiApplication::primaryScreen()->geometry();
			setGeometry(QStyle::alignedRect(Qt::LeftToRight,
							Qt::AlignCenter, size(),
							rect));
		}
	}

	// add dock
	obs_frontend_add_dock(this);
}
//---------------------------------------------------------------------------



























//---------------------------------------------------------------------------
void jrNoteDock::Update()
{
	if (!flagObsIsFullyLoaded) {
		// avoid running this if not loaded
		return;
	}


	if (firstUpdate) {
		firstUpdate = false;
	}
}
//---------------------------------------------------------------------------
















//---------------------------------------------------------------------------
void jrNoteDock::Reset()
{
	timer.start();
	Update();
}
//---------------------------------------------------------------------------





























//---------------------------------------------------------------------------
void note_frontend_save(obs_data_t *save_data, bool saving, void *data) {
	jrNoteDock *pluginp = reinterpret_cast<jrNoteDock *>(data);
	// pluginp->saveNotes();
}



void jrNoteDock::allocateInitNoteStuff() {
}


void jrNoteDock::builtNoteStuffUi() {

	// the text edit
	textEdit = new QTextEdit(this);

	if (true) {
		mainLayout->setSpacing(0);
		mainLayout->setContentsMargins (0, 0, 0, 0);
		textEdit->setContentsMargins (0, 0, 0, 0);
	}




	// ATTN: jr default font
	if (true) {
		// see also clearformat
		jrSetDefaultFont(textEdit);
	}

	//auto *mainLayout = new QVBoxLayout(this);

	mainLayout->addWidget(textEdit);

	/*
	// already done
	auto *dockWidgetContents = new QWidget;
	dockWidgetContents->setLayout(mainLayout);
	setWidget(dockWidgetContents);
	*/

	auto changeText = [this]() {
		dirtyChanges = true;
		const auto new_notes = textEdit->toPlainText();
		// bug fix attempt for blank text
		if (new_notes == "") {
			jrSetDefaultFont(textEdit);
		}
	};
	connect(textEdit, &QTextEdit::textChanged, changeText);

	textEdit->setContextMenuPolicy(Qt::CustomContextMenu);
	auto contextMenu = [this]() {
		auto *menu = textEdit->createStandardContextMenu();
		if (!textEdit->isReadOnly()) {
			menu->addSeparator();
			auto setFont = [this]() {
				bool success = false;
				const auto font = QFontDialog::getFont(
					&success, textEdit->currentFont(), this,
					QT_UTF8(obs_module_text("Font")));
				if (success)
					textEdit->setCurrentFont(font);
			};
			menu->addAction(QT_UTF8(obs_module_text("Font")), this,
					setFont);

			auto setFontColor = [this]() {
				const QColor newColor = QColorDialog::getColor(
					textEdit->textColor(), this,
					QT_UTF8(obs_module_text("TextColor")));
				if (newColor.isValid()) {
					textEdit->setTextColor(newColor);
				}
			};
			menu->addAction(QT_UTF8(obs_module_text("TextColor")),
					this, setFontColor);
			auto setBackgroundColor = [this]() {
				const QColor newColor = QColorDialog::getColor(
					textEdit->textBackgroundColor(), this,
					QT_UTF8(obs_module_text(
						"BackGroundColor")));
				if (newColor.isValid()) {
					textEdit->setTextBackgroundColor(
						newColor);
				}
			};
			menu->addAction(
				QT_UTF8(obs_module_text("BackgroundColor")),
				this, setBackgroundColor);

			auto listMenu =
				menu->addMenu(QT_UTF8(obs_module_text("List")));

			std::vector<
				std::pair<QTextListFormat::Style, const char *>>
				types{
					{QTextListFormat::ListDisc, "Disc"},
					{QTextListFormat::ListCircle, "Circle"},
					{QTextListFormat::ListSquare, "Square"},
					{QTextListFormat::ListDecimal,
					 "Decimal"},
					{QTextListFormat::ListLowerAlpha,
					 "LowerAlpha"},
					{QTextListFormat::ListUpperAlpha,
					 "UpperAlpha"},
					{QTextListFormat::ListLowerRoman,
					 "LowerRoman"},
					{QTextListFormat::ListUpperRoman,
					 "UpperRoman"},
				};
			for (const auto &it : types) {
				auto t = it.first;
				auto setListType = [this, t]() {
					auto cursor = textEdit->textCursor();
					auto cl = cursor.currentList();
					if (!cl) {
						cursor.createList(t);
					} else {
						auto f = cl->format();
						f.setStyle(t);
						cl->setFormat(f);
					}
				};
				auto a = listMenu->addAction(
					QT_UTF8(obs_module_text(it.second)),
					this, setListType);
				a->setCheckable(true);
				auto cursor = textEdit->textCursor();
				auto cl = cursor.currentList();
				if (cl && cl->format().style() == it.first) {
					a->setChecked(true);
				}
			}

			listMenu->addSeparator();

			auto setListIncr = [this]() {
				auto cursor = textEdit->textCursor();
				QTextBlockFormat blockFormat =
					cursor.block().blockFormat();
				blockFormat.setIndent(blockFormat.indent() + 1);
				cursor.beginEditBlock();
				cursor.setBlockFormat(blockFormat);
				auto cl = cursor.currentList();
				if (cl) {
					cursor.createList(cl->format().style());
				}
				cursor.endEditBlock();
			};
			listMenu->addAction(
				QT_UTF8(obs_module_text("IncreaseIndent")),
				this, setListIncr);

			auto setListDecr = [this]() {
				auto cursor = textEdit->textCursor();
				auto block = cursor.block();
				QTextBlockFormat blockFormat =
					block.blockFormat();
				auto i = blockFormat.indent();
				if (i <= 0) {
					auto cl = block.textList();
					if (cl) {
						cursor.beginEditBlock();
						const auto count = cl->count();
						for (int i = 0; i < count; i++)
							cl->removeItem(0);
						cursor.endEditBlock();
					}
					return;
				}
				cursor.beginEditBlock();
				blockFormat.setIndent(i - 1);
				cursor.setBlockFormat(blockFormat);
				block = cursor.block();
				if (auto cl = block.textList()) {
					auto p = block.previous();
					auto ptll = p.textList();
					if (ptll && p.blockFormat().indent() ==
							    block.blockFormat()
								    .indent()) {
						auto count = cl->count();
						for (int i = 0; i < count;
						     i++) {
							auto item = cl->item(0);
							if (ptll) {
								ptll->add(item);
							} else {
								cl->remove(
									item);
							}
						}
					}
				}
				cursor.endEditBlock();
			};
			listMenu->addAction(
				QT_UTF8(obs_module_text("DecreaseIndent")),
				this, setListDecr);

			menu->addSeparator();
			auto clearFormat = [this]() {
				const auto text = textEdit->toPlainText();
				textEdit->setTextColor(textEdit->palette().color(
					QPalette::ColorRole::Text));
				textEdit->setTextBackgroundColor(
					textEdit->palette().color(
						QPalette::ColorRole::Base));
				jrSetDefaultFont(textEdit);
				textEdit->setPlainText(text);
			};
			menu->addAction(QT_UTF8(obs_module_text("ClearFormat")),
					this, clearFormat);
		}
		menu->addSeparator();

		auto a = menu->addAction(QT_UTF8(obs_module_text("Locked")), this, [this] {
			textEdit->setReadOnly(!textEdit->isReadOnly());
		});
		a->setCheckable(true);
		a->setChecked(textEdit->isReadOnly());

		menu->exec(QCursor::pos());
	};
	connect(textEdit, &QTextEdit::customContextMenuRequested, contextMenu);

	// set deferred initial values
	textEdit->setHtml(QT_TO_UTF8(notesQstr));
	textEdit->setReadOnly(notes_locked);

	obs_frontend_add_save_callback(note_frontend_save, this);
}

void jrNoteDock::jrSetDefaultFont(QTextEdit* te) {
	if (false) {
		textEdit->setCurrentFont(textEdit->font());
	}
	else {
		QFont font = textEdit->currentFont();
		font.setBold(true);
		font.setPointSize(DefFontSize);
		textEdit->setCurrentFont(font);
		textEdit->setFontPointSize(DefFontSize);
	}
}

void jrNoteDock::destructNoteStuff() {
	obs_frontend_remove_save_callback(note_frontend_save, this);
}


void jrNoteDock::loadNotes(obs_data_t *settings) {
	const char *notesbuf = obs_data_get_string(settings, "notes");
	notesQstr = QString(notesbuf);
	notes_locked = obs_data_get_bool(settings, "notes_locked");
	// we cant set it here because it has not been constructed; it will be set later
}


void jrNoteDock::saveNotes(obs_data_t *settings) {
	const auto new_notes = textEdit->toHtml();
	notesQstr = new_notes;
	auto h = notesQstr.toUtf8();
	auto html = h.constData();
	obs_data_set_string(settings, "notes", html);
	obs_data_set_bool(settings, "notes_locked", textEdit->isReadOnly());
	dirtyChanges = false;
}
//---------------------------------------------------------------------------

