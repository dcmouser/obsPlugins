JrDockie plugin for OBS
v1.3 (7/10/24)
by Jesse Reichler aka dcmouser <jessereichler@gmail.com)
https://github.com/dcmouser/obsPlugins/tree/main/jr
My YouTube channel: https://www.youtube.com/@COOPFORTWO 

------

This plugin adds a new "Dock Sets" SUBMENU to your toolbar (by default under Docks/).
From this submenu you can quicky save and restore your current OBS layout (configuration of docks open, sizes, positions, etc.)

------

INSTALLATION:
This plugin has been compiled for WINDOWS 64bit
Install plugin as typical by putting the jrdockie.dll file into your obs-plugins/64bit folder.

INSTRUCTIONS:

When you run OBS you should have a new menu under the Docks/ main toolbar called "Dock Sets"
(if that top level menu cannot be found you will see a new top-level menu in your toolbar called "Dock Sets").

You can access the options dialog from that menu or from the Tools/JrDockie menu.

To use, simply choose to SAVE your current dockset into a file, or LOAD an existing one you have previously saved.
The menu will automatically detect dockset files and show them on the recently used bottom areas of the menu, so you should save your dockset files in the recommended location for the plugin.
To manually cleanup (delete, rename, etc.) the dockset files, use the menu option to open the folder directory.

------

WEBSOCKET COMMANDS:

1. To load a specific dockset by filename:

Call with Vendor: "JrDockie"
Request command: "LoadDockset"
JSON payload: {"filename": "dockfilenameorfullpathwithorwithoutextension"}


2. To cycle to through the docksets:

Call with Vendor: "JrDockie"
Request command: "CycleDockset"
JSON payload: {}


------

Hotkeys:

Hotkey for CycleDockset
(search for JrDockie in hotkey filters)
