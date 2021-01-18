var wss;

var ON  = {'font-size': '30px', 'color': 'rgb(0,255,0)'};
var OFF = {'font-size': '30px', 'color': 'rgb(0,0,0)'};
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
                $("#console").text(resource_val);
                break;
            case "blink":
                $("#led").css( (led? OFF: ON) );
                led = !led;
                break;
            case "progress":
                $("#console").text("Downloading " + response.data + "%");
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

