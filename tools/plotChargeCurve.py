#!/bin/python

import matplotlib.pyplot as plt
import matplotlib
matplotlib.rcParams['text.usetex'] = True
import sys

from menu import menu,executeOnRecource
from quantities import totalNumberOfParticles


#Loading the Data:

times = []
particleCount = []
names = []

def load():
	time,pCount = executeOnRecource("Which file should be loadet?",totalNumberOfParticles)
	times.append(time)
	particleCount.append(pCount)
	names.append(input("Please enter the Name of the loaded dataset."))

def doNothing():
	pass;

def loadFiles():
	load()
	menu("Do you want to enter another dataset?",
			[["Yes",loadFiles],["No",doNothing]])


if(len(sys.argv) == 1):
	loadFiles()
else:
	for i in range(1,len(sys.argv) - 1):
		if i%2 == 0:
			names.append(sys.argv[i])
		else:
			time,pCount = totalNumberOfParticles(sys.argv[i])
			times.append(time)
			particleCount.append(pCount)

#Plotting

fig,ax = plt.subplots()

for i in range(0,len(times)):
	
	ax.plot(times[i],particleCount[i],label = names[i])

ax.tick_params(direction='in',top=True,bottom=True,left=True,right=True);
ax.set_xlabel("Time in s")
ax.set_ylabel("Carriers in System")
if(len(sys.argv)==1):
	plt.title(input("Please enter a Title for the Plot"))
else:
	plt.title(sys.argv[-1])

plt.legend()
	
plt.show()
