/// <reference path="libs/js/action.js" />
/// <reference path="libs/js/stream-deck.js" />

//---------------------------------------------------------------------------
var globalAddress;
var globalPasswd;
//
var globalLastError;
var globalConnectionStatus;
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
const myAction = new Action('com.donationcoder.jrcft.action_wscommand');
//---------------------------------------------------------------------------










//---------------------------------------------------------------------------
myAction.onKeyUp(({ action, context, device, event, payload }) => {
	let settings = payload.settings;
	doTriggerActionInstance(context, settings);
});


// setup events tied to each action that are generic websocket config stuff
setupActionWebsocketConfig(myAction);
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
function doTriggerActionInstance(context, settings) {
	let vendor = settings.vendor;
	let request = settings.request;
	let jsonPayloadStr = settings.jsonPayloadStr;

	// promise based async wrapper to connect first if need be
	sdCheckObsConnection(context).then(() => {
		// connected, now send command
  	sdSendWsVendorCommandJsonPayloadString(context, vendor, request, jsonPayloadStr);
  }, ()=>{});
}
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
function onConnectionStatusChange() {
	// broadcast connection status change to inspector?
}


function loadPluginData() {
	// request load websocket address and password
	//console.log("In loadPluginData.");
	$SD.getGlobalSettings();
}








//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// The first event fired when Stream Deck starts

$SD.onConnected(({ actionInfo, appInfo, connection, messageType, port, uuid }) => {
	//console.log('JrCft Stream Deck connected..');
	//
	doStuffOnConnect();
});
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
$SD.onDidReceiveGlobalSettings(({event, payload}) => {
	let settings = payload.settings;
	//console.log("IN DID RECEIVE GLOBAL SETTINGS1!");
	//console.log(payload);
	//console.log(settings);
	//
	if (settings) {
		//console.log('got onDidReceiveGlobalSettings', payload, settings);
		updateConfig(settings, false);
	}
});
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
function doStuffOnConnect() {
	loadPluginData();

	/*
	/// should we try to start connecting immediately and never give up? or just connect on first us
	let flagConnectWithRetry = false;
	if (flagConnectWithRetry) {
 		console.log('Initiating obs websocket connections..');
		doStartup(flagConnectWithRetry);
	}
	*/
}
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
// helpers

function sendConfigSettingsToPropertyInspector(picontext) {
	let wsAddress = globalAddress;
	let wsPassword = globalPasswd;
	let connectionStatus = globalConnectionStatus;
	let lastError = globalLastError;
	payload = {command:"updateVals", vals: {wsAddress: wsAddress, wsPassword: wsPassword, connectionStatus:connectionStatus, lastError:lastError}};
	$SD.sendToPropertyInspector(picontext, payload, "com.donationcoder.jrcft.action_wscommand");
}

function sendConfigStatusToPropertyInspector(picontext) {
	//console.log("In sendConfigStatusToPropertyInspector");
	let connectionStatus = globalConnectionStatus;
	let lastError = globalLastError;
	payload = {command:"updateVals", vals: {connectionStatus:connectionStatus, lastError:lastError}};
	//console.log(payload);
	$SD.sendToPropertyInspector(picontext, payload, "com.donationcoder.jrcft.action_wscommand");
}






function connectThenUpdatePropertyInspectorStatus(picontext) {
	doConnectPromise().then(()=> {
			// success
			sendConfigStatusToPropertyInspector(picontext);
		}, (flagRetryable)=> {
			// failure
			sendConfigStatusToPropertyInspector(picontext);
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
//---------------------------------------------------------------------------

