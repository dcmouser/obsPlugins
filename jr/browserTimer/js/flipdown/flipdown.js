"use strict";

function _typeof(obj) { if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

var FlipDown = function () {
  function FlipDown(uts) {
    var el = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : "flipdown";
    var opt = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : {};

    _classCallCheck(this, FlipDown);

    if (typeof uts !== "number") {
      throw new Error("FlipDown: Constructor expected unix timestamp, got ".concat(_typeof(uts), " instead."));
    }

    if (_typeof(el) === "object") {
      opt = el;
      el = "flipdown";
    }

    this.version = "0.3.2jr2";
    this.initialised = false;
    this.now = this._getTime();

		if (uts == 0) {
			this.epoch = this.now;
		} else {
	    this.epoch = uts;
	  }
	  
    this.countdownEnded = false;
    this.hasEndedCallback = null;
    this.element = document.getElementById(el);
    this.rotors = [];
    this.rotorLeafFront = [];
    this.rotorLeafRear = [];
    this.rotorTops = [];
    this.rotorBottoms = [];
    this.countdown = null;
    this.daysRemaining = 0;
    this.clockValues = {};
    this.clockStrings = {};
    this.clockValuesAsString = [];
    this.prevClockValuesAsString = [];
    this.opts = this._parseOptions(opt);
    this.firstCountUpDisplay = true;
    
    // ATTN: 1/20/22
    this.modeCountUp = opt.modeCountUp;
    this.countUpEndSeconds = opt.countUpEndSeconds;
    
    // ATTN: 1/5/21
    this.rotorGroups = [];

    this._setOptions();

    //console.log("FlipDown ".concat(this.version, " (Theme: ").concat(this.opts.theme, ")"));
  }

  _createClass(FlipDown, [{
    key: "start",
    value: function start() {
    	var intervalTickSpeed = 1000;
    	// speed it up because it sometimes missed turning..
    	intervalTickSpeed = 500;
      if (!this.initialised) this._init();
      this.countdown = setInterval(this._tick.bind(this), intervalTickSpeed);
      return this;
    }
  }, {
    key: "ifEnded",
    value: function ifEnded(cb) {
      this.hasEndedCallback = function () {
        cb();
        this.hasEndedCallback = null;
      };

      return this;
    }
  }, {
    key: "_getTime",
    value: function _getTime() {
      return new Date().getTime() / 1000;
    }
  }, {
    key: "_hasCountdownEnded",
    value: function _hasCountdownEnded() {
    	
			var timerEndCondition = false;
			// count up mode?
			if (this.modeCountUp) {
				// end timer after some amount of time?
				if (this.countUpEndSeconds && ((this.now - this.epoch) > this.countUpEndSeconds)) {
					timerEndCondition = true;
				}
	    } else {
	    	if (this.epoch - this.now < 0) {
	    		timerEndCondition = true;
	    	}
	    }
        

      if (timerEndCondition) {
				if (!this.countdownEnded) {
					// ATTN: cleanup

					// ATTN: stop timer?
					clearInterval(this.countdown);
					
					// make something vis
					var doneEl = document.getElementById("flipdown_done");
					if (doneEl) {
						doneEl.style.display = 'block';
					}
					// hide timer
					if (true) {
						var el = this.element;
						if (el) {
							el.style.display = 'none';
						}
					}
					// callback to main global -- better way to do this?
					globalCallbackTimerEnds();
				}
		  
        this.countdownEnded = true;

        if (this.hasEndedCallback != null) {
          this.hasEndedCallback();
          this.hasEndedCallback = null;
        }

        return true;
      } else {
        this.countdownEnded = false;
        return false;
      }
    }





  }, {
    key: "_parseOptions",
    value: function _parseOptions(opt) {
      var headings = ["Days", "Hours", "Minutes", "Seconds"];

      if (opt.headings && opt.headings.length === 4) {
        headings = opt.headings;
      }

      return {
        theme: opt.hasOwnProperty("theme") ? opt.theme : "dark",
        headings: headings
      };
    }
  }, {
    key: "_setOptions",
    value: function _setOptions() {
      this.element.classList.add("flipdown__theme-".concat(this.opts.theme));
    }
  }, {
    key: "_init",
    value: function _init() {
      this.initialised = true;

			// ATTN: hide the done message?
			var doneEl = document.getElementById("flipdown_done");
			if (doneEl) {
				doneEl.style.display = 'none';
			}


      if (this._hasCountdownEnded()) {
        this.daysremaining = 0;
      } else {
      	
      	// ATTN: 1/20/22 - allow count up
      	//var diff = this.epoch - this.now <= 0 ? 0 : this.epoch - this.now;
      	var diff = this.epoch - this.now <= 0 ? (this.now - this.epoch) : this.epoch - this.now;
        this.daysremaining = Math.floor((diff) / 86400).toString().length;
      }

      var dayRotorCount = this.daysremaining <= 2 ? 2 : this.daysremaining;

      for (var i = 0; i < dayRotorCount + 6; i++) {
        this.rotors.push(this._createRotor(0));
      }

      var dayRotors = [];

      for (var i = 0; i < dayRotorCount; i++) {
        dayRotors.push(this.rotors[i]);
      }

			var rg = this._createRotorGroup(dayRotors, 0);
			this.rotorGroups.push(rg);

      this.element.appendChild(rg);
      var count = dayRotorCount;

      for (var i = 0; i < 3; i++) {
        var otherRotors = [];

        for (var j = 0; j < 2; j++) {
          otherRotors.push(this.rotors[count]);
          count++;
        }

				rg = this._createRotorGroup(otherRotors, i + 1);
				this.rotorGroups.push(rg);
        this.element.appendChild(rg);
      }

      this.rotorLeafFront = Array.prototype.slice.call(this.element.getElementsByClassName("rotor-leaf-front"));
      this.rotorLeafRear = Array.prototype.slice.call(this.element.getElementsByClassName("rotor-leaf-rear"));
      this.rotorTop = Array.prototype.slice.call(this.element.getElementsByClassName("rotor-top"));
      this.rotorBottom = Array.prototype.slice.call(this.element.getElementsByClassName("rotor-bottom"));

      this._tick();

      this._updateClockValues(true);

      return this;
    }
  }, {
    key: "_createRotorGroup",
    value: function _createRotorGroup(rotors, rotorIndex) {
      var rotorGroup = document.createElement("div");
      rotorGroup.className = "rotor-group";
      var dayRotorGroupHeading = document.createElement("div");
      dayRotorGroupHeading.className = "rotor-group-heading";
      dayRotorGroupHeading.setAttribute("data-before", this.opts.headings[rotorIndex]);
      rotorGroup.appendChild(dayRotorGroupHeading);
      appendChildren(rotorGroup, rotors);
      return rotorGroup;
    }
  }, {
    key: "_createRotor",
    value: function _createRotor() {
      var v = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 0;
      var rotor = document.createElement("div");
      var rotorLeaf = document.createElement("div");
      var rotorLeafRear = document.createElement("figure");
      var rotorLeafFront = document.createElement("figure");
      var rotorTop = document.createElement("div");
      var rotorBottom = document.createElement("div");
      rotor.className = "rotor";
      rotorLeaf.className = "rotor-leaf";
      rotorLeafRear.className = "rotor-leaf-rear";
      rotorLeafFront.className = "rotor-leaf-front";
      rotorTop.className = "rotor-top";
      rotorBottom.className = "rotor-bottom";
      rotorLeafRear.textContent = v;
      rotorTop.textContent = v;
      rotorBottom.textContent = v;
      appendChildren(rotor, [rotorLeaf, rotorTop, rotorBottom]);
      appendChildren(rotorLeaf, [rotorLeafRear, rotorLeafFront]);
      return rotor;
    }
  }, {
    key: "_tick",
    value: function _tick() {
      this.now = this._getTime();
      
      // ATTN: 1/20/22 - allow count up
      //var diff = this.epoch - this.now <= 0 ? 0 : this.epoch - this.now;
      var diff = this.epoch - this.now <= 0 ? (this.now - this.epoch) : this.epoch - this.now;

      this.clockValues.d = Math.floor(diff / 86400);
      diff -= this.clockValues.d * 86400;
      this.clockValues.h = Math.floor(diff / 3600);
      diff -= this.clockValues.h * 3600;
      this.clockValues.m = Math.floor(diff / 60);
      diff -= this.clockValues.m * 60;
      this.clockValues.s = Math.floor(diff);

      this._updateClockValues();

      this._hasCountdownEnded();
    }
  }, {
    key: "_updateClockValues",
    value: function _updateClockValues() {
      var _this = this;

      var init = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : false;
      this.clockStrings.d = pad(this.clockValues.d, 2);
      this.clockStrings.h = pad(this.clockValues.h, 2);
      this.clockStrings.m = pad(this.clockValues.m, 2);
      this.clockStrings.s = pad(this.clockValues.s, 2);
      this.clockValuesAsString = (this.clockStrings.d + this.clockStrings.h + this.clockStrings.m + this.clockStrings.s).split("");
      this.rotorLeafFront.forEach(function (el, i) {
        el.textContent = _this.prevClockValuesAsString[i];
      });
      this.rotorBottom.forEach(function (el, i) {
        el.textContent = _this.prevClockValuesAsString[i];
      });

      function rotorTopFlip() {
        var _this2 = this;

        this.rotorTop.forEach(function (el, i) {
          if (el.textContent != _this2.clockValuesAsString[i]) {
            el.textContent = _this2.clockValuesAsString[i];
          }
        });
      }

			function setElHidden(el, isHidden) {
				// hidden doesnt want to work setting it to false so we have to do it this way
				if (isHidden) {
					el.style.display = "none";
					//el.setAttribute("hidden", "true");
				} else {
					el.style.display = "block";
					//el.setAttribute("hidden", "false");
				}
			}


      function rotorLeafRearFlip() {
        var _this3 = this;

        this.rotorLeafRear.forEach(function (el, i) {
          if (el.textContent != _this3.clockValuesAsString[i]) {
            el.textContent = _this3.clockValuesAsString[i];
            el.parentElement.classList.add("flipped");
            var flip = setInterval(function () {
              el.parentElement.classList.remove("flipped");
              clearInterval(flip);
            }.bind(_this3), 500);
          }
        });
      }

      if (!init) {
        setTimeout(rotorTopFlip.bind(this), 500);
        setTimeout(rotorLeafRearFlip.bind(this), 500);
      } else {
        rotorTopFlip.call(this);
        rotorLeafRearFlip.call(this);
      }

      this.prevClockValuesAsString = this.clockValuesAsString;
      
      
      // ATTN: mouser@donationcoder.com - 1/5/21 - hide 0 leftmost values
			if (this.epoch - this.now <= 0) {
				// counting up we need to be more intensive here to turn them back on
				var el;
				el = this.rotorGroups[0];
				if (this.clockStrings.d > 0 || this.firstCountUpDisplay) {
					setElHidden(el, this.clockStrings.d == 0);
				}
				el = this.rotorGroups[1];
				if (this.clockStrings.h > 0 || this.firstCountUpDisplay) {
					setElHidden(el, this.clockStrings.h == 0);
				}
				el = this.rotorGroups[2];
				if (this.clockStrings.m > 0 || this.firstCountUpDisplay) {
					setElHidden(el, this.clockStrings.m == 0);
				}
				el = this.rotorGroups[3];
				if (this.clockStrings.s > 0 || this.firstCountUpDisplay) {
					setElHidden(el, this.clockStrings.s == 0);
				}
				this.firstCountUpDisplay = false;
			} else {
				if (this.clockStrings.d == 0) {
					var el = this.rotorGroups[0];
					setElHidden(el, true);
					if (this.clockStrings.h == 0) {
						var el = this.rotorGroups[1];
						setElHidden(el, true);
						if (this.clockStrings.m == 0) {
							var el = this.rotorGroups[2];
							setElHidden(el, true);
							if (this.clockStrings.s == 0) {
								var el = this.rotorGroups[3];
								setElHidden(el, true);
							}
						}
					}
				}
      }
 
    }
  }]);

  return FlipDown;
}();

function pad(n, len) {
  n = n.toString();
  return n.length < len ? pad("0" + n, len) : n;
}

function appendChildren(parent, children) {
  children.forEach(function (el) {
    parent.appendChild(el);
  });
}
