@echo off
setlocal EnableDelayedExpansion
set deviceid=-s 0123456789ABCDEF
set memtester=/data/memtester
set chk_status=/data/memtest_check_status.sh
set test_rank=no
set test_num=6
set /a test_sz=64*1024*1024
set debug_script=0
set logprefix=/data/rshmoo_log_

:start
echo "wait for device.."
adb %deviceid% wait-for-device && adb %deviceid% root
adb %deviceid% remount

:: sleep 10 sec
:: echo "wait 10 sec for remount.."
:: ping 1.1.1.1 -n 1 -w 10000 > nul
:: adb %deviceid% remount

:pretest

FOR /L %%i IN (1, 1, %test_num%) DO call :clean_log 0 %%i

call :get_status st
IF NOT "%st%" == "initial" (
:: This should not happened, dont know how to handle it..
  echo "[ERROR] status != initial before invoke memtest.."
  adb %deviceid% reboot
  goto :start
)

echo "push memtester"
call :detect_3264 arch
adb %deviceid% push memtester%arch% %memtester%
adb %deviceid% shell "chmod 777 %memtester%"
adb %deviceid% push memtest_check_status.sh %chk_status%
adb %deviceid% shell "chmod 777 %chk_status%"

adb %deviceid% shell "mkdir /data/memtest_logs"


:: get rank and mem size
call :get_mem_size total_sz
call :get_each_size %total_sz% %test_num% sz
call :toSize %sz% szhex
echo "total " %total_sz%
echo "each "%sz%

:echo "=== run DVFS ==="
:start cmd /c adb %deviceid% shell "sh /data/vcorefs_cervino.sh &"

:echo "=== run nenamark ==="
:start cmd /c adb %deviceid% shell "sh /data/run.sh &"

echo "=== invoke memtester ==="
start cmd /c adb %deviceid% shell "%memtester% -n 32M 2>> /data/memtest_logs/rshmoo_log_v_1"
start cmd /c adb %deviceid% shell "%memtester% -n 32M 2>> /data/memtest_logs/rshmoo_log_v_2"
start cmd /c adb %deviceid% shell "%memtester% -n 32M 2>> /data/memtest_logs/rshmoo_log_v_3"
start cmd /c adb %deviceid% shell "%memtester% -n 32M 2>> /data/memtest_logs/rshmoo_log_v_4"
start cmd /c adb %deviceid% shell "%memtester% -n 32M 2>> /data/memtest_logs/rshmoo_log_v_5"
start cmd /c adb %deviceid% shell "%memtester% -n 32M 2>> /data/memtest_logs/rshmoo_log_v_6"

:testing
:: pulling
adb %deviceid% shell "sh %chk_status% 6"

:END
pause
goto :eof














:: function definition start

:detect_3264
::
::
SETLOCAL ENABLEDELAYEDEXPANSION
set a3264=
set r=
FOR /f "tokens=1 USEBACKQ" %%i IN (`adb %deviceid% shell "uname -m"`) DO (
set r=%%i
)
IF NOT "%r%" == "aarch64" (
  set a3264=32
) ELSE (
  set a3264=64
)
(
ENDLOCAL & REM RETURN VALUES
set %~1=%a3264%
)
goto :eof

:get_mem_size
::
::
SETLOCAL ENABLEDELAYEDEXPANSION
set r=
IF "%test_rank%" == "no" (
  ENDLOCAL & REM RETURN VALUES
  set /a %~1=%test_sz%*%test_num%
  goto :eof
)
FOR /f "tokens=2 USEBACKQ" %%i IN (`adb %deviceid% shell "cat /d/memtest/mem%rank%"`) DO (
set r=%%i
)
( ENDLOCAL & REM RETURN VALUES
  set /a %~1=%r%
)
goto :eof

:get_each_size
::
::
SETLOCAL ENABLEDELAYEDEXPANSION
set total=%~1
set num=%~2
set /a r=%total%/%num%
set /a m=r%%4096
IF NOT "%m%" == "0" (
  set /a r=r-m
)
( ENDLOCAL & REM RETURN VALUES
  set /a %~3=%r%
)
goto :eof

:get_dev_status
::
::
SETLOCAL ENABLEDELAYEDEXPANSION
set r=
FOR /f "tokens=1 USEBACKQ" %%i IN (`adb %deviceid% get-state`) DO (
set r=%%i
)
IF "%r%" == "device" (
  set /a r=1
) ELSE (
  set /a r=0
)
( ENDLOCAL & REM RETURN VALUES
  set %~1=%r%
)
goto :eof

:get_status
::
::
SETLOCAL ENABLEDELAYEDEXPANSION
call :get_dev_status dev
IF NOT "%dev%" == "1" (
  echo "adb offline.."
  set r=fail
  ) ELSE (
  set r=
  FOR /f "tokens=1 USEBACKQ" %%i IN (`adb %deviceid% shell "cat /d/memtest/result"`) DO (
  set r=%%i
  )
)
( ENDLOCAL & REM RETURN VALUES
  set %~1=%r%
)
goto :eof

:clean_log
::
::
set /a testid=%~1 + %~2
set logname=%logprefix%%testid%
set debug=

IF "%debug_script%" == "1" (
  set debug=echo
)

%debug% adb %deviceid% shell "rm -f %logname%"
goto :eof

:runtest
::
::
set /a testid=%~2 + %~3
set /a adrid=%~3 - 1
set logname=%logprefix%%testid%
call :toPhyBase 0x0 %adrid% %~1 adr
set debug=

IF "%debug_script%" == "1" (
  set debug=echo
)

IF NOT "%test_rank%" == "no" (
  %debug% start cmd /c adb %deviceid% shell "%memtester% -m /d/memtest/mem%test_rank% -p %adr% %~1 2>> %logname%"
) ELSE (
  %debug% start cmd /c adb %deviceid% shell "%memtester% %~1 2>> %logname%"
)
goto :eof

:toSize
::
::
SETLOCAL ENABLEDELAYEDEXPANSION
set sz=%~1
set /a g=sz%%(1024*1024*1024)
set /a m=sz%%(1024*1024)
set r=

IF "%g%" == "0" (
  set /a s=sz/(1024*1024*1024)
  set r=%s%G
) ELSE IF "%m%" == "0" (
  set /a s=sz/(1024*1024)
  set r=%s%M
) ELSE (
  set /a s=sz/1024
  set r=%s%K
)

( ENDLOCAL & REM RETURN VALUES
    SET %~2=%r%
)
goto :eof

:toPhyBase
::
::
SETLOCAL ENABLEDELAYEDEXPANSION
set b=%~1
call :toDec %b:~2% base
set s=%~3
set flag=%s:~-1%
set num=%s:~0,-1%
set /a size=0
IF "%flag%" == "G" (set /a size=1024*1024*1024)
IF "%flag%" == "g" (set /a size=1024*1024*1024)
IF "%flag%" == "M" (set /a size=1024*1024)
IF "%flag%" == "m" (set /a size=1024*1024)
IF "%flag%" == "K" (set /a size=1024)
IF "%flag%" == "k" (set /a size=1024)
IF "%size%" == "0" (set /a size=1024*1024)

set /a addr=(%base%*1024*1024)+(%~2*%num%*%size%)

call :toHex %addr% hex
( ENDLOCAL & REM RETURN VALUES
    IF "%~4" NEQ "" (SET %~4=0x%hex%) ELSE ECHO.0x%hex%
)
goto :eof

:genError
::
::
SETLOCAL ENABLEDELAYEDEXPANSION

adb %deviceid% shell "echo 4:0 > /proc/aed/generate-wdt"

goto :END
goto :eof

:toHex dec hex -- convert a decimal number to hexadecimal, i.e. -20 to FFFFFFEC or 26 to 0000001A
::             -- dec [in]      - decimal number to convert
::             -- hex [out,opt] - variable to store the converted hexadecimal number in
::Thanks to 'dbenham' dostips forum users who inspired to improve this function
:$created 20091203 :$changed 20110330 :$categories Arithmetic,Encoding
:$source http://www.dostips.com
SETLOCAL ENABLEDELAYEDEXPANSION
set /a dec=%~1
set "hex="
set "map=0123456789ABCDEF"
FOR /L %%N IN (1,1,8) DO (
    set /a "d=dec&15,dec>>=4"
    FOR %%D IN (!d!) DO set "hex=!map:~%%D,1!!hex!"
)
rem !!!! REMOVE LEADING ZEROS by activating the next line, e.g. will return 1A instead of 0000001A
rem FOR /f "tokens=* delims=0" %%A IN ("%hex%") DO set "hex=%%A"&if not defined hex set "hex=0"
( ENDLOCAL & REM RETURN VALUES
    IF "%~2" NEQ "" (SET %~2=%hex%) ELSE ECHO.%hex%
)
goto :eof

:toDec hex dec -- convert a hexadecimal number to decimal
::             -- hex [in]      - hexadecimal number to convert
::             -- dec [out,opt] - variable to store the converted decimal number in
:$created 20091203 :$changed 20091203 :$categories Arithmetic,Encoding
:$source http://www.dostips.com
SETLOCAL
set /a dec=0x%~1
( ENDLOCAL & REM RETURN VALUES
    IF "%~2" NEQ "" (SET %~2=%dec%) ELSE ECHO.%dec%
)
goto :eof