// global
var globalJrYtVersion = "2.0 (3/12/23 by mouser@donationcoder.com)";
var currentTargetDiv = "jrYtInteriorAltDiv1";
var obs;
var jrytMode;
var kludgeDontAutoScrollToEndTime = 0;
var reconnectTimeMs = 5000;
var globalAddress, globalPasswd;
var useStarCheckboxes = true;
//
var elapsedTimerStarTime = 0;
var timerIntervalId = -1;








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
	setupStuff();
	doConnect(globalAddress, globalPasswd)
}

function setupStuff() {
	/*
	// longpress support 
	document.addEventListener('long-press', function(ev) {  triggerLongPressOnElement(ev); });
	*/
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
				//console.log("GOT jrYtMessageChanged.");
				doMessageSelected(data.eventData);
			}

      else if (controlMode) {
      	// only check these in control mode
    			if (data.eventType == "jrYtMessageListCleared") {
    				doMessageListCleared(data.eventData);
    			}
    			else if (data.eventType == "jrYtMessageAdded") {
    				doMessageAdded(data.eventData, true, false);
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




    if (controlMode) {
	   	// get LIST of all messages
	   	obs.call("CallVendorRequest", {vendorName:"jrYoutube",requestType:"JrYtListGet"}).then((data) => {
	   		if (data.responseData) {
	   			// got reply!
	   			doFillList(data.responseData);

			   	// now force the first update to get currently selected item
			   	// note we do this AFTER we get list reply and build list so that we can select the active one
			   	obs.call("CallVendorRequest", {vendorName:"jrYoutube",requestType:"JrYtMessageGet"}).then((data) => {
			   		if (data.responseData) {
			   			// got reply!
			   			doMessageSelected(data.responseData);
			   		}
			   	});
	   		}
	   	});

			// a problem is this can happen AFTER we fill list so we move it inside the reply
	   	obs.call("CallVendorRequest", {vendorName:"jrYoutube",requestType:"JrYtGetState"}).then((data) => {
	   		if (data.responseData) {
	   			// got reply!
	   			doHandleStateMessage(data.responseData);
	   		}
	   	});

	  } else {
	  	// non control mode
	   	// force the first update to get currently selected item
	   	// note we do this AFTER we get list reply and build list so that we can select the active one
	   	obs.call("CallVendorRequest", {vendorName:"jrYoutube",requestType:"JrYtMessageGet"}).then((data) => {
	   		if (data.responseData) {
	   			// got reply!
	   			doMessageSelected(data.responseData);
	   		}
	   	});
	  }

}



function setClassBasedOnLength(div, len) {
	// remove all length classes
	div.classList.remove("msgLen1","msgLen2","msgLen3","msgLen4","msgLen5","msgLen6","msgLen7","msgLen8");
	// add new one
	let msgLenClass;
	//console.log(len);
	if (len<=101) {
		msgLenClass = "msgLen1";
	} else if (len<=201) {
		msgLenClass = "msgLen2";
	} else if (len<=401) {
		msgLenClass = "msgLen3";
	} else if (len<=801) {
		msgLenClass = "msgLen4";
	} else if (len<=1501) {
		msgLenClass = "msgLen5";
	} else {
		msgLenClass = "msgLen6";
	}
	div.classList.add(msgLenClass);
}







function doMessageSelected(wrappedData) {
	// wrapped value
	//console.log("WRAPPED:");
	//console.log(wrappedData);
	let data = JSON.parse(wrappedData.data);
	
	// alternate
	console.log("ATTN: in doMessageSelected");
	console.log(data);
	
	if (data.index==-1) {
		// clear it
		cancelElapsedTimerUpdater();
		clearMainMessage();
		return;
	}

	let highlighted = data.highlighted;
	let messageChanged = data.messageChanged;
	
	if (highlighted) {
		// build html
		// ATTN: in this version of code this isnt needed if highlighted is not set
		let result = buildMessageHtml(data, false);
		let innerHtml = result[0];
		let msgLen = result[1];
		let rowindex = result[2];
		//let msgForList = result[3];

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
			divMain.style.display = "block";
		}

		if (getInControlMode() && messageChanged) {
			// force change in list as well, since the item has changed
			doMessageAdded(data, false, true);
		}
		
		//jump to it in list?
		if (getInControlMode()) {
			jumpToAndSelectHighlightedMessage(rowindex);
		}
		
	} else {
		// REPLACEMENT IN LIST
		if (getInControlMode()) {
			doMessageAdded(data, false, true);
		} else {
			// not pertinent to overlay
		}
	}
}


function clearMainMessage() {
			// hide internal completely?
			let divCurrent = document.getElementById(currentTargetDiv);
			let divMain = document.getElementById("jrYtMainDiv");
			divCurrent.style.display = "none";
			divMain.style.display = "none";


			let div = document.getElementById("jrYtInteriorAltDiv1");
			if (div) {
				div.innerHtml="";
			}
			div = document.getElementById("jrYtInteriorAltDiv2");
			if (div) {
				div.innerHtml="";
			}


			// jrselected?
			jumpToAndSelectHighlightedMessage(-1);
}




function buildMessageHtml(data, flagUseListMsg) {
	let innerHtml;
	let msgLen;
	let rowindex;
	let msgForList;
	
	//
	//console.log("buildMessageHtml:");
	//console.log(data);

	let authorName = "";
	let authorImageUrl = "";
	
	// parse data
	rowindex = data.index;
	innerHtml = data.messageHtml;
	if (!innerHtml || (flagUseListMsg && data.showPlainInList)) {
		// fall back to plain text
		innerHtml = data.messageSimplePlaintext;
	}

	let extraClass = "";
	msgLen = innerHtml.length;
	authorName = data.authorName;
	authorImageUrl = data.authorImageUrl;
	msgLen = data.plainTextLen;
	if (data.rowcount) {
		// table based msgLen estimator
		msgLen = parseInt(data.rowcount) * 100;
		extraClass = " JrYtRowCounted";
	}

	if (authorName!="") {
		//innerHtml = '<span class="jrYtAuthorName">' + authorName + afterAuthorText + '</span>' + innerHtml;
		innerHtml = '<span class="jrYtAuthorName' + extraClass + '">' + authorName + '</span>' + innerHtml;
	}
	
	
	if (innerHtml!="" || (false && getInControlMode())) {
		innerHtml = '<p class="jrYtMsgp">' + innerHtml + "</p>";
	}
	if (authorImageUrl!="") {
		if (useStarCheckboxes && flagUseListMsg) {
			innerHtml = '<img class="jrYtUserImage" src="' + authorImageUrl + '" onclick="onClickStarAvatar(this,event);" /><div class="jrYtMessage">' + innerHtml + '</div><div class="imgClearing"></div>';				
		} else {
			innerHtml = '<img class="jrYtUserImage" src="' + authorImageUrl + '"/><div class="jrYtMessage">' + innerHtml + '</div><div class="imgClearing"></div>';
		}
	}
	

	let showingElapsedTime = false;
	if (!flagUseListMsg) {
		let startTime = data.startTime;
		if (startTime) {
			let endTime = data.endTime;
			if (!endTime) {
				elapsedTimerStarTime = startTime;
				let timeElapsedString = calcTimeElapsedString(startTime);
				elapsedTimeText = ' <div id="JrYtElapsedTimer" class="JrYtElapsedTimer">' + timeElapsedString + '</div>';
				innerHtml = elapsedTimeText + innerHtml;
				showingElapsedTime = true;
			}
		}
	setVisibilityElapsedTimerUpdate(showingElapsedTime);
	}



	
	return [innerHtml, msgLen, rowindex, msgForList]
}



function calcTimeElapsedString(startTime) {
	let nowTime = Math.round(new Date() / 1000);
	let elapsed = Math.round(nowTime - startTime);
	if (elapsed<0) {
		elapsed = 0;
	}
	//
	let totalSeconds = elapsed;
	let hours = Math.floor(totalSeconds / 3600);
	totalSeconds = totalSeconds % 3600;
	let minutes = Math.floor(totalSeconds / 60);
	let seconds = totalSeconds % 60;
	//
	let timeString = (hours>0) ? (hours.toString().padStart(2, '0') + ':') : '' + 
    minutes.toString().padStart(2, '0') + ':' + 
    seconds.toString().padStart(2, '0');
   return timeString;
}





function doMessageAdded(data, flagIsWrapped, flagReplace) {
	// build html

//	console.log("In doMessageAdded:");
	if (flagIsWrapped) {
		data = JSON.parse(data.data);
	}
//console.log("In doMessageAdded:");
//console.log(data);

	let result = buildMessageHtml(data, true);
	let innerHtml = result[0];
	let msgLen = result[1];
	let rowindex = result[2];
	let msgForList = result[3];
	let isStarred = data.starred;
	let isHighlighted = data.highlighted;
	
	// add star?
	//console.log(data);
	if (useStarCheckboxes) {
		let starVal = "";
		if (isStarred) {
			starVal = "checked"
		}
		innerHtml =  '<div class="jrYtMsgMainFlex">' + innerHtml + '</div><div class="jrYtStarDiv" onclick="onClickStar(this,event);"></div>';
	} else {
		innerHtml = innerHtml + '<input type="checkbox" name="jrYtStar" class="jrYtStar JrYtHideNoStars" />';		
	}
	
	let wasLastItemVisible = isLastItemVisible();

	// create list item
	let divUl = document.getElementById("JrYtMessageListUl");
  let li = document.createElement("li");
	let el = createHTMLNode(innerHtml);
	el.classList.add("jrYtListMessageOuter");
	li.setAttribute("data-rowindex", rowindex);
	li.appendChild(el);
		
	// add click
	li.addEventListener("click", function() { clickItemRow(li, rowindex);}, false);

	// mark info messages
	if (innerHtml[0]==']') {
		li.classList.add("jrytInfo");
	}
	if (isStarred) {
		li.classList.add("jrYtStarred");
	}
	
	// base class for li
	li.classList.add("jrytListMessage");

  // set class based on length
	setClassBasedOnLength(li, msgLen);
	
	// add it to list
	if (flagReplace) {
		//console.log("ATTN: trying to replace node - 1.");
		//console.log(rowindex);
		// find and replace existing
		let items = divUl.getElementsByTagName("li");
		for (let i = 0; i < items.length; ++i) {
			let ali = items[i];
			let liRowIndex = ali.getAttribute("data-rowindex");
			//console.log("found ri: " + liRowIndex);
			if (liRowIndex==rowindex) {
				// found it
				//console.log("ATTN: trying to replace node - 2.");
				divUl.replaceChild(li,ali);
			}
		}
	} else {
		// normal thing we do is append
  	divUl.appendChild(li);
	}

  // scroll into view?
  if (wasLastItemVisible && !checkKludgeDontAutoScrollToEndTime() && !flagReplace) {
		li.scrollIntoView();
  }
 
	//jump to it in list?
	if (getInControlMode() && isHighlighted) {
		jumpToAndSelectHighlightedMessage(rowindex);
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
	let dataListStr = data.dataList;
	let dataListArray = JSON.parse(dataListStr)
	let len = dataListArray.length;
	for (let i = 0; i < len; i++) {
		doMessageAdded(dataListArray[i], false, false)
	}
	
	// show tabs?
	recheckTabVisibility();
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




function triggerLongPressOnElement(ev) {
	console.log("Long press on: " + ev.target.id + " / " + ev.target.className);
	//console.log(el);
	// cancel normal click?
	return false;
}

function onClickStarAvatar(el,ev) {
	ev.stopPropagation(); 
	let parentDiv = (el.parentElement).parentElement;
	//let checkboxEl = parentDiv.getElementsByTagName("input")[0];
	let checkboxEl = parentDiv.getElementsByClassName("jrYtStarDiv")[0];
	if (!checkboxEl) {
		return;
	}
	checkboxEl.click();
	//checkboxEl.checked = !checkboxEl.checked;
	//onClickStar(checkboxEl,ev);
}



function onClickStar(el,ev) {
	//console.log("ATTN: in onClickStar");
	ev.stopPropagation(); 
	// send starred state change of item to server
	let liEl = (el.parentElement).parentElement; 
	let newState = !liEl.classList.contains("jrYtStarred");

	let itemId = liEl.getAttribute("data-rowindex");
	sendJrYtMessageStarStateChange(itemId, newState);
	if (newState) {
		liEl.classList.add("jrYtStarred");
	} else {
		liEl.classList.remove("jrYtStarred");
	}
	// update whether to show starred tab
	recheckTabVisibility();
}


function recheckTabVisibility() {
	// decide whether to show tabs for starred vs unstarred items (or whether to hide starred tab if there are no starred items)
	let el = document.getElementById("JrYtButtonStars");
	let isAnyStarred = isAnyStarredItems();
	let div = document.getElementsByTagName("BODY")[0];
	let showingAll = div.classList.contains("JrYtShowAllMsgs");

	if (isAnyStarred) {
		el.classList.add("JrYtShowStars");
		// change text based on showingAll?
		if (showingAll) {
			el.innerHTML = '<img class="JrYtButtonImg" src="images/check_unchecked_light.png">';
		} else {
			el.innerHTML = '<img class="JrYtButtonImg" src="images/check_checked_light.png">';			
		}
	} else {
		el.classList.remove("JrYtShowStars");
		// if we are in stars only view, drop out? at least don't hide button
	}
}




function sendJrYtMessageStarStateChange(itemid, state) {
	let reqData = {"itemid":parseInt(itemid), "state":state};
	/*
	console.log("attn: should send server info about star state change:");
	console.log(reqData);
	return;
	*/

	obs.call("CallVendorRequest", {vendorName:"jrYoutube",requestType:"JrYtMsgStarState", requestData:reqData}).then((data) => {
		if (data.responseData) {
			// got reply, but we don't care about it
		}
	});
}


function optionsStarsViewClick() {
	// toggle whehter the list shows ONLY stars or all
	let div = document.getElementsByTagName("BODY")[0];
	let showingAll = div.classList.contains("JrYtShowAllMsgs");
	if (showingAll) {
		div.classList.remove("JrYtShowAllMsgs");		
		div.classList.add("JrYtShowOnlyStarMsgs");	
	} else {
		div.classList.remove("JrYtShowOnlyStarMsgs");	
		div.classList.add("JrYtShowAllMsgs");		
	}
	recheckTabVisibility();
}


function isAnyStarredItems() {
	return (countStarredItems()>0);
}
	
function countStarredItems() {
	let divUl = document.getElementById("JrYtMessageListUl");
	let items = divUl.getElementsByTagName("li");
	let starCount = 0;
	for (let i = 0; i < items.length; ++i) {
		let li = items[i];
		if (li.classList.contains("jrYtStarred")) {
			++starCount;
		}
	}
	return starCount;
}





function setVisibilityElapsedTimerUpdate(isVisible) {
	if (!isVisible && timerIntervalId!=-1) {
		cancelElapsedTimerUpdater();
		return;
	}
	if (isVisible && timerIntervalId==-1) {
		initiatedElapsedTimerUpdater();
		return;
	}
	// no change
}

function cancelElapsedTimerUpdater() {
	clearInterval(timerIntervalId);
	timerIntervalId = -1;
	//console.log("Canceling update timer.");
}

function initiatedElapsedTimerUpdater() {
	let timerFreq = 500;
	timerIntervalId = setInterval(updateElapsedTimerContents, timerFreq);
	//console.log("Initiated update timer.");
}

function updateElapsedTimerContents() {
	// find the div
	let parentEl = document.getElementById(currentTargetDiv);
	if (!parentEl) {
		cancelElapsedTimerUpdater();
		return;
	}
	let els = parentEl.getElementsByClassName("JrYtElapsedTimer");
	if (!els) {
		cancelElapsedTimerUpdater();
		return;		
	}
	let el = els[0];
	if (!el) {
		cancelElapsedTimerUpdater();
		return;
	}
	//console.log(el);
	let timeString = calcTimeElapsedString(elapsedTimerStarTime);
	el.innerHTML  = timeString;
}

