// version 11/29/21
// mouser@donationcoder.com / co-op for two on youtube



		// see https://stackoverflow.com/questions/9375014/sending-parameters-to-local-html-file
		function processArgs() {
			var goodArgCount = 0;
			var params = window.location.search.slice(1).split("&");
			for(var p=0; p<params.length; p++) {
  			var nv = params[p].split("=");
  			var name = nv[0], value = nv[1];

				if (name=="") {
					continue;
				}
  			
  			if (name=="countdown") {
  				var mins = parseFloat(value);
					if (isNaN(mins)) {
						doShowError("ERROR: countdown value should be specified as an floating point # (found: '" + value+"'); specifying minutes.");
						return;
					}
  				setupTimedBreak(mins);
  				initDisplayTimerBegin();
  				++goodArgCount;
  			}
  			
  			else if (name=="countup") {
  				var mins = parseFloat(value);
					if (isNaN(mins)) {
						doShowError("ERROR: countup value should be specified as a floating point # (found: '" + value+"')<br/>that specifies when countup timer expires.<br/>Use zero for no end.");
						return;
					}
					modeCountUp = true;
  				setupCountUpTimer(mins);
  				initDisplayTimerBegin();
  				++goodArgCount;
  			}
 
  			else if (name=="starts") {
					//value should be of the form "DEC 19, 2021 at 11:00 pm";
  				// date and time set
					//alert("ATTN: VALUE = " + value)
					var ovalue = value;
  				value = value.replace(/\%20/gi," ");
  				value = value.replace(/\./gi,",");
					//alert("ATTN: VALUE = " + value)
					const valArray = value.split(" at ");
					dateString = valArray[0];
					timeString = valArray[1];

					var bretv = setScheduleTimeAndThumbnailHeaderFromEventDateTime(ovalue, dateString, timeString);
					if (bretv) {
						setupStartingSoon();
						++goodArgCount;
					}
				}

  			else if (name=="finish") {
  				value = value.replace(/\%20/gi," ");
					finishMessage = value;
  				++goodArgCount;
  			}
  			
  			else if (name=="center") {
					centerLayout = true;
  				++goodArgCount;
					document.getElementById("flipdown").style.margin = "auto";
					document.getElementById("flipdown_done").style.margin = "auto";
  			}
   			else if (name=="left") {
					centerLayout = false;
  				++goodArgCount;
					//document.getElementById("flipdown").style.margin = "auto";
					//document.getElementById("flipdown_done").style.margin = "auto";
  			}


				else {
					doShowError("unknown commandline parameter passed: " + name)
				}
			}
			
			return goodArgCount;
		}







		function setScheduleTimeAndThumbnailHeaderFromEventDateTime(ovalue, dateString, timeString) {
			// helper function called from index.html
			// Automatic stuff that shouldn't need changing
			// schedule countdown timer for a 5 minutes after the scheduled time
			
			//
			var dateval;
			if (dateString=="now") {
				dateval = Date.now();
			} else {
				dateval = new Date(dateString + " " + timeString);
			}
			
			if (isNaN(dateval.getTime())) {
				doShowError("ERROR: Date time string not understood:\n'" + ovalue + "'\nDate parsed as: '" + dateString + "' and time parsed as: '" + timeString + "'.\nShould look like 'DEC 19, 2021 at 11:00 pm'.");
				return false;
			}
			scheduleTime = (dateval.getTime() / 1000);// + (5*60);
			// alert(new Date(scheduleTime*1000).toString())
			// string to show the date in nice way (removes year, add "at" to time)
			niceScheduleString = dateString.replace(/\,\s*20\d\d/i,"") + " AT " + timeString;
			return true;
		}



		function doShowError(str) {
			if (globalError!="") {
				globalError += "\n";
			}
			globalError += str;
			// show it in alert
			alert(str);
			//
			updateGlobalError();
		}
		
		function updateGlobalError() {
			if (globalError!="") {
				var globalErrorHtml = globalError.replace(/\n/g,"<br/>\n");
				document.getElementById("globalError").innerHTML = globalErrorHtml;
				document.getElementById("globalError").style.display = "";
			} else {
				document.getElementById("globalError").style.display = "none";
			}
		}


		function setupStartingSoon() {
			resetVars();
			headerText = "STARTING SOON...";
			endTime = scheduleTime;
			showTimer = true;
			showTertiary = false;
			initDisplayTimerBegin();
		}




		function setupTimedBreak(mins) {
			resetVars();
			setTimerMinutes(mins);
			headerText = mins.toString() + " MINUTE BREAK IN PROGRESS";
			secondaryText = "WE WILL BE RIGHT BACK!";
			// secondaryText = "&nbsp;";
			showTimer = true;
			showTertiary = false;
			initDisplayTimerBegin();
		}
		
		
		
		function setupCountUpTimer(mins) {
			resetVars();
			endTime = (new Date().getTime() / 1000);
			countUpEndSeconds = mins * 60;
			headerText = "BREAK IN PROGRESS";
			secondaryText = "WE WILL BE RIGHT BACK!";
			// secondaryText = "&nbsp;";
			showTimer = true;
			showTertiary = false;
			initDisplayTimerBegin();
		}






		

		function doSetup() {
			resetVars();
			updateBackgroundColor();
			//setupContextMenu();
			initDisplayTimerBegin(false);

			// new, process any args in url
			var goodArgCount = processArgs();
			if (goodArgCount == 0) {
				// error
				doShowError("No valid commandline arguement passed. Should be one of:\nindex.html?countdown=# (e.g. index.html?countdown=5)\nindex.html?starts=date at time (e.g.: 'index.html?starts=DEC 19, 2021 at 11:00 pm\nyou can also pass finish=end message.'); you can also add commandline option 'center' to horizontally center the text.");
			}
		}
		
		function initDisplayTimerBegin() {
			updateDisplay();
			initTimer();
		}


		function updateDisplay() {
			// background color
			updateBackgroundColor();

			if (showTimer) {
				document.getElementById("flipContainter").style.display = "flex";
			} else {
				document.getElementById("flipContainter").style.display = "none";
			}

		}


		function setIdFontSize(idName, fontSize) {
			// this can be set to fontSize "" to go back to defaults
			document.getElementById(idName).style.fontSize = fontSize;
			//alert("ATTN: setIdFontSize " + idName + " size = " + fontSize);
		}

		function setTimerMinutes(mins) {
			endTime = (new Date().getTime() / 1000) + mins*60;
		}



	  function initTimer() {
	  	// delete any existing timer
	  	document.getElementById("flipdown").innerHTML = "";
	  	
	  	if (flipdown) {
	  		// stop prior one timer?
	  		flipdown.countdownEnded = true;
	  		flipdown.hasEndedCallback = null;
    		clearInterval(flipdown.countdown);
    		flipdown.countdown = null;
	  	}

	  	if (showTimer) {
	  		var opts = {
	  			modeCountUp: modeCountUp,
	  			countUpEndSeconds: countUpEndSeconds,
	  			};
		  	flipdown = new FlipDown(endTime, undefined, opts).start();
		  } else {
		  	flipdown = null;
		  }
	  }
	  
	  
	  function globalCallbackTimerEnds() {
			var doneEl = document.getElementById("flipdown_done");
			doneEl.innerHTML = finishMessage;

	  }














		function setBackgroundBlack() {
			setBackgroundColorDirectly(backgroundColorBlack);
		}




		function toggleBackgroundChroma() {
			if (backgroundColor==backgroundColorNormal) {
				backgroundColor = backgroundColorTransparent;
			} else {
				backgroundColor = backgroundColorNormal;
			}
			updateBackgroundColor();
		}
		
		function updateBackgroundColor() {
			setBackgroundColorDirectly(backgroundColor);
			//alert("update background color = "+backgroundColor);
		}
		
		function setBackgroundColorDirectly(color) {
			document.body.style.backgroundColor = color;
		}
	


		function resetVars() {
			// defaults
			setDefaultVarValues();
			// user values
			setVars();
		}
		
		
		function setDefaultVarValues() {
			endTime = null;
			headerText = "RIGHT-CLICK IMAGE FOR MENU";
			mainText = "&nbsp;";
			secondaryText = "&nbsp;";
			tertiaryText = "";
			tertiaryTextSupportUs = "&nbsp;&nbsp;&nbsp;&nbsp;support us at:<br/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;www.patreon.com/coopfortwo";
			showTimer = false;
			showTertiary = false;
			// dont clear audio variable !
			// audio = null;
			backgroundColorNormal = "#ffedb4"
			//backgroundColorTransparent = "#00b140"
			backgroundColorTransparent = "#b4f268"
			backgroundColorBlack = "#000000";

			fontsizeMain = "";
			fontsizeSecondary = "";
			fontsizeTertiary = "";
			//
			delayAfterVideoReturn = 2000;
			//
			introVideoFilename = "introVideo/intro.mp4";
			//
			videoFileNames = [];
			//
			lastVideoFileLoaded = "";

			// values not to override by default
			if (backgroundColor == undefined) {
				backgroundColor = backgroundColorNormal
			}
		}







		// global vars
		var flipdown;
		var scheduleTime;
		var endTime;
		//
		var modeCountUp = false;
		var countUpEndSeconds = 300;
		//
		var headerText;
		var mainText;
		var secondaryText;
		var tertiaryText;
		var tertiaryTextSupportUs;
		var showTimer;
		var showTertiary;
		var audio = null;
		var backgroundColorNormal;
		var backgroundColorTransparent;
		var backgroundColorBlack;
		var backgroundColor = undefined;
		var globalError = "";
		var finishMessage = "STAND BY..";
		var centerLayout = false;
		
		// set default var values
		setDefaultVarValues();



// new, show visible when its ready
$(document).ready(function() {
  document.getElementsByTagName("html")[0].style.visibility = "visible";
});
