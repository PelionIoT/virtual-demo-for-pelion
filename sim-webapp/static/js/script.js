var wss;

var ON_CSS  = {'font-size': '30px', 'color': 'rgb(0,255,0)'};
var OFF_CSS = {'font-size': '30px', 'color': 'rgb(0,0,0)'};

var SMALL_CONSOLE_TEXT_CSS= {'margin': '0px','color': '#bad8fc','font-family': 'Electrolize', 'font-size': '26px'};
var LARGE_CONSOLE_TEXT_CSS= {'margin': '0px','color': '#bad8fc','font-family': 'Electrolize', 'font-size': '38px'};

var led = false;
var sensor_type = "vibration"; // default

function onLoad() {
    var url = (location.protocol==="https:"? "wss://": "ws://") + location.host + "/comsock";
    wss = new WebSocket(url);

    wss.onmessage = (message) => {
        var response = JSON.parse(message.data);
        switch (response.cmd) {
            case "getID":
                device_id = response.data.substr(0, 15);
                $("#deviceID").text(device_id);
                break;
            case "getFW":
                device_id = response.data;
                if (device_id == "0.0.0") { // if we don't have a version number in the 14/ resource then show 0.0.1 instead of default 0.0.0
                    device_id = "0.0.1"
                }
                $("#firmware").text(device_id);
                break;                
            case "observe":
                resource_val = response.data;
                $("#console").css(LARGE_CONSOLE_TEXT_CSS);
                $("#console").text(resource_val);
                break;
            case "blink":
                $("#led").css( (led? OFF_CSS: ON_CSS) );
                led = !led;
                break;
            case "fota":
                $("#console").css(SMALL_CONSOLE_TEXT_CSS);
                if (response.params === "fota_app_on_download_progress") { // if progress msg
                    $("#console").text("Downloading " + response.data + "%");

                    if (response.data === "100") { // did we reach 100%
                        // start a timeout to refresh page after a 
                        // wait period to allow the backend to reboot.
                        setTimeout(() => {
                            $("#console").text("Rebooting...");
                            location.reload();
                        }, 5000);                
                    }    
                } else { // for all others, just display msg
                    $("#console").text(response.data);
                }
                break;
            case "sensorType":
                sensor_type = response.data;
                if (sensor_type === "counter" || sensor_type === "temperature") {
                    $("#shakeButtonId").text("+1");
                } else if (sensor_type === "vibration") {
                    $("#shakeButtonId").text("SHAKE");
                }
                // else the default "BUTTON" label will be used from the static index.html file
                
                break;
            default:
                console.log("received unknown payload: " + message.data);
        }
    }

    // onload, we need to retrieve device ID and FW version
    wss.onopen = (event) => {
        // retrieve device id
        var cmd = { cmd: "getID" };
        wss.send(JSON.stringify(cmd));

        // retrieve firmware version
        var cmd = { cmd: "getFW" };
        wss.send(JSON.stringify(cmd));

        // retrieve sensor type
        var cmd = { cmd: "getSensorType" };
        wss.send(JSON.stringify(cmd));        
    }

    wss.onerror = (error) => {
        alert(`[error] ${error.message}`);
    };
}

function toggleShake() {
    if (sensor_type === "vibration") // vibration
    {
        // toggle button animation
        $("#shakeButtonId").toggleClass("animate__wobble");
        setTimeout(() => $("#shakeButtonId").toggleClass("animate__wobble"), 20000);   

        //instruct server that shake is enabled and revert after 20000ms
        var cmd = { cmd: "shake", enable: true };
        wss.send(JSON.stringify(cmd));
        setTimeout(() => {
            var cmd = { cmd: "shake", enable: false };
            wss.send(JSON.stringify(cmd));
        }, 20000);
    }
    else
    {
        //instruct server that button is down and revert after 100ms
        var cmd = { cmd: "shake", enable: true };
        wss.send(JSON.stringify(cmd));
        setTimeout(() => {
            var cmd = { cmd: "shake", enable: false };
            wss.send(JSON.stringify(cmd));
        }, 100);        
    }
}

