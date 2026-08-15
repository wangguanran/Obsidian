echo === install NenaMark ===
adb install -r %CD%\benchmark_apk\NenaMark2.apk
ping 127.0.0.1 -n 5 -w 1000 > nul
adb install -r %CD%\benchmark_apk\NenaMark2.apk
echo === gen start for x,y to press run ===
monkeyrunner.bat %CD%\get_xy.py

