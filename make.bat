@echo "Rerunning qmake ..." 
qmake -r
@echo "Running make ..."
mingw32-make
@echo "Assembling binary package in cacheme.win32 ..."
rmdir /Q /S cacheme.win32
md cacheme.win32
copy cacheme.exe cacheme.win32
copy lang\cacheme_*.qm cacheme.win32
md cacheme.win32\data\icons
copy data\icons\*.svg cacheme.win32\data\icons
copy *.ico cacheme.win32
md cacheme.win32\plugins\geoservices
copy plugins\geoservices\*.dll cacheme.win32\plugins\geoservices
md cacheme.win32\plugins\cacheprovider
copy plugins\cacheprovider\*.dll cacheme.win32\plugins\cacheprovider
copy C:\QtMobility\lib\QtLocation1.dll cacheme.win32
copy C:\QtMobility\lib\QtSensors1.dll cacheme.win32
copy C:\Qt\2010.05\mingw\bin\mingwm10.dll cacheme.win32
copy C:\Qt\2010.05\mingw\bin\libgcc_s_dw2-1.dll cacheme.win32
copy C:\Qt\2010.05\Qt\bin\QtCore4.dll cacheme.win32
copy C:\Qt\2010.05\Qt\bin\QtGui4.dll cacheme.win32
copy C:\Qt\2010.05\Qt\bin\QtNetwork4.dll cacheme.win32
copy C:\Qt\2010.05\Qt\bin\QtSvg4.dll cacheme.win32
@echo "CacheMe prepared in cacheme.win32"
