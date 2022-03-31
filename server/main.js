const { app, globalShortcut, BrowserWindow } = require("electron");

let win;
function createWindow() {
  win = new BrowserWindow({
    autoHideMenuBar: true,
    width: 640,
    height: 480,
    icon: "favicon.ico",
    webPreferences: {
      nodeIntegration: true,
      enableRemoteModule: true,
      contextIsolation: false,
    },
  });

  win.loadFile("index.html");
  //win.webContents.openDevTools();
}

app
  .whenReady()
  .then(() => {
    globalShortcut.register("CommandOrControl+L", () => {
        win.webContents.openDevTools();
    });
  })
  .then(createWindow);

app.on("activate", () => {
  if (BrowserWindow.getAllWindows().length === 0) {
    createWindow();
  }
});

app.on("window-all-closed", () => {
  if (process.platform !== "darwin") {
    app.quit();
  }
});
