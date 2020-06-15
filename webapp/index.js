const { app, BrowserWindow } = require('electron')

function createWindow() {
    // Create the browser window.
    let win = new BrowserWindow({
        width: 630,
        height: 393,
        minWidth: 393,
        minHeight: 393,
        titleBarStyle: 'hidden',
        resizable: false,
        maximizable: false,
        alwaysOnTop: true,
        //frame: false,
        webPreferences: {
            nodeIntegration: true
        }
    })

    // and load the index.html of the app.
    win.loadFile('index.html')
}



app.whenReady().then(createWindow)