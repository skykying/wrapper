const electron = require('electron')
// Module to control application life.
const app = electron.app
// Module to create native browser window.
const BrowserWindow = electron.BrowserWindow

const path = require('path')
const url = require('url')

const Tray = electron.Tray
const Menu = electron.Menu
const globalShortcut  = electron.globalShortcut

let tray = null

let mainWindow = null

function showWindow () {
  mainWindow.show()
}

app.on('ready', () => {
  // Create the browser window.
  mainWindow = new BrowserWindow({width: 800, height: 600, show: false});

  // Hide on close
  mainWindow.on('close', (e) => {
    mainWindow.hide();
    e.preventDefault();
  })

  // and load the index.html of the app.
  mainWindow.loadURL(url.format({
    pathname: path.join(__dirname, 'index.html'),
    protocol: 'file:',
    slashes: true
  }))

  tray = new Tray(path.join(__dirname, 'trayicon.png'));
  const contextMenu = Menu.buildFromTemplate([
    {label: 'Show window', type: 'normal', click: () => { mainWindow.show(); }},
    {label: 'Quit', type: 'normal', click: () => { mainWindow.destroy(); app.quit(); }}
  ])
  tray.setToolTip('This is my application.')
  tray.setContextMenu(contextMenu)

  globalShortcut.register('CommandOrControl+Alt+U', () => {
    showWindow()
  })

  tray.on('click', () => {
    showWindow()
  });

  tray.on('double-click', () => {
    showWindow()
  });
});
