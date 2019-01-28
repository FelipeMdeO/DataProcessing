import serial
from pyqtgraph.Qt import QtGui, QtCore
import numpy as np
import pyqtgraph as pg
from pyqtgraph.ptime import time

app = QtGui.QApplication([])

p = pg.plot()
p.setWindowTitle('live plot from serial')
p.setConfigOption('background', 'w')

curve = p.plot()

ser = serial.Serial('COM13', 38400, timeout = 1) # ttyACM1 for Arduino board

readOut = 0   #chars waiting from laser range finder

print ("Starting up")

acelx_list = []
velx_list = []

while True:
    while (ser.inWaiting()==0):
        pass #do nothing
    readOut = ser.readline().decode('ascii')     
    readOut = readOut.rstrip() # remocao do \n
    splitedData = readOut.split("\t")
    acelx_list.append ( float( splitedData[0] ) )
    velx_list.append ( float( splitedData[1] ) )
    print (readOut) 
    
    curve.setData(acelx_list)
    app.processEvents()

    ser.flush() #flush the buffer