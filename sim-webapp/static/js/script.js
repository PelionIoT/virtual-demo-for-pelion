var wss;

var ON_CSS  = {'font-size': '30px', 'color': 'rgb(0,255,0)'};
var OFF_CSS = {'font-size': '30px', 'color': 'rgb(0,0,0)'};

var SMALL_CONSOLE_TEXT_CSS= {'margin': '0px','color': '#bad8fc','font-family': 'Electrolize', 'font-size': '26px'};
var LARGE_CONSOLE_TEXT_CSS= {'margin': '0px','color': '#bad8fc','font-family': 'Electrolize', 'font-size': '38px'};

var led = false;

function onLoad() {
    var url = "ws://" + location.host + "/comsock";
    wss = new WebSocket(url);

    wss.onmessage = (message) => {
        var response = JSON.parse(message.data);
        switch (response.cmd) {
            case "getID":
                device_id = response.data.substr(response.data.length - 7);
                $("#deviceID").text(device_id);
                break;
            case "getFW":
                device_id = response.data;
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
                        }, 5000)                
                    }    
                } else { // for all others, just display msg
                    $("#console").text(response.data);
                }
                break;
            default:
                console.log("received unknown paylod: " + message.data);
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
    }

    wss.onerror = (error) => {
        alert(`[error] ${error.message}`);
    };
}

function toggleShake() {
    // toggle button animation
    $("#shakeButtonId").toggleClass("animate__wobble");
    setTimeout(() => $("#shakeButtonId").toggleClass("animate__wobble"), 5000);

    //instruct server and revert after 20s
    var cmd = { cmd: "shake", enable: true };
    wss.send(JSON.stringify(cmd));
    setTimeout(() => {
        var cmd = { cmd: "shake", enable: false };
        wss.send(JSON.stringify(cmd));
    }, 20000)

}

