function onLoad() {
    var url = "ws://" + location.host + "/comsock";
    var wss = new WebSocket(url);

    wss.onmessage = function (message) {
        console.log(message);

        var response = JSON.parse(message.data);

        switch (response.cmd) {
            case "getID":
                data = response.data.substr(response.data.length - 7)
                $("#deviceID").text(data);
                break;
            case "observe":
                data = response.data;
                $("#number").text(data);
                break;
            default:
                console.log("err: unknown message");
        }
    }

    wss.onopen = function (event) {
        // onload, we need to retrieve device ID
        var cmd = {
            cmd: "getID"
        };

        wss.send(JSON.stringify(cmd));
    }
}

