var globalAddress;
var globalPasswd;
//
var globalLastError;
var globalConnectionStatus;



window.onload = () => {
	doOnLoad();
};


function doOnLoad() {
	// fill settings
	fillConfigSettings();
}



function fillConfigSettings() {

	//alert("FROM config, globalConnectionStatus is " + globalConnectionStatus)

	let connectionAddress = document.getElementById("connectionAddress");
	connectionAddress.value = globalAddress;
	//
	let connectionPassword = document.getElementById("connectionPassword");
	connectionPassword.value = globalPasswd;
}

