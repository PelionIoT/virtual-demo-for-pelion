

console.log("select22 here");
var PelionDMSDK = require("mbed-cloud-sdk");

var devices = new PelionDMSDK.DeviceDirectoryApi({
    apiKey: "ak_1MDE2MDExYWE5N2VlYmUyMDZhZTVlY2Y2MDAwMDAwMDA01714fcc9c0bee18ba36734b00000000OG4LuersbPoTDtPFwRFgFUlHZzXTHEqJ"
});

devices.listDevices({
    filter: {
        state: { $eq: "registered" },
        createdAt: { $gte: new Date("01-01-2014"), $lte: new Date("01-01-2021") },
        updatedAt: { $gte: new Date("01-01-2014"), $lte: new Date("01-01-2021") }
    }
}, function (error, devices) {
    var i = 0;
    //l = total number of devices in Pelion
    var l = devices.data.length


    var data = {
        id: 1,
        text: 'NewID'
    };
    for (i = 0; i < l; i++) {
        console.log(devices.data[i].id)
        data.id = 1 + i;

        //troncate the first and last chars of the Pelion device ID 
        var len = devices.data[i].id.length;
        //pelion_id_last_chars = devices.data[i].id.substring(0, 4) + "..." + devices.data[i].id.substring(len - 6, len)
        pelion_id_last_chars = devices.data[i].id;
        data.text = pelion_id_last_chars;
        console.log(pelion_id_last_chars);
        var newOption = new Option(data.text, data.id, false, false);
        //Append the troncated Pelion IDs to the drop-down list
        $('#selUser').append(newOption);

    }

    if (error) throw error;
    // Utilize devices here
});


//console.log($('#selUser').select2('data'));
console.log("select22 end");