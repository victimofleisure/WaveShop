@echo off
navgen template.html .
if errorlevel 1 goto err
"C:\Chris\MyProjects\FixSelfUrl\Release\FixSelfUrl" *.html
if errorlevel 1 goto err
md Help
C:\Chris\MyProjects\doc2web\release\doc2web /nospaces C:\Chris\MyProjects\WaveShop\Help\help.txt Help Contents.htm C:\Chris\MyProjects\WaveShop\doc\WaveShopHelp.htm "WaveShop Help"
if errorlevel 1 goto err
cd Help
md images
copy C:\Chris\MyProjects\WaveShop\Help\images\*.* images
copy ..\helptopic.css content.css
navgen C:\Chris\MyProjects\WaveShop\Help\template.txt .
copy ..\helpheader.txt x
copy x + Contents.htm
echo ^<body^>^<html^> >>x
del Contents.htm
ren x Contents.htm
md printable
cd printable
move C:\Chris\MyProjects\WaveShop\doc\WaveShopHelp.htm .
cd ..
cd ..
ren issues.html issues.htm
echo y | fsr issues.htm "<div id=body>" "<div id=widebody>"
ren issues.htm issues.html
goto exit
:err
pause Error!
:exit
