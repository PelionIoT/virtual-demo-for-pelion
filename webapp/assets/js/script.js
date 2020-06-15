const fs = require('fs')

console.log("Loaded script.js");

// These are executed as soon as this file loads, before the HTML body is loaded.

//CHANGE value of FILE to location of file you write to
const file = '/home/george/repo/device-simulator-linux-client/__x86_x64_NativeLinux_mbedtls/Release/sensor_value.out'

const ConnectApi = require("mbed-cloud-sdk").ConnectApi;
const connect = new ConnectApi({
    autostartNotifications: true,
});

let observer; // To be initialised later.
let payload = ""
var previous_id = "";

function shakeButtonAnimation() {
    //animate button
    $("button").addClass("wobble animated infinite");
    setTimeout(function() {
        $("button").removeClass("wobble animated infinite");
    }, 5000)
}

function shakeScript() {
    //add your script here after shake triggered

}

function onLoad() {
    // Put the things you want to call when the HTML body loads.
    // This is generally setting-up stuff for later.
    console.log("Loaded HTML body")

    setButtonColor("green")

}

function setButtonColor(color) {
    //change color of buttonColor element in index.html(the button)
    var col = document.getElementById("buttonColor");
    if (color == "red")
        col.style.color = "#FF0000";
    else if (color == "green")
        col.style.color = "#65FF00"
    else if (color == "yellow")
        col.style.color = "#F0FF00"
    else if (color == "blue")
        col.style.color = "#005FFF"
    else if (color == "black")
        col.style.color = "#005FFF"
    else
        console.log("setButtonColor: Unrecognized color")
}

function selectChanged() {
    var selectIdElement = document.getElementById("selUser");
    var deviceId = selectIdElement.options[selectIdElement.selectedIndex].text;

    if (deviceId) {
        setObserver(deviceId);
        previous_id = deviceId
    } else {
        console.err("Selected a bad deviceId!: ", deviceId);
    }
}

fs.watch(file, (event, filename) => {
    if (filename && event === "change") {
        console.log("File changed")
        fs.readFile(file, 'utf-8', (err, data) => {
                if (err) throw err;

                // Converting Raw Buffer to text 
                // data using tostring function. 
                console.log(data);
                document.getElementById("number").innerHTML = data;
            })
            //document.getElementById("number").innerHTML = data;
    }
});
/*
fs.readFile('Input.txt', 'utf-8', (err, data) => { 
    if (err) throw err; 
  
    // Converting Raw Buffer to text 
    // data using tostring function. 
	console.log(data); 
	document.getElementById("number").innerHTML = data;
})*/

/*
function setObserver(deviceId) {
	// args: deviceId should be "*" or some deviceId like "001234567890123456789000"
	console.log("setObserver function");
	//Try to delete device subscriptions - not working for the moment
	connect.deletePresubscriptions()
	connect.deleteDeviceSubscriptions(previous_id);


	// Set the 'observer' variable to this new subscription to a given deviceId.
	observer = connect.subscribe.resourceValues({
		deviceId: deviceId,
	});

	// Add a listener that executes a function every time the observer returns a value
	observer.addListener(value => {
		console.log("Observer returned value: ", value.payload);
		payload = value.payload;
		
		//If the payload is non-integer - it has to be a color and call setButtonColor to change button color
		// Else put the number returned by the observer in the text box with the id "number" 
		if(isNaN(payload) == true)
			setButtonColor(payload)
		else
			document.getElementById("number").innerHTML = payload;
	});
}
*/

/*
	Notes:

		These things are equivalent:
	    	- document.getElementById("<id>") == $('#<id>')
        	- document.getElementsByClassName("<classname>") == $('.<classname>')

*/