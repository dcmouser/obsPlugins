// global
var currentTargetDiv = "jrYtInteriorAltDiv1";



function doStartup() {
	doConnect(globalAddress,globalPasswd)
}

function mydebug(msg) {
	console.log("jrytws debug: " + msg);
}
function myerror(msg) {
	console.log("jrytws error: " + msg);
	setJrYtDivInnerHtml("ERROR: " + msg)
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

	obs.on("VendorEvent", (data) => {		
		//console.log(data);
		if (data.vendorName=="jrYoutube") {
			if (data.eventType == "jrYtMessageChanged") {
				doMessageSelected(data.eventData);
			}
		}
	});
	
	// now force the first update
	obs.call("CallVendorRequest", {vendorName:"jrYoutube",requestType:"JrYtMessageGet"}).then((data) => {
		if (data.responseData) {
			// got reply!
			doMessageSelected(data.responseData);
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
	// render message
	let innerHtml;
	let authorName = "";
	let authorImageUrl = "";
	let msgLen
	
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

	if (typeof data === 'string' || data instanceof String) {
		innerHtml = data;
		msgLen = innerHtml.length;
	} else {
		// parse data
		innerHtml = data.msgHtml;
		msgLen = innerHtml.length;
		authorName = data.authorName;
		authorImageUrl = data.authorImageUrl;
		msgLen = data.plainTextLen;
		if (authorName!="") {
			innerHtml = '<span class="jrYtAuthorName">' + authorName + '</span>' + innerHtml;
		}
		if (authorImageUrl!="") {
			innerHtml = '<img class="jrYtUserImage" src="' + authorImageUrl + '"/><div class="jrYtMessage">' + innerHtml + "</div>";
		}
	}

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


