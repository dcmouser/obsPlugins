for %%a in ("%cd%") do set "CurDir=%%~na"
set thedir=%CurDir%

set mydate=%date:~12,2%%date:~4,2%%date:~7,2%
set TIMEZERO=%TIME: =0%
set HOUR=%TIMEZERO:~0,2%
set MIN=%TIMEZERO:~3,2%
set mytime=%HOUR%%MIN%
set zformat=zip

set extraoptions=-xr!Library/Bee -xr!Library/Android -xr!Library/PackageCache -xr!Library/il2cpp_android_arm64-v8a  -xr!Library/Artifacts -xr!Library/Il2cppBuildCache -xr!Library/ShaderCache

set zcmd="c:\program files\7-zip\7z" a -t%zformat% -r %extraoptions%
set fname=%thedir%_%mydate%_%mytime%.%zformat%

echo %zcmd% ..\%fname% .\
%zcmd% ..\%fname% .\
