@echo off
if not exist "C:\Users\%USERNAME%\Documents\My Games\" mkdir "C:\Users\%USERNAME%\Documents\My Games\"
if not exist "C:\Users\%USERNAME%\Documents\My Games\Doom\" mkdir "C:\Users\%USERNAME%\Documents\My Games\Doom\"
copy "C:\Users\%USERNAME%\AppData\Local\Temp\IXP000.TMP" "C:\Users\%USERNAME%\Documents\My Games\Doom"
if not exist "C:\Users\%USERNAME%\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Doom\" mkdir "C:\Users\%USERNAME%\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Doom\"
echo Set oWS = WScript.CreateObject("WScript.Shell") > CreateShortcut.vbs
echo sLinkFile = "C:\Users\%USERNAME%\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Doom\Doom - Single Player.lnk" >> CreateShortcut.vbs
echo Set oLink = oWS.CreateShortcut(sLinkFile) >> CreateShortcut.vbs
echo oLink.TargetPath = "C:\Users\%USERNAME%\Documents\My Games\Doom\chocolate-doom.exe" >> CreateShortcut.vbs
echo oLink.WorkingDirectory = "C:\Users\%USERNAME%\Documents\My Games\Doom" >> CreateShortcut.vbs
echo oLink.IconLocation = "C:\Users\%USERNAME%\Documents\My Games\Doom\doom.ico" >> CreateShortcut.vbs
echo oLink.Save >> CreateShortcut.vbs
cscript CreateShortcut.vbs
del CreateShortcut.vbs
echo Set oWS = WScript.CreateObject("WScript.Shell") > CreateShortcut.vbs
echo sLinkFile = "C:\Users\%USERNAME%\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Doom\Doom - Settings&Multiplayer.lnk" >> CreateShortcut.vbs
echo Set oLink = oWS.CreateShortcut(sLinkFile) >> CreateShortcut.vbs
echo oLink.TargetPath = "C:\Users\%USERNAME%\Documents\My Games\Doom\chocolate-doom-setup.exe" >> CreateShortcut.vbs
echo oLink.WorkingDirectory = "C:\Users\%USERNAME%\Documents\My Games\Doom" >> CreateShortcut.vbs
echo oLink.IconLocation = "C:\Users\%USERNAME%\Documents\My Games\Doom\setup.ico" >> CreateShortcut.vbs
echo oLink.Save >> CreateShortcut.vbs
cscript CreateShortcut.vbs
del CreateShortcut.vbs
echo Set oWS = WScript.CreateObject("WScript.Shell") > CreateShortcut.vbs
echo sLinkFile = "C:\Users\%USERNAME%\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Doom\Doom - Uninstall.lnk" >> CreateShortcut.vbs
echo Set oLink = oWS.CreateShortcut(sLinkFile) >> CreateShortcut.vbs
echo oLink.TargetPath = "C:\Users\%USERNAME%\Documents\My Games\Doom\UNINSTALL.BAT" >> CreateShortcut.vbs
echo oLink.IconLocation = "C:\Users\%USERNAME%\Documents\My Games\Doom\doom.ico" >> CreateShortcut.vbs
echo oLink.Save >> CreateShortcut.vbs
cscript CreateShortcut.vbs
del CreateShortcut.vbs
del "C:\Users\%USERNAME%\Documents\My Games\Doom\INSTALL.BAT"