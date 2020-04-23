@echo off
SET _DEBUG_FLAGS=/Zi /Od /W2
SET _FLAGS=/nologo /std:c++14 /FC
SET _INC=
REM The lib directory is changed if you don't run the 64 bit one.
SET _LINK_FLAGS=/SUBSYSTEM:console
SET _LIBDIR=
SET _LIB=
SET _NAME=out.exe

SET _PATH=""
SET _RUN=""
SET _ARCH=""

FOR %%A IN (%*) DO (
    IF "%%A"=="/r"  ( SET _RUN=YES )
    IF "%%A"=="/p"  ( SET _PATH=YES )
	IF "%%A"=="/32" ( SET _ARCH=YES )
)

IF %_PATH% == YES (
	IF %_ARCH% == YES (
		call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" > null
		REM call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"
		SET _LIBDIR=../lib/win32
	) ELSE (
		call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 > null
		REM call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
	)
)

if not exist bin mkdir bin
REM PUSHD bin
REM PUSHD %_LIBDIR%
REM COPY /Y *.dll "../../bin" >nul 2>nul
REM POPD
ECHO  ====== COMPILE ====== 
cl %_DEBUG_FLAGS% %_FLAGS% ../src/windows_main.cpp /Fe%_NAME% /I%_INC% "/link /LIBPATH:%_LIBDIR% %_LIB%"

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

