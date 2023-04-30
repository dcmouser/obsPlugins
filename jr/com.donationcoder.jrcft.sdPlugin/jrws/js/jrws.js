// global
var globalJrYtVersion = "2.3 (4/03/23 by mouser@donationcoder.com)";
//
var obs;
//
var reconnectTimeMs = 5000;
//
var globalAddress = "";
var globalPasswd = "";
var globalLastError = "none";
var globalConnectionStatus = "unconnected";








//---------------------------------------------------------------------------
function mydebug(msg) {
	console.log("jrytws debug: " + msg);
}
function mydebugObj(obj) {
	console.log(obj);
}
function myerror(msg) {
	console.log("jrytws error: " + msg);
	addInfoMsg("ERROR: " + msg, true);
	globalLastError = msg;
}
function myinfo(msg) {
	console.log("jrytws info: " + msg);
	addInfoMsg(msg, true);
}
function myinfoQuiet(msg) {
	// add text but dont force visible
	console.log("jrytws info: " + msg);
	addInfoMsg(msg, false);
}
function clearDebugMsg() {
	addInfoMsg("",true);
}

function addInfoMsg(msg, flagForceShow) {
	// not used in streamdeck version
}

function myErrorToString(err) {
	if (typeof err === 'string' || err instanceof String) {
		return err;
	}
	return JSON.stringify(err);
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// not used in this code
function doConnectWithRetry() {
	// return a promise so caller can do a then() on it
	if (isObsConnected()) {
		// connected
		return;
	}

	// try to connect, and if fail schedule a reconnect
	doConnectPromise().then(()=> {
		// success
	}, (flagRetryable)=> {
		// failure
		if (flagRetryable) {
			//
			setTimeout(doConnectWithRetry, reconnectTimeMs);
		}
	});
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
function registerForWsEvents() {
	// register to get jryt plugin message changed events

	// main obs plugin
	obs.on("VendorEvent", (data) => {		
		//mydebug(data);
		if (data.vendorName=="jrCft") {
   				myerror("Unknown event: " + data.eventType)
 		}
   });
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
function isObsConnected() {
	return (obs && obs.socket);
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
function sdCheckObsConnection(context) {
	// return a promise so caller can do a then() on it
	if (isObsConnected()) {
		// return "promise" that just executes the then part
		const p1 = new Promise((resolve, reject) => {
			if (globalLastError!=="") {
				// clear obs error
				globalLastError="";
				resolve(true);
			} else {
				// resolve saying we dont need to show stat
	  		resolve(false);
	  	}
			});
		return p1;
	}	
		
	// we create a promise that tries to connect so the user can do something afterwards
	const p1 = new Promise((resolve, reject) => {
		doConnectPromise().then(()=>{
			// sucess
			resolve(true);
			},
		()=>{
			// failure. report then return promise rejection
			if (context) {
				$SD.showAlert(context);
			}
			//myerror("Failed to connect prior to issuing command.");
			reject(new Error("Failed to connect."));
			});
	});
	return p1;
}





function sdSendWsVendorCommandJsonPayloadString(context, vendor, request, jsonPayloadStr) {
	if (!isObsConnected()) {
		// connection error
  	if (context) {
			$SD.showAlert(context);
		}
		myerror("OBS not connected to jrcft streamdeck plugin.");
		return true;
	}
	
	// async call and eventual reply
	let reqData
	try {
		reqData = JSON.parse(jsonPayloadStr);
	} catch (err) {
		myerror("Faled to parse json payload to send to jrcft command: "+err.toString());
		if (context) {
			$SD.showAlert(context);
		}
		// return true saying we need to update status
		return true;
	}

	mydebug("In sendWsCommandJsonPayloadString, vendor: "+vendor +", request: " + request +", jsonpayload: " + jsonPayloadStr);
		obs.call("CallVendorRequest", {vendorName:vendor,requestType:request, requestData:reqData}).then((data) => {
			//mydebugObj(data.responseData);
			if (data.responseData) {
				// got reply, but we don't care about it
	  		if (data.responseData.result) {
			  	// show succcess
			  	if (context) {
	  				$SD.showOk(context);
	  			}
	  		} else if (data.responseData.error) {
					myerror(data.responseData.error);
			  	// show succcess
			  	if (context) {
	  				$SD.showAlert(context);
	  				// do we need to call this or will caller?
	  				onConnectionStatusChange(context);
	  			}
	  		}
			}
		}).catch((err)=> {
			myerror("Faled2 to invoke OBS websocket command: "+err.toString());
			if (context) {
				$SD.showAlert(context);
   			// update any status message
   			onConnectionStatusChange(context);
			}
  	});

	// sent message, no need to auto display new status
	return false;
}



// test command useful for testing from web page
function senTestdWsCommandTestToggleVisibility() {
	let vendor = "JrCft";
	let request = "JrCftCommand";
	let jsonPayloadStr = '{"command": "toggleSourceVisibility", "sourceName": "Overlay - Front Inset", "optionAllScenes": false}';

	if (true) {
		// async with connection attempt
		sdCheckObsConnection().then(() => {
	  	sdSendWsVendorCommandJsonPayloadString(null, vendor, request, jsonPayloadStr);
	  }, () => {
				// failed to connect
				$SD.showAlert(context);
	  });
	} else {
		sdSendWsVendorCommandJsonPayloadString(null, vendor, request, jsonPayloadStr);
	}
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
function doConnectPromise() {

	let address = globalAddress;
	let passwd = globalPasswd;

	const p1 = new Promise((resolve, reject) => {
		// run the promise return resolve or reject
		
		if (!globalAddress.startsWith("ws://")) {
			errMsg = "Connection failed: Bad address; should start with ws://";
			myerror(errMsg);
			globalConnectionStatus = "Not connected.";
			globalLastError = errMsg;
			reject("Bad address.");
			return;
		}

		// clear errors
	 	globalConnectionStatus = "Connecting..";
	  globalLastError = "";

		// set up OBS web socket
	  obs = new OBSWebSocket();
	 	myinfo("Starting up connection to: " + globalAddress);

		obs.on("ConnectionOpened", () => {
			mydebug("Connection Opened");
			globalConnectionStatus = "Connection opened, pending identification..";
		});

		obs.on("Identified", () => {
			mydebug("Identified, good to go!");
			globalConnectionStatus = "Connection identified, pending negotiation..";		
		});

		obs.on("ConnectionError", (err) => {
			//console.log(err);
			myerror("Connection failed (bad address or server offline?): " + myErrorToString(err));
			globalConnectionStatus = "ERROR: Connection failed (" + myErrorToString(err) + ")";
			// reject and say we should try to reconnect
			reject(true);
		});

		obs.on("ConnectionClosed", (err) => {
			//console.log(err);
			myerror("Connection closed (bad password?): " + myErrorToString(err));
			globalConnectionStatus = "ERROR: Connection closed (" + myErrorToString(err) + ")";
			// reject and say we should try to reconnect
			reject(true);
		});

		let connectArgs = {
			eventSubscriptions: (1 << 0) | (1 << 9)
		}

		// so far so good
		try {
			myinfoQuiet("Connecting to: "+globalAddress);
			obs.connect(address, passwd).then((info) => {
				// this comes after we are fully connected
				mydebug("Connected and identified", info)
				registerForWsEvents()
				clearDebugMsg();
				globalConnectionStatus = "Successfully connected to: "+globalAddress;
				resolve("connected");
			}, (err) => {
				// error will be set in a separate obs.on callback
				// reject and say we shouldn't try to reconnect
				reject(false);
			});
		} catch (err) {
			//myerror("Exception: " + myErrorToString(err));
			globalConnectionStatus = "ERROR: Exception encountered when trying to connect (" + myErrorToString(err) + ")";
			// reject and say we shouldn't try to reconnect
			reject(false);
		}
	});
	
	// ok return the promise
	return p1;
}
//---------------------------------------------------------------------------








