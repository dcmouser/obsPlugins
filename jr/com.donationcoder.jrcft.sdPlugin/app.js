/// <reference path="libs/js/action.js" />
/// <reference path="libs/js/stream-deck.js" />


//---------------------------------------------------------------------------
const myAction = new Action('com.donationcoder.jrcft.action_wscommand');
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// setup events tied to each action that are generic websocket config stuff
setupActionWebsocketConfig(myAction);
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
myAction.onKeyUp(({ action, context, device, event, payload }) => {
	let settings = payload.settings;
	doTriggerActionInstance(context, settings);
});
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------
function setupActionWebsocketConfig(actionObj) {
	actionObj.onSendToPlugin(({action, event, context, payload}) => {
	  //console.log('onSendToPlugin received', payload);
		if (payload.command) {
			if (payload.command=="updateConfig") {
				// plugin sends us updates vals for status and settings
				updateConfig(payload.vals, true);
				// should we try to connect now?
				if (true) {
					connectThenUpdatePropertyInspectorStatus(context);
				}
			} else if (payload.command=="testAction") {
				// test the action
				let settings = payload.settings;
				doTriggerActionInstance(context, settings);
			}
		}
	});

	actionObj.onPropertyInspectorDidAppear(({ action, event, context, device }) => {
		sendConfigSettingsToPropertyInspector(context);
	});
}
//---------------------------------------------------------------------------















//---------------------------------------------------------------------------
// The first event fired when Stream Deck starts

$SD.onConnected(({ actionInfo, appInfo, connection, messageType, port, uuid }) => {
	//console.log('JrCft Stream Deck connected..');
	//
	doStuffOnConnect();
});


$SD.onDidReceiveGlobalSettings(({event, payload}) => {
	let settings = payload.settings;
	//console.log("In onDidReceiveGlobalSettings");
	//
	if (settings) {
		//console.log('got onDidReceiveGlobalSettings', payload, settings);
		updateConfig(settings, false);
	}
});
//---------------------------------------------------------------------------















//---------------------------------------------------------------------------
// helpers

function sendConfigSettingsToPropertyInspector(context) {
	let wsAddress = globalAddress;
	let wsPassword = globalPasswd;
	let connectionStatus = globalConnectionStatus;
	let lastError = globalLastError;
	payload = {command:"updateVals", vals: {wsAddress: wsAddress, wsPassword: wsPassword, connectionStatus:connectionStatus, lastError:lastError}};
	$SD.sendToPropertyInspector(context, payload, "com.donationcoder.jrcft.action_wscommand");
}


function sendConfigStatusToPropertyInspector(context) {
	//console.log("In sendConfigStatusToPropertyInspector");
	let connectionStatus = globalConnectionStatus;
	let lastError = globalLastError;
	payload = {command:"updateVals", vals: {connectionStatus:connectionStatus, lastError:lastError}};
	//console.log(payload);
	$SD.sendToPropertyInspector(context, payload, "com.donationcoder.jrcft.action_wscommand");
}


function connectThenUpdatePropertyInspectorStatus(context) {
	doConnectPromise().then(()=> {
			// success
			sendConfigStatusToPropertyInspector(context);
		}, (flagRetryable)=> {
			// failure
			sendConfigStatusToPropertyInspector(context);
		});
}


function updateConfig(vals, flagSaveSettings) {
	let wsAddress = vals.wsAddress;
	let wsPassword = vals.wsPassword;
	// set globals that obs websocket code sees
	globalAddress = wsAddress;
	globalPasswd = wsPassword;
	//console.log("In updateconfig in plugin.");
	//
	if (flagSaveSettings) {
		let payload = {wsAddress: wsAddress, wsPassword: wsPassword};
		//let pluginContext = "com.donationcoder.jrcft.sdPlugin";
		//console.log("Setting global settings",payload);
		$SD.setGlobalSettings(payload);
	}
}


function doStuffOnConnect() {
	loadPluginData();
}


function loadPluginData() {
	// request load websocket address and password
	//console.log("In loadPluginData.");
	$SD.getGlobalSettings();
}


function onConnectionStatusChange(context) {
	// broadcast connection status change to inspector?
	sendConfigStatusToPropertyInspector(context);
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
function doTriggerActionInstance(context, settings) {
	let vendor = settings.vendor;
	let request = settings.request;
	let jsonPayloadStr = settings.jsonPayloadStr;

	// promise based async wrapper to connect first if need be
	sdCheckObsConnection(context).then((isNewlyConnected) => {
		// connected, now send command
  	needsStatusUpdate = sdSendWsVendorCommandJsonPayloadString(context, vendor, request, jsonPayloadStr);
  	if (isNewlyConnected || needsStatusUpdate) {
   		// update any status message
   		onConnectionStatusChange(context);
  	}
  }, ()=>{
  	// update any status message on failure
   		onConnectionStatusChange(context);
  	});
}
//---------------------------------------------------------------------------
