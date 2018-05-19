@echo off
echo @echo off > "C:\Users\%USERNAME%\AppData\Local\Temp\Uninstall.bat"
echo rmdir /S /Q "C:\Users\%USERNAME%\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Doom\" >> "C:\Users\%USERNAME%\AppData\Local\Temp\Uninstall.bat"
echo rmdir /S /Q "C:\Users\%USERNAME%\Documents\My Games\Doom\" >> "C:\Users\%USERNAME%\AppData\Local\Temp\Uninstall.bat"
echo del "C:\Users\%USERNAME%\AppData\Local\Temp\Uninstall.bat" >> "C:\Users\%USERNAME%\AppData\Local\Temp\Uninstall.bat"
::ETC
start "New Window" cmd /c "C:\Users\%USERNAME%\AppData\Local\Temp\Uninstall.bat"
