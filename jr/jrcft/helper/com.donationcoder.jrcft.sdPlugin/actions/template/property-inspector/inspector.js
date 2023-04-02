/// <reference path="../../../libs/js/property-inspector.js" />
/// <reference path="../../../libs/js/utils.js" />
/// <reference path="../../../libs/js/utils.js" />

$PI.onConnected((jsn) => {
    const form = document.querySelector('#property-inspector');
    const {actionInfo, appInfo, connection, messageType, port, uuid} = jsn;
    const {payload, context} = actionInfo;
    const {settings} = payload;

    Utils.setFormValue(settings, form);

    form.addEventListener(
        'input',
        Utils.debounce(150, () => {
            const value = Utils.getFormValue(form);
            $PI.setSettings(value);
        })
    );
});

$PI.onDidReceiveGlobalSettings(({payload}) => {
    //console.log('onDidReceiveGlobalSettings', payload);
})



function htmlUnescape(str){
    return str
        .replace(/&quot;/g, '"')
        .replace(/&#39;/g, "'")
        .replace(/&lt;/g, '<')
        .replace(/&gt;/g, '>')
        .replace(/&amp;/g, '&');
}


/*
// doesnt work yet
document.querySelector('#checkRequest').addEventListener('click', () => {
	let vendor = document.getElementById("vendor").value;
	let request = document.getElementById("request").value;
	let jsonPayloadStrEscaped = document.getElementById("jsonPayloadStr").value;
	let jsonPayloadStr = jsonPayloadStrEscaped;
});
*/

if (document.getElementById("configureObsWebsocket")) {
	document.querySelector('#configureObsWebsocket').addEventListener('click', () => {
    	window.open('../../../config.html');
	});
}

if (document.getElementById("saveConfig")) {
	document.querySelector('#saveConfig').addEventListener('click', (event) => {
		saveAndSendConfigSettingsToPlugin();
	});
}

if (document.getElementById("testAction")) {
	document.querySelector('#testAction').addEventListener('click', (event) => {
		AskPluginToTestAction();
	});
}





/** 
 * TABS
 * ----
 * 
 * This will make the tabs interactive:
 * - clicking on a tab will make it active
 * - clicking on a tab will show the corresponding content
 * - clicking on a tab will hide the content of all other tabs
 * - a tab must have the class "tab"
 * - a tab must have a data-target attribute that points to the id of the content
 * - the content must have the class "tab-content"
 * - the content must have an id that matches the data-target attribute of the tab
 * 
 *  <div class="tab selected" data-target="#tab1" title="Show some inputs">Inputs</div>
 *  <div class="tab" data-target="#tab2" title="Here's some text-areas">Text</div>
 * a complete tab-example can be found in the index.html
   <div type="tabs" class="sdpi-item">
      <div class="sdpi-item-label empty"></div>
      <div class="tabs">
        <div class="tab selected" data-target="#tab1" title="Show some inputs">Inputs</div>
        <div class="tab" data-target="#tab2" title="Here's some text-areas">Text</div>
      </div>
    </div>
    <hr class="tab-separator" />
 * You can use the code below to activate the tabs (`activateTabs` and `clickTab` are required)
 */

function activateTabs(activeTab) {
    const allTabs = Array.from(document.querySelectorAll('.tab'));
    let activeTabEl = null;
    allTabs.forEach((el, i) => {
        el.onclick = () => clickTab(el);
        if(el.dataset?.target === activeTab) {
            activeTabEl = el;
        }
    });
    if(activeTabEl) {
        clickTab(activeTabEl);
    } else if(allTabs.length) {
        clickTab(allTabs[0]);
    }
}

function clickTab(clickedTab) {
    const allTabs = Array.from(document.querySelectorAll('.tab'));
    allTabs.forEach((el, i) => el.classList.remove('selected'));
    clickedTab.classList.add('selected');
    activeTab = clickedTab.dataset?.target;
    allTabs.forEach((el, i) => {
        if(el.dataset.target) {
            const t = document.querySelector(el.dataset.target);
            if(t) {
                t.style.display = el == clickedTab ? 'block' : 'none';
            }
        }
    });
}

activateTabs();






//$PI.onSendToPropertyInspector("com.donationcoder.jrcft.action_wscommand", ({action, event, context, payload}) => {
$PI.onSendToPropertyInspector("com.donationcoder.jrcft.action_wscommand", ({action,payload}) => {
  //console.log('onSendToPropertyInspector', payload);
	if (payload.command && payload.command=="updateVals") {
		// plugin sends us updates vals for status and settings
		updateStatus(payload.vals);
	}
});



function saveAndSendConfigSettingsToPlugin() {
	let wsAddress = document.getElementById("wsAddress").value;
	let wsPassword = document.getElementById("wsPassword").value;
	payload = {command:"updateConfig", vals: {wsAddress: wsAddress, wsPassword: wsPassword}};
	$PI.sendToPlugin(payload);
	//console.log("Sent to plugin from inspector.");
}


function AskPluginToTestAction() {
  const form = document.querySelector('#property-inspector');
	let settings = {};
  // extract form values
  if (false) {
		settings = {vendor: document.getElementById("vendor").value, request: document.getElementById("request").value, jsonPayloadStr: document.getElementById("jsonPayloadStr").value };
	} else {
		const elements = form?.elements;
		Array.from(elements)
			.filter((element) => element?.name)
			.forEach((element) => {
				const { name, type } = element;
				settings[name]=element.value;
				});
	}

	payload = {command:"testAction", settings:settings};
	$PI.sendToPlugin(payload);
}




function updateStatus(vals) {
	let eldiv = document.getElementById("statuInfoDiv");
	if (eldiv && vals.connectionStatus) {
		eldiv.innerHTML = vals.connectionStatus;
	}
	eldiv = document.getElementById("statuInfoDiv2");
	if (eldiv && vals.connectionStatus) {
		eldiv.innerHTML = vals.connectionStatus;
	}
	//
	eldiv = document.getElementById("lastErrorInfoDiv");
	if (eldiv && vals.lastError) {
		eldiv.innerHTML = vals.lastError;
	}
	eldiv = document.getElementById("lastErrorInfoDiv2");
	if (eldiv && vals.lastError) {
		eldiv.innerHTML = vals.lastError;
	}
	//
	eldiv = document.getElementById("wsAddress");
	if (eldiv && vals.wsAddress) {
		eldiv.value = vals.wsAddress;
	}
	eldiv = document.getElementById("wsPassword");
	if (eldiv && vals.wsPassword) {
		eldiv.value = vals.wsPassword;
	}
}