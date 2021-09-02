#!/usr/bin/python

import sys;
import matplotlib.pyplot as plt;
import numpy as np;
import json;

if len(sys.argv) == 1:
	print("""You need to Specify at least one Path to a Json-File containing Data
	you want to view.""");
	exit();

fig,axs = plt.subplots(1,len(sys.argv)-1)


def fill_Subplot(ax,path):
	with open(path) as f:
		data = json.load(f)
	
	x = np.arange(data["NumberOfStates"])
	y = np.array(data["Time"])
	X,Y = np.meshgrid(x,y)

	Z = np.array(data["Occupation"]);

	im = ax.pcolor(X,Y,Z,vmin = 0,vmax =1,shading='nearest',cmap="rainbow")
	
	ax.set_xticks(x);
	ax.set_xticklabels(x);
	

	ax.set_title(path)	
	ax.set_xlabel("State Number")
	ax.set_ylabel("Time in ms")
	ax.tick_params(direction='in',top=True,bottom=True,left=True,right=True);
	
	return im;	

if len(sys.argv) == 2:
	img=fill_Subplot(axs,sys.argv[1])
else:
	for i in range(1,len(sys.argv)):
		img=fill_Subplot(axs[i-1],sys.argv[i])

fig.colorbar(img);

fig.tight_layout();
plt.show();
