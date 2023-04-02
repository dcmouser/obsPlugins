// global
var globalJrYtVersion = "2.2 (3/12/23 by mouser@donationcoder.com)";
//
var obs;
//
var reconnectTimeMs = 10000;
//
var globalAddress = "";
var globalPasswd = "";
//
var globalLastError = "none";
var globalConnectionStatus = "unconnected";
//
var globalLastUseTime = 0;








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
	let div = document.getElementById("infoDiv");
	if (!div) {
		return;
	}
	if (msg=="") {
		// make it invisible?
		div.innerHTML = "";
		div.style.display = "none";
	} else {
		let timestr = new Date().toLocaleString();
		div.innerHTML = div.innerHTML + "<p>" + timestr + " -> " + msg + "</p>";	
		//div.innerHTML = "<div>" + timestr + " -> " + msg+"</div>";		
		if (flagForceShow) {
			div.style.display = "block";
		}
	}
}




function doStartup(flagConnectWithRetry) {
	loadOptions();
	setupStuff();
	if (flagConnectWithRetry) {
		doConnectWithRetry();
	}
}

function setupStuff() {
	/*
	// longpress support 
	document.addEventListener('long-press', function(ev) {  triggerLongPressOnElement(ev); });
	*/
}



function isObsConnected() {
	return (obs && obs.socket);
}


function reStartupIfDisconnected() {
	if (!isObsConnected()) {
		// try to reconnect
		myinfo("Attempting to reconnect to address: " + globalAddress);
		doConnectWithRetry();
	} else {
		// mydebug("jrytws checked already running.");
	}
}











































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























function saveOptionsButtonClick() {
	saveOptions();
	// reload
	location.reload();
	/*
	// hide options dialog
	let div = document.getElementById("optionsDiv");
	div.style.display = "none";
	*/
}


function optionsButtonClick() {
	let div = document.getElementById("optionsDiv");
	if (div.style.display == "none" || div.style.display == "") {
		// show it
		loadOptions();
		let divVersionInfo = document.getElementById("versionInfo");
		divVersionInfo.innerHTML = "JrWsController v." + globalJrYtVersion;
		//
		div.style.display = "block";
	} else {
		// hide it
		div.style.display = "";
	}
}

function loadOptions() {
	// get globalAddress, globalPasswd
	let storage = window.localStorage;
	
	globalAddress = readStorageVal(storage, "globalAddress", globalAddress);
	globalPasswd = readStorageVal(storage, "globalPasswd", globalPasswd);
	
	// explicit on url?
	processCommandlineArgs();
	
	if (!globalAddress) {
		globalAddress = "ws://localhost:4455";
	}
	if (!globalAddress) {
		globalPasswd = "";
	}
	
	if (document.getElementById("optionsDiv")) {
		document.getElementById("connectionAddress").value = globalAddress;
		document.getElementById("connectionPassword").value = globalPasswd;
	}
}

function saveOptions() {
	// get globalAddress, globalPasswd
	let storage = window.localStorage;

	if (document.getElementById("optionsDiv")) {
		globalAddress = document.getElementById("connectionAddress").value;
		globalPasswd = document.getElementById("connectionPassword").value;
	}
	
	writeStorageVal(storage, "globalAddress", globalAddress);
	writeStorageVal(storage, "globalPasswd", globalPasswd);
}



function readStorageVal(storage, varname, defaultval) {
	let val = storage.getItem("JrYt." + varname);
	if (val === undefined || val === null) {
		return defaultval;
	}
	return val;
}


function writeStorageVal(storage, varname, val) {
	storage.setItem("JrYt." + varname, val);
}


function processCommandlineArgs() {
	let ip;
	let portval;
	let passwd;
	
	var goodArgCount = 0;
	var params = window.location.search.slice(1).split("&");
	for(var p=0; p<params.length; p++) {
			var nv = params[p].split("=");
			var name = nv[0], value = nv[1];

		if (name=="") {
			continue;
		}
		if (name=="ip") {
			ip = value;
		} else if (name=="port") {
			portval = value;
		} if (name=="passwd") {
			passwd = value;
		}
	}
	
	if (ip) {
		globalAddress = "ws://" + ip + ":" + portval;
	}
	if (passwd) {
		globalPasswd = passwd;
	}
	
	
}


























function sdCheckObsConnection(context) {
	// return a promise so caller can do a then() on it
	if (isObsConnected()) {
		// return "promise" that just executes the then part
		const p1 = new Promise((resolve, reject) => {
	  	resolve("Success!");
	  	// or reject(new Error("Error!"));
			});
		return p1;
	}	
		
	// we create a promise that tries to connect so the user can do something afterwards
	const p1 = new Promise((resolve, reject) => {
		doConnectPromise().then(()=>{
			// sucess
			resolve("Success!");
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
		return false;
	}
	
	// async call and eventual reply
	let reqData
	try {
		reqData = JSON.parse(jsonPayloadStr);
	} catch (err) {
		myerror("Faled to parse json payload to send to jrcft command: ");
		myerror(err);
		if (context) {
			$SD.showAlert(context);
		}
		return false;
	}

	mydebug("In sendWsCommandJsonPayloadString, vendor: "+vendor +", request: " + request +", jsonpayload: " + jsonPayloadStr);
	obs.call("CallVendorRequest", {vendorName:vendor,requestType:request, requestData:reqData}).then((data) => {
		mydebugObj(data.responseData);
		if (data.responseData) {
			// got reply, but we don't care about it
  		if (data.responseData.result) {
		  	// show succcess
		  	if (context) {
  				$SD.showOk(context);
  			}
  		} else if (data.responseData.error) {
				mydebugObj(data.responseData);
		  	// show succcess
		  	if (context) {
  				$SD.showAlert(context);
  			}
  		}
		}
	});

	// sent message
	return true;
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
			myerror("Connection failed: " + myErrorToString(err));
			globalConnectionStatus = "ERROR: Connection failed (" + myErrorToString(err) + ")";
			// reject and say we should try to reconnect
			reject(true);
		});

		obs.on("ConnectionClosed", (err) => {
			//console.log(err);
			myerror("Connection closed: " + myErrorToString(err));
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
				if (false) {
					// error will have been set via another callback, so dont overwrite here
					myerror(err.toString());
					//myerror("Error Connecting:" + err.toString())
					//globalConnectionStatus = "Error connecting: "+err.toString();
				}
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





function myErrorToString(err) {
	if (typeof err === 'string' || err instanceof String) {
		return err;
	}
	return JSON.stringify(err);
}