// global
var currentTargetDiv = "jrYtInteriorAltDiv1";
var obs;


function doStartup() {
	doConnect(globalAddress,globalPasswd)
}

function mydebug(msg) {
	console.log("jrytws debug: " + msg);
}
function myerror(msg) {
	console.log("jrytws error: " + msg);
	let divMain = document.getElementById(currentTargetDiv);	
	divMain.innerHTML = "<div>ERROR: " + msg+"</div><div>Address: " + globalAddress + "<br/>Password: " + globalPasswd+ "</div>";
	//alert(msg);
}








function doConnect(address, passwd) {
	// set up OBS web socket
  obs = new OBSWebSocket();

	obs.on("ConnectionOpened", () => {
		mydebug("Connection Opened");
	});

	obs.on("Identified", () => {
		mydebug("Identified, good to go!");
	});

	let connectArgs = {
		eventSubscriptions: (1 << 0) | (1 << 9)
	}

	obs.connect(address, passwd).then((info) => {
		// this comes after we are fully connected
		mydebug("Connected and identified", info)
		registerForWsEvents()
	}, () => {
		myerror("Error Connecting")
	});
}


function registerForWsEvents() {
	// register to get jryt plugin message changed events

	// main obs plugin
	obs.on("VendorEvent", (data) => {		
		//console.log(data);
		if (data.vendorName=="jrYoutube") {
			//alert("event: " + data.eventType)
			if (data.eventType == "jrYtMessageChanged") {
				doMessageSelected(data.eventData);
			}
			else if (data.eventType == "jrYtMessageListCleared") {
				doMessageListCleared(data.eventData);
			}
			else if (data.eventType == "jrYtMessageAdded") {
				doMessageAdded(data.eventData);
			}
			else if (data.eventType == "jrYtMessageListChanged") {
				doMessageListChanged(data.eventData);
			}
			else {
				alert("Unknown event: " + data.eventType)
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

	// get LIST of all messages
	obs.call("CallVendorRequest", {vendorName:"jrYoutube",requestType:"JrYtListGet"}).then((data) => {
		if (data.responseData) {
			// got reply!
			doFillList(data.responseData);
		}
	});
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
	
	//alert("Message added: " + innerHtml);

	// create list item
	let divUl = document.getElementById("JrYtMessageListUl");
  let li = document.createElement("li");
	let el = createHTMLNode(innerHtml);
	el.setAttribute("data-rowinex", rowindex);
	li.appendChild(el);

	// add click
	li.addEventListener("click", function() { clickItemRow(li, rowindex);}, false);
  
	// set class based on length
	setClassBasedOnLength(li, msgLen);

	// add it to list
  divUl.appendChild(li);
  
  // scroll into view?
  if (wasLastItemVisible) {
		li.scrollIntoView();
  }
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
	divUl.replaceChildren();
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
	
	// should be an array
	
	// first clear
	doMessageListCleared("");
	
	let listJsonStr = data.listJson;
	let listJsonArray = JSON.parse(listJsonStr)
	
	
	// now walk through object array
	var len = listJsonArray.length;
	for (let i = 0; i < len; i++) {
		doMessageAdded(listJsonArray[i])
	  //console.log(jsonListArray[i]);
	}
}




function sendSelectItem() {
	// tell the websocket client the user has selected an item
	// now force the first update to get currently selected item
	sendJrYtMessageSelect(2);
}


function clickButtonTestSelect() {
	sendSelectItem();
}

function clickButtonClearSelection() {
	sendJrYtMessageSelect(-1);
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
    //var htmlNode = document.createElement('span');
    var htmlNode = document.createElement('div');
    htmlNode.innerHTML = htmlCode
    //htmlNode.className = 'treehtml';
    return htmlNode;
}




