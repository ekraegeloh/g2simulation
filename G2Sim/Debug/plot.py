#! /usr/bin/python3

import numpy as np
import sys
import matplotlib.pyplot as plt
plt.style.use('mastercolor')

#atm know what to plot, later show all individual plots and then decide
title = sys.argv[1]
timestep = float(sys.argv[2])
datasets = sys.argv[3:]

datarray = []
for d in datasets:
	array = np.loadtxt(d)
	datarray.append(array)
datarray = np.array(datarray)

time = np.linspace(0, len(datarray[0])*timestep, len(datarray[0]), False)

for d in range(len(datasets)):
	plt.plot(time, datarray[d])#labels!!
plt.xlabel('t [s]')
plt.title(title)
plt.savefig(title+'.png')
