
if EXIST Debug rmdir /S /Q Debug
if EXIST Release rmdir /S /Q Release
if EXIST tlsh.VC*.ncb del tlsh.VC*.ncb 
if EXIST tlsh.VC*.suo del tlsh.VC*.suo 

if EXIST tlsh_dll\Debug rmdir /S /Q tlsh_dll\Debug
if EXIST tlsh_dll\Release rmdir /S /Q tlsh_dll\Release
if EXIST tlsh_dll\tlsh.VC*.vcproj.TREND* del tlsh_dll\tlsh.VC*.vcproj.TREND*

if EXIST tlsh_unittest\Debug rmdir /S /Q tlsh_unittest\Debug
if EXIST tlsh_unittest\Release rmdir /S /Q tlsh_unittest\Release
if EXIST tlsh_unittest\tlsh_unittest.VC*.vcproj.TREND* del tlsh_unittest\tlsh_unittest.VC*.vcproj.TREND*

if EXIST tlsh_version\Debug rmdir /S /Q tlsh_version\Debug
if EXIST tlsh_version\Release rmdir /S /Q tlsh_version\Release
if EXIST tlsh_version\tlsh_version.VC*.vcproj.TREND* del tlsh_version\tlsh_version.VC*.vcproj.TREND*

if EXIST simple_unittest\Debug rmdir /S /Q simple_unittest\Debug
if EXIST simple_unittest\Release rmdir /S /Q simple_unittest\Release
if EXIST simple_unittest\simple_unittest.vcproj.TREND* del simple_unittest\simple_unittest.vcproj.TREND*

if EXIST ..\bin\*.dll del ..\bin\*.dll
if EXIST ..\bin\*.exe del ..\bin\*.exe
