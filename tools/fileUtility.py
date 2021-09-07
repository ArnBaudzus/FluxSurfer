#!/bin/python
# Utility for loading the files generated by the cpp code.

import xml.etree.ElementTree as ET
import numpy as np

def getRoot(pathToData):
	tree = ET.parse(pathToData)
	root = tree.getroot()
	return root

def getGraph(root):
	graphData = root.find("{GraphInfo}graphml")
	return graphData.find("{GraphInfo}graph")

def getKeyFrames(root):
	graphData = root.find("{GraphInfo}graphml")
	keys = graphData.findall('{GraphInfo}key')
	
	indexNodes = 0
	indexEdges = 0
	NodeKeys = [0]*(int(len(keys)/2)) #There are 2 keys for every time step.
	EdgeKeys = [0]*len(NodeKeys)

	for k in keys:
		if(k.attrib['attr.name'][0:4] == "Occu"):
			NodeKeys[indexNodes] = k.attrib['id']
			indexNodes+=1;
		else:
			EdgeKeys[indexEdges] = k.attrib['id']
			indexEdges+=1;


	timeValues = [0]*len(NodeKeys)

	for i in range(0,len(NodeKeys)):
		timeValues[i] = float(NodeKeys[i].split('_')[2]) 

	return np.array(timeValues),NodeKeys,EdgeKeys

#Test Code
#root = getRoot("../testSystemSaves/System no. 0")

#graph = getGraph(root)
#print(graph)

#times = getKeyFrames(root)
#print(times)
