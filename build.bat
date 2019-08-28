@echo off
SET _RUN=""
SET _PATH=""
SET _ARCH=""
SET _RELEASE=NO

FOR %%A IN (%*) DO (
    IF "%%A"=="/r"  ( SET _RUN=YES )
    IF "%%A"=="/p"  ( SET _PATH=YES )
	IF "%%A"=="/32" ( SET _ARCH=YES )
	IF "%%A"=="/RELEASE" ( SET _RELEASE=YES )
)


SET _DEBUG_FLAGS=/Zi /Od /W2
SET _RELEASE_FLAGS=/O2
IF %_RELEASE% == YES (
	SET _FLAGS=%_RELEASE_FLAGS% /nologo /std:c++17 /FC
) ELSE (
	SET _FLAGS=%_DEBUG_FLAGS% /nologo /std:c++17 /FC
)

SET _INC=../inc
REM The lib directory is changed if you don't run the 64 bit one.
SET _LINK_FLAGS=/SUBSYSTEM:windows
SET _LIBDIR=..\lib\win64
SET _LIB=SDL2.lib SDL2main.lib
SET _NAME=fog.exe


IF %_PATH% == YES (
	IF %_ARCH% == YES (
		call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" > nul
		REM call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"
		SET _LIBDIR=../lib/win32
	) ELSE (
		call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 > nul
		REM call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
	)
)

if not exist bin mkdir bin
PUSHD bin
PUSHD %_LIBDIR%
COPY /Y *.dll "../../bin" >nul 2>nul
POPD
ECHO  ====== COMPILE ====== 
cl %_FLAGS% ../src/windows_main.cpp /Fe%_NAME% /I%_INC% "/link /LIBPATH:%_LIBDIR% %_LIB%"

@if not %ErrorLevel% == 0 (
	ECHO ====== FAILED ======
	goto _END
)

if %_RUN% == YES (
	ECHO  ====== RUNNING ======
	cd ..
	start bin/%_NAME%
	cd bin
)

:_END
POPD

