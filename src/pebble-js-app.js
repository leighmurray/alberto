var configKeys = ["label", "timeStart", "timeEnd", "progress", "format"];
var eventUrl = 'http://percentage.leighmurray.com/get-event.php';

Pebble.addEventListener("ready",
  function(e) {
    getCurrentEvent();
  }
);

Pebble.addEventListener("showConfiguration",
  function(e) {
	var url = 'http://percentage.leighmurray.com/settings.htm';
	//url += GetJSONConfig();
	url = encodeURI(url);
	console.log(url);
	Pebble.openURL(url);
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    getCurrentEvent();
  }                     
);

Pebble.addEventListener("webviewclosed",
  function(e) {
    getCurrentEvent();
    //console.log("Configuration window returned: " + e.response);
    if (!e.response) {
      return;
    }

    var configuration = JSON.parse(e.response);
    console.log(configuration);
    for (var i = 0; i < configuration.time.length; i++) {
      var returnedObject = configuration.time[i];
      
      for(var k in returnedObject) {
        SetConfig(k, returnedObject[k]);
      }
    }

  }
);

function getCurrentEvent () {
  xhrRequest(eventUrl, 'GET', 
    function(responseText) {
      var json = JSON.parse(responseText);
      
      if (json.success !== undefined) {
        if (json.success === false) {
          console.log ("cant get event, you has to log in.");
        } else if (json.success === true) {
          console.log ("no future events yo.");
        }
      } else {
        console.log('Title:' + json.t0);
        console.log('Start:' + json.s0);
        console.log('End:' + json.e0);
        sendCurrentEvents(json);
      }
    }
  );
}

function sendCurrentEvents (jsonObject) {
  
  var title1 = jsonObject.t0;
  var startDate1 = jsonObject.s0;
  var endDate1 = jsonObject.e0;
  var title2 = jsonObject.t1;
  var startDate2 = jsonObject.s1;
  var endDate2 = jsonObject.e1;
  
  var dictionary = {
    "KEY_TITLE_1": title1,
    "KEY_START_TIME_1": startDate1,
    "KEY_END_TIME_1": endDate1,
    "KEY_TITLE_2": title2,
    "KEY_START_TIME_2": startDate2,
    "KEY_END_TIME_2": endDate2,
  };
  
  Pebble.sendAppMessage(dictionary,
  function(e) {
    console.log("Event info sent to Pebble successfully!");
  },
  function(e) {
    console.log("Error sending event info to Pebble!");
  }
);
}

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function GetConfig (key) {
  return window.localStorage.getItem(key);
}

function SetConfig (key, value) {
	console.log("Setting Key:" + key + " - Value:" + value);
	window.localStorage.setItem(key, value);
}

function GetJSONConfig () {

	var tempConfig = {};
	for (var i=0; i<configKeys.length; i++) {
		var configKey = configKeys[i];
		tempConfig[configKey] = GetConfig(configKey);
	}
	var jsonConfig = JSON.stringify(tempConfig);
	return jsonConfig;
}