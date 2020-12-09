var wss;

function onLoad() {
    var url = "ws://" + location.host + "/comsock";
    wss = new WebSocket(url);

    wss.onmessage = (message) => {
        var response = JSON.parse(message.data);
        switch (response.cmd) {
            case "getID":
                data = response.data.substr(response.data.length - 7);
                $("#deviceID").text(data);
                break;
            case "observe":
                data = response.data;
                $("#number").text(data);
                break;
            default:
                console.log(response.data);
        }
    }

    // onload, we need to retrieve device ID
    wss.onopen = (event) => {
        var cmd = { cmd: "getID" };
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

    //instruct server and revert after 5s
    var cmd = { cmd: "shake", enable: true };
    wss.send(JSON.stringify(cmd));
    setTimeout(() => {
        var cmd = { cmd: "shake", enable: false };
        wss.send(JSON.stringify(cmd));
    }, 10000)

}

