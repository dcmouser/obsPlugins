for %%a in ("%cd%") do set "CurDir=%%~na"
set thedir=%CurDir%

set zformat=zip

set extraoptions=-xr!*.bak

set zcmd="c:\program files\7-zip\7z" a -t%zformat% -r %extraoptions%
set zcmd1="c:\program files\7-zip\7z"
set zcmd2=a -t%zformat% -r %extraoptions%

set releasedir=..\..\release\rundir\RelWithDebInfo
set releasedirDlls=%releasedir%\obs-plugins\64bit


set pluginName=jrtimestamps
set fname=%pluginName%.%zformat%
set builddir=%pluginName%\release
mkdir "%builddir%"
mkdir "%builddir%\obs-plugins"
mkdir "%builddir%\obs-plugins\64bit"
mkdir "%builddir%\data"
mkdir "%builddir%\data\obs-plugins"
mkdir "%builddir%\data\obs-plugins\%pluginName%"
mkdir "%builddir%\data\obs-plugins\%pluginName%/locale"
mkdir "%builddir%\timestampOffset"
copy "%releasedirDlls%\%pluginName%.*" ".\%builddir%\obs-plugins\64bit\"
copy "%pluginName%\data\locale\*.*" ".\%builddir%\data\obs-plugins\%pluginName%\locale\"
copy "%pluginName%\timestampOffset\*.*" ".\%builddir%\timestampOffset\"
cd %builddir%
call %zcmd1% %zcmd2% "..\..\%fname%" "*.*"
cd ..\..


set pluginName=jrstats
set fname=%pluginName%.%zformat%
set builddir=%pluginName%\release
mkdir "%builddir%"
mkdir "%builddir%\obs-plugins"
mkdir "%builddir%\obs-plugins\64bit"
mkdir "%builddir%\data"
mkdir "%builddir%\data\obs-plugins"
mkdir "%builddir%\data\obs-plugins\%pluginName%"
mkdir "%builddir%\data\obs-plugins\%pluginName%\locale"
copy "%releasedirDlls%\%pluginName%.*" ".\%builddir%\obs-plugins\64bit\"
copy "%pluginName%\data\locale\*.*" ".\%builddir%\data\obs-plugins\%pluginName%\locale\"
cd %builddir%
call %zcmd1% %zcmd2% "..\..\%fname%" "*.*"
cd ..\..


set pluginName=obs-textml
set fname=%pluginName%.%zformat%
set builddir=%pluginName%\release
mkdir "%builddir%"
mkdir "%builddir%\obs-plugins"
mkdir "%builddir%\obs-plugins\64bit"
mkdir "%builddir%\data"
mkdir "%builddir%\data\obs-plugins"
mkdir "%builddir%\data\obs-plugins\%pluginName%"
mkdir "%builddir%\data\obs-plugins\%pluginName%/locale"
copy "%releasedirDlls%\%pluginName%.*" ".\%builddir%\obs-plugins\64bit\"
copy "%pluginName%\data\locale\*.*" ".\%builddir%\data\obs-plugins\%pluginName%\locale\"
cd %builddir%
call %zcmd1% %zcmd2% "..\..\%fname%" "*.*"
cd ..\..


set pluginName=ObsAutoZoom
set fname=%pluginName%.%zformat%
set builddir=%pluginName%\release
mkdir "%builddir%"
mkdir "%builddir%\obs-plugins"
mkdir "%builddir%\obs-plugins\64bit"
mkdir "%builddir%\data"
mkdir "%builddir%\data\obs-plugins"
mkdir "%builddir%\data\obs-plugins\%pluginName%"
mkdir "%builddir%\data\obs-plugins\%pluginName%/locale"
copy "%releasedirDlls%\%pluginName%.*" ".\%builddir%\obs-plugins\64bit\"
copy "%pluginName%\data\locale\*.*" ".\%builddir%\data\obs-plugins\%pluginName%\locale\"
cd %builddir%
call %zcmd1% %zcmd2% "..\..\%fname%" "*.*"
cd ..\..


set pluginName=transition-fadeDelayed
set fname=%pluginName%.%zformat%
set builddir=%pluginName%\release
mkdir "%builddir%"
mkdir "%builddir%\obs-plugins"
mkdir "%builddir%\obs-plugins\64bit"
mkdir "%builddir%\data"
mkdir "%builddir%\data\obs-plugins"
mkdir "%builddir%\data\obs-plugins\%pluginName%"
mkdir "%builddir%\data\obs-plugins\%pluginName%/locale"
copy "%releasedirDlls%\%pluginName%.*" ".\%builddir%\obs-plugins\64bit\"
copy "%pluginName%\data\locale\*.*" ".\%builddir%\data\obs-plugins\%pluginName%\locale\"
cd %builddir%
call %zcmd1% %zcmd2% "..\..\%fname%" "*.*"
cd ..\..

set pluginName=jrnotedock
set fname=%pluginName%.%zformat%
set builddir=%pluginName%\release
mkdir "%builddir%"
mkdir "%builddir%\obs-plugins"
mkdir "%builddir%\obs-plugins\64bit"
mkdir "%builddir%\data"
mkdir "%builddir%\data\obs-plugins"
mkdir "%builddir%\data\obs-plugins\%pluginName%"
mkdir "%builddir%\data\obs-plugins\%pluginName%/locale"
copy "%releasedirDlls%\%pluginName%.*" ".\%builddir%\obs-plugins\64bit\"
copy "%pluginName%\data\locale\*.*" ".\%builddir%\data\obs-plugins\%pluginName%\locale\"
cd %builddir%
call %zcmd1% %zcmd2% "..\..\%fname%" "*.*"
cd ..\..


set pluginName=jryoutubechat
set fname=%pluginName%.%zformat%
set builddir=%pluginName%\release
mkdir "%builddir%"
mkdir "%builddir%\obs-plugins"
mkdir "%builddir%\obs-plugins\64bit"
mkdir "%builddir%\data"
mkdir "%builddir%\data\obs-plugins"
mkdir "%builddir%\data\obs-plugins\%pluginName%"
mkdir "%builddir%\data\obs-plugins\%pluginName%/locale"
copy "%releasedirDlls%\%pluginName%.*" ".\%builddir%\obs-plugins\64bit\"
copy "%pluginName%\data\locale\*.*" ".\%builddir%\data\obs-plugins\%pluginName%\locale\"
cd %builddir%
call %zcmd1% %zcmd2% "..\..\%fname%" "*.*"
cd ..\..

set pluginName=jrcft
set fname=%pluginName%.%zformat%
set builddir=%pluginName%\release
mkdir "%builddir%"
mkdir "%builddir%\obs-plugins"
mkdir "%builddir%\obs-plugins\64bit"
mkdir "%builddir%\data"
mkdir "%builddir%\data\obs-plugins"
mkdir "%builddir%\data\obs-plugins\%pluginName%"
mkdir "%builddir%\data\obs-plugins\%pluginName%/locale"
copy "%releasedirDlls%\%pluginName%.*" ".\%builddir%\obs-plugins\64bit\"
copy "%pluginName%\data\locale\*.*" ".\%builddir%\data\obs-plugins\%pluginName%\locale\"
cd %builddir%
call %zcmd1% %zcmd2% "..\..\%fname%" "*.*"
cd ..\..


set pluginName=jrborder
set fname=%pluginName%.%zformat%
set builddir=%pluginName%\release
mkdir "%builddir%"
mkdir "%builddir%\obs-plugins"
mkdir "%builddir%\obs-plugins\64bit"
mkdir "%builddir%\data"
mkdir "%builddir%\data\obs-plugins"
mkdir "%builddir%\data\obs-plugins\%pluginName%"
mkdir "%builddir%\data\obs-plugins\%pluginName%/locale"
copy "%releasedirDlls%\%pluginName%.*" ".\%builddir%\obs-plugins\64bit\"
copy "%pluginName%\data\locale\*.*" ".\%builddir%\data\obs-plugins\%pluginName%\locale\"
cd %builddir%
call %zcmd1% %zcmd2% "..\..\%fname%" "*.*"
cd ..\..

set pluginName=jrborderSrc
set fname=%pluginName%.%zformat%
set builddir=%pluginName%\release
mkdir "%builddir%"
mkdir "%builddir%\obs-plugins"
mkdir "%builddir%\obs-plugins\64bit"
mkdir "%builddir%\data"
mkdir "%builddir%\data\obs-plugins"
mkdir "%builddir%\data\obs-plugins\%pluginName%"
mkdir "%builddir%\data\obs-plugins\%pluginName%/locale"
copy "%releasedirDlls%\%pluginName%.*" ".\%builddir%\obs-plugins\64bit\"
copy "%pluginName%\data\locale\*.*" ".\%builddir%\data\obs-plugins\%pluginName%\locale\"
cd %builddir%
call %zcmd1% %zcmd2% "..\..\%fname%" "*.*"
cd ..\..