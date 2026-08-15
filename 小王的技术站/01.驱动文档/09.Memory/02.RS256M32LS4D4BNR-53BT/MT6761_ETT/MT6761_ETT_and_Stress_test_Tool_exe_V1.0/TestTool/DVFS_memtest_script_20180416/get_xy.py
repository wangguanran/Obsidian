#import monkeyrunner modules and connect device
from com.android.monkeyrunner import MonkeyRunner, MonkeyDevice
device = MonkeyRunner.waitForConnection()

import os
import sys
import shutil

print '***********************************************************'
print 'Please place the phone on the table. '
print 'Remember to unlock the screen before running this script'
print 'Stress test is going to start, please do not move the phone.'
print '************************************************************'

if(len(sys.argv) > 1):
	run_time=int(sys.argv[1])
else:
	run_time=45
width = int(device.getProperty('display.width'))
height = int(device.getProperty('display.height'))
times=0
x = ((185*width)/480)
y = ((170*height)/800)
print "width:"+str(width)+",x:"+str(x)
print "height:"+str(height)+",y:"+str(y)
# Get current path
path = os.path.dirname(os.path.abspath(__file__))
#print "path:"+str(path)

# input the content to the file
f = open(str(path)+'/start', 'w')
#if(f!=NULL)
f.write('type= user \n')
f.write('count= 1\n')
f.write('speed= 1.0\n')
f.write('start data >>\n')
f.write('UserWait(3000)\n')
f.write('DispatchPointer(0, 0, 0, '+str(x)+', '+str(y)+', 1.100, 0.0, 0, 0.0, 0.0, 0, 0);\n')
f.write('DispatchPointer(0, 0, 1, '+str(x)+', '+str(y)+', 1.100, 0.0, 0, 0.0, 0.0, 0, 0);\n')
f.close()
