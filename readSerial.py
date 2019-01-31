import serial
from pyqtgraph.Qt import QtGui, QtCore
import numpy as np
import pyqtgraph as pg
from pyqtgraph.ptime import time

app = QtGui.QApplication([])

win = pg.GraphicsWindow(title="Plot iterativo")
win.resize(1200,900)
win.setWindowTitle("Motion Graphics from Accelerometer Data")

# Enable antialiasing for prettier plots
pg.setConfigOptions(antialias=True)

p1 = win.addPlot(title = "Acceleration data")
p1.enableAutoRange('xy', True) 
#p1.setAutoPan(y=True)
#p1.setConfigOption('background', 'w')

#win.nextRow()

# Grafico de velocidade filtrada a partir de media movel
#p2 = win.addPlot(title = "Velocity filtered data")
#p2.enableAutoRange('xy', True) 
#p2.setAutoPan(y=True)
#p2.setConfigOption('background', 'w')

#win.nextRow()

# Grafico de velocidade Bruta
#p3 = win.addPlot(title = "Velocity data")
#p3.enableAutoRange('xy', True) 

#win.nextRow()

# Grafico de Posicao a partir da integral das velocidades filtradas
#p4 = win.addPlot(title = "Position data")
#p4.enableAutoRange('xy', True) 

splitedData = [0,0,0,0]
splitedData[0] = float(splitedData[0])
splitedData[1] = float(splitedData[1])
splitedData[2] = float(splitedData[2])
splitedData[3] = float(splitedData[3])

aux = -1
aux2 = -1
aux3 = -1

def updatePlot():
    p1.setXRange(len(acelx_list)-200, len(acelx_list), padding=0)
    #p2.setXRange(len(velx_filtered_list)-200, len(velx_filtered_list), padding=0)    
    #p3.setXRange(len(velx_list)-200, len(velx_list), padding=0)    
    #p4.setXRange(len(pos_list)-200, len(pos_list), padding=0)    

acelx_list = []
velx_filtered_list = []
velx_list = []
pos_list = []

curve = p1.plot(pen='y', fillLevel=aux*(splitedData[0]+splitedData[0]*0.25), brush=(50,50,200,100))
p1.setLabel('left', "Accel", units='g')
#curve2 = p2.plot(pen='r', fillLevel=aux2*(splitedData[1]+splitedData[1]*0.25), brush=(50,50,200,100))
#p2.setLabel('left', "Vel mean", units='m/s')
##curve3 = p3.plot(pen='g', fillLevel=aux3*(splitedData[2]+splitedData[2]*0.25), brush=(50,50,200,100))
#p3.setLabel('left', "Vel", units='m/s')
#curve4 = p4.plot(pen='w', fillLevel=aux3*(splitedData[3]+splitedData[3]*0.25), brush=(50,50,200,100))
#p4.setLabel('left', "Pos", units='m')

#p1.setYRange(-0.1,0.1, padding = 0)
#p2.setYRange(-0.5, 0.5, padding=0)    
##p3.setYRange(-0.5, 0.5, padding=0)    
#p4.setYRange(-0.35, 0.35, padding=0)    

ser = serial.Serial('COM15', 38400, timeout = 1) # ttyACM1 for Arduino board

print ("Starting up")

readOut = 0   

while True:
    while (ser.inWaiting()==0):
        pass #do nothing
    readOut = ser.readline().decode('ascii')     
    #print(readOut)
    readOut = readOut.rstrip() # remocao do \n
    splitedData = readOut.split("\t")
   
    acelx_list.append ( float( splitedData[0] )/1000 )
   # velx_filtered_list.append( float( splitedData[1] ) )
    #velx_list.append ( float( splitedData[2] ) )
    #pos_list.append(float(splitedData[3]))
    
    #print (len(acelx_list))

    curve.setData(acelx_list)
    #curve2.setData(velx_filtered_list)
    #curve3.setData(velx_list)
    #curve4.setData(pos_list)

    app.processEvents()
    updatePlot()

    ser.flush() #flush the buffer
