// global
var globalJrYtVersion = "2.0 (3/12/23 by mouser@donationcoder.com)";
var currentTargetDiv = "jrYtInteriorAltDiv1";
var obs;
var jrytMode;
var kludgeDontAutoScrollToEndTime = 0;
var reconnectTimeMs = 5000;
var globalAddress, globalPasswd;









function mydebug(msg) {
	console.log("jrytws debug: " + msg);
}
function myerror(msg) {
	console.log("jrytws error: " + msg);
	addInfoMsg("ERROR: " + msg, true);
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











function doStartupOverlay() {
  jrytMode = "overlay";
	doStartup()
}

function doStartupControl() {
  jrytMode = "control";
	doStartup()
}

function getInControlMode() {
	if (jrytMode == "control") {
		return true;
	}
	return false;
}




function doStartup() {
	loadOptions();
	doConnect(globalAddress, globalPasswd)
}




function reStartupIfDisconnected() {
	if (!obs || !obs.socket) {
		// try to reconnect
		myinfo("Attempting to reconnect to address: " + globalAddress);
		doConnect(globalAddress, globalPasswd);
	} else {
		// console.log("jrytws checked already running.");
	}
}




function doConnect(address, passwd) {

	// set up OBS web socket
  obs = new OBSWebSocket();
  
  if (getInControlMode()) {
  	myinfo("Starting up connection to: " + globalAddress);
  }

	obs.on("ConnectionOpened", () => {
		mydebug("Connection Opened");
	});

	obs.on("Identified", () => {
		mydebug("Identified, good to go!");
	});

	obs.on("ConnectionError", (err) => {
		mydebug("Connection error: " + err);
		// set up one time timer to recheck
		setTimeout(reStartupIfDisconnected, reconnectTimeMs);
	});
	obs.on("ConnectionClosed", (err) => {
		myerror("Connection closed; " + err);
		// set up one time timer to recheck
		setTimeout(reStartupIfDisconnected, reconnectTimeMs);
	});

	let connectArgs = {
		eventSubscriptions: (1 << 0) | (1 << 9)
	}

	try {
		myinfoQuiet("Connecting to: "+globalAddress);
		obs.connect(address, passwd).then((info) => {
			// this comes after we are fully connected
			mydebug("Connected and identified", info)
			registerForWsEvents()
			clearDebugMsg();
		}, () => {
			myerror("Error Connecting")

		});
	} catch (err) {
		myerror("Exception: " + err);
	}
}


function registerForWsEvents() {
	// register to get jryt plugin message changed events

	let controlMode = getInControlMode();
	
	// main obs plugin
	obs.on("VendorEvent", (data) => {		
		//console.log(data);
		if (data.vendorName=="jrYoutube") {
			if (data.eventType == "jrYtMessageChanged") {
				doMessageSelected(data.eventData);
			}

      else if (controlMode) {
      	// only check these in control mode
    			if (data.eventType == "jrYtMessageListCleared") {
    				doMessageListCleared(data.eventData);
    			}
    			else if (data.eventType == "jrYtMessageAdded") {
    				doMessageAdded(data.eventData);
    			}
    			else if (data.eventType == "jrYtMessageListChanged") {
    				doMessageListChanged(data.eventData);
    			}
    			else if (data.eventType == "jrYtStateChange") {
    				doHandleStateChangeMessage(data.eventData);
    			}
    			else {
    				myerror("Unknown event: " + data.eventType)
    			}
    		}
    	}
    });


   	// now force the first update to get currently selected item
   	obs.call("CallVendorRequest", {vendorName:"jrYoutube",requestType:"JrYtMessageGet"}).then((data) => {
   		if (data.responseData) {
   			// got reply!
   			doMessageSelected(data.responseData);
   		}
   	});

    if (controlMode) {
	   	// get LIST of all messages
	   	obs.call("CallVendorRequest", {vendorName:"jrYoutube",requestType:"JrYtListGet"}).then((data) => {
	   		if (data.responseData) {
	   			// got reply!
	   			doFillList(data.responseData);
	   		}
	   	});

	   	obs.call("CallVendorRequest", {vendorName:"jrYoutube",requestType:"JrYtGetState"}).then((data) => {
	   		if (data.responseData) {
	   			// got reply!
	   			doHandleStateMessage(data.responseData);
	   		}
	   	});

	  }

}



function setClassBasedOnLength(div, len) {
	// remove all length classes
	div.classList.remove("msgLen1","msgLen2","msgLen3");
	// add new one
	let msgLenClass;
	if (len<=101) {
		msgLenClass = "msgLen1";
	} else if (len<=201) {
		msgLenClass = "msgLen2";
	} else {
		msgLenClass = "msgLen3";
	}
	div.classList.add(msgLenClass);
}







function doMessageSelected(data) {
	// alternate
	let lastTargetDiv = currentTargetDiv;
	if (currentTargetDiv=="jrYtInteriorAltDiv1") {
		currentTargetDiv = "jrYtInteriorAltDiv2";
	} else {
		currentTargetDiv = "jrYtInteriorAltDiv1";
	}
	let divCurrent = document.getElementById(currentTargetDiv);
	let divLast = document.getElementById(lastTargetDiv);
	//
	let divMain = document.getElementById("jrYtMainDiv");

	// build html
	let result = buildMessageHtml(data);
	let innerHtml = result[0];
	let msgLen = result[1];
	let rowindex = result[2];


	// set class based on length
	setClassBasedOnLength(divCurrent, msgLen);

	// set it
	divCurrent.innerHTML = innerHtml;
	
	// hide non-displaying one
	divLast.style.display = "none";
	
	if (innerHtml=="") {
		// hide internal completely?
		divCurrent.style.display = "none";
		divMain.style.display = "none";
	} else {
		// make sure it's visible
		divCurrent.style.display = "";
		divMain.style.display = "";
	}
	
	//jump to it in list?
	if (getInControlMode()) {
		jumpToAndSelectHighlightedMessage(rowindex);
	}
}





function buildMessageHtml(data) {
	let innerHtml;
	let msgLen;
	let rowindex;

	let authorName = "";
	let authorImageUrl = "";
	
	if (typeof data === 'string' || data instanceof String) {
		innerHtml = data;
		msgLen = innerHtml.length;
		if (innerHtml!="") {
			innerHtml = "<p>" + innerHtml + "</p>";
		}
	} else {
		// parse data
		rowindex = data.index;
		innerHtml = data.msgHtml;
		msgLen = innerHtml.length;
		authorName = data.authorName;
		authorImageUrl = data.authorImageUrl;
		msgLen = data.plainTextLen;
		if (authorName!="") {
			innerHtml = '<span class="jrYtAuthorName">' + authorName + '</span>' + innerHtml;
		}
		if (innerHtml!="" || (false && getInControlMode())) {
			innerHtml = "<p>" + innerHtml + "</p>";
		}
		if (authorImageUrl!="") {
			innerHtml = '<img class="jrYtUserImage" src="' + authorImageUrl + '"/><div class="jrYtMessage">' + innerHtml + '</div><div class="imgClearing"><div>';
		}
	}
	
	return [innerHtml, msgLen, rowindex]
}





function doMessageAdded(data) {
	// build html
	
	let wasLastItemVisible = isLastItemVisible();

	let result = buildMessageHtml(data);
	let innerHtml = result[0];
	let msgLen = result[1];
	let rowindex = result[2];

	// create list item
	let divUl = document.getElementById("JrYtMessageListUl");
  let li = document.createElement("li");
	let el = createHTMLNode(innerHtml);
	li.setAttribute("data-rowindex", rowindex);
	li.appendChild(el);

	// add click
	li.addEventListener("click", function() { clickItemRow(li, rowindex);}, false);
  
	// set class based on length
	setClassBasedOnLength(li, msgLen);

	// mark info messages
	if (innerHtml[0]==']') {
		li.classList.add("jrytInfo");
	}

	// add it to list
  divUl.appendChild(li);
  
  // scroll into view?
  if (wasLastItemVisible && !checkKludgeDontAutoScrollToEndTime()) {
		li.scrollIntoView();
  }
}





function goToFirstItem() {
	goToItemByPosition(0);
	updateKludgeDontAutoScrollToEndTime();
}

function goToLastItem() {
	let divUl = document.getElementById("JrYtMessageListUl");
	let items = divUl.getElementsByTagName("li");
	goToItemByPosition(items.length-1);
	resetKludgeDontAutoScrollToEndTime();
}


function goToItemByPosition(index) {
	let divUl = document.getElementById("JrYtMessageListUl");
	let items = divUl.getElementsByTagName("li");
	if (index<0 || items.length<=index) {
		return;
	};
	let li = items[index];
	if (li) {
		li.scrollIntoView({ behavior: "smooth", block: "center", inline: "nearest" });
	}
}

function updateKludgeDontAutoScrollToEndTime() {
	kludgeDontAutoScrollToEndTime = Date.now();
}
function resetKludgeDontAutoScrollToEndTime() {
	kludgeDontAutoScrollToEndTime = 0;
}

function checkKludgeDontAutoScrollToEndTime() {
	let nowtime = Date.now();
	let elapsed = nowtime - kludgeDontAutoScrollToEndTime;
	if (elapsed<750) {
		return true;
	}
	return false;
}




function isLastItemVisible() {

	let divUl = document.getElementById("JrYtMessageListUl");
	let items = divUl.getElementsByTagName("li");
	if (items.length<=0) {
		return true;
	};
	let li = items[items.length-1];

	let retv = isElementVisible(li, divUl);
	return retv;
}

function isElementVisible (el, holder) {
  holder = holder || document.body
  const { top, bottom, height } = el.getBoundingClientRect()
  const holderRect = holder.getBoundingClientRect()

  return top <= holderRect.top
    ? holderRect.top - top <= height
    : bottom - holderRect.bottom <= height
}





function clickItemRow(li, rowindex) {
	clearAllOtherSelectedItems();
	li.classList.add("jrSelected");
	sendJrYtMessageSelect(rowindex);
}

function clearAllOtherSelectedItems() {
	let divUl = document.getElementById("JrYtMessageListUl");
	let items = divUl.getElementsByTagName("li");
	for (let i = 0; i < items.length; ++i) {
		let li = items[i];
		li.classList.remove("jrSelected");
	}
}

function doMessageListCleared(data) {
	let divUl = document.getElementById("JrYtMessageListUl");
	doReplaceChildren(divUl);
}


function doReplaceChildren(el) {
	// not supported on all browsers?
	//el.replaceChildren();
	let child;
	while (child = el.firstChild) {
    	child.remove();
	}
}



function doMessageListChanged(data) {
	// get LIST of all messages
	obs.call("CallVendorRequest", {vendorName:"jrYoutube",requestType:"JrYtListGet"}).then((data) => {
		if (data.responseData) {
			// got reply!
			doFillList(data.responseData);
		}
	});
}


function doFillList(data) {
	// build full list
	
	// first clear
	doMessageListCleared("");
	
  // now walk through object array
	let listJsonStr = data.listJson;
	let listJsonArray = JSON.parse(listJsonStr)
	let len = listJsonArray.length;
	for (let i = 0; i < len; i++) {
		doMessageAdded(listJsonArray[i])
	}
}






function clickOverlayDiv() {
	sendJrYtMessageSelect(-1);
}



function sendJrYtMessageSelect(index) {
	let reqData = {"itemid":index};
	obs.call("CallVendorRequest", {vendorName:"jrYoutube",requestType:"JrYtMessageSelect", requestData:reqData}).then((data) => {
		if (data.responseData) {
			// got reply, but we don't care about it
		}
	});
}






function createHTMLNode(htmlCode) {
    // create html node
    //let htmlNode = document.createElement('span');
    let htmlNode = document.createElement('div');
    htmlNode.innerHTML = htmlCode
    return htmlNode;
}



function sendWsCommand(commandCode) {
	//console.log("Sending ws command: " + commandCode);

	if (commandCode=="gotoFirst") {
		goToFirstItem();
	}
	if (commandCode=="gotoLast") {
		goToLastItem();
	}

	let reqData = {"command":commandCode};
	obs.call("CallVendorRequest", {vendorName:"jrYoutube",requestType:"JrYtCommand", requestData:reqData}).then((data) => {
		if (data.responseData) {
			// got reply, but we don't care about it
		}
	});
}







function jumpToAndSelectHighlightedMessage(rowindex) {
	let divUl = document.getElementById("JrYtMessageListUl");
	let items = divUl.getElementsByTagName("li");
	for (let i = 0; i < items.length; ++i) {
		let li = items[i];
		let liRowIndex = li.getAttribute("data-rowindex");
		if (liRowIndex==rowindex) {
			li.classList.add("jrSelected");
			li.scrollIntoView({ behavior: "smooth", block: "center", inline: "nearest" });
		} else {
			li.classList.remove("jrSelected");
		}
	}
}



function doHandleStateMessage(data) {
	doHandleStateChangeMessage(data);
}



function doHandleStateChangeMessage(data) {
	if (data.autoToggle != undefined) {
		setAutoToggleButtonState(data.autoToggle);
	}
}




function toggleAutoToggleButtonState() {
	let val = getAutoToggleButtonState();
	val = !val;
	setAutoToggleButtonState(val);
}


function setAutoToggleButtonState(val) {
	let buttonEl = document.getElementById("JrYtButtonAutoToggle");
	if (val) {
		buttonEl.classList.add("JrYtToggledOn");
		buttonEl.classList.remove("JrYtToggledOff");
	} else {
		buttonEl.classList.add("JrYtToggledOff");
		buttonEl.classList.remove("JrYtToggledOn");
	}
}

function getAutoToggleButtonState() {
	let buttonEl = document.getElementById("JrYtButtonAutoToggle");
	return buttonEl.classList.contains("JrYtToggledOn");
}





function doToggleAutoButton() {
	toggleAutoToggleButtonState();
	sendWsCommand('toggleAuto');
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
		divVersionInfo.innerHTML = "JrYoutube Chat v." + globalJrYtVersion;
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

	document.getElementById("connectionAddress").value = globalAddress;
	document.getElementById("connectionPassword").value = globalPasswd;
}

function saveOptions() {
	// get globalAddress, globalPasswd
	let storage = window.localStorage;
	
	globalAddress = document.getElementById("connectionAddress").value;
	globalPasswd = document.getElementById("connectionPassword").value;
	
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