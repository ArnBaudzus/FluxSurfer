#!/bin/python
#Takes a measurement file and calculates some quantities of the system based on
# the system file that is passed.

import numpy as np
from fileUtility import getRoot,getGraph,getKeyFrames
from binaryNumberUtility import numberOfParticles,occupiedLevels

def totalNumberOfParticles(pathToData):
	
	root = getRoot(pathToData)
	times,NodeKeys,EdgeKeys = getKeyFrames(root)
	graph = getGraph(root)

	nodes = graph.findall("{GraphInfo}node")

	numParticles = np.zeros(len(times))

	for node in nodes:
		
		particlesInState = numberOfParticles(int(node.attrib['id']))
		
		for i in range(0,len(times)):
			
			queue ='.//{GraphInfo}data[@key=\''+NodeKeys[i]+'\']'
			occupation = float(node.find(queue).text)
			numParticles[i] += (occupation * particlesInState)

	return times,numParticles

def occupationOfLevel(pathToData,Level):
	
	root = getRoot(pathToData)

	systemData = root.find("{SystemInfo}SystemInformation")
	numLevels = len(systemData.findall("{SystemInfo}niveau"))

	times,NodeKeys,EdgeKeys = getKeyFrames(root)
	graph = getGraph(root)

	nodes = graph.findall("{GraphInfo}node")

	numParticlesInLevel = np.zeros(len(times))
	
	for node in nodes:
		
		nodeId = int(node.attrib['id'])

		isOccupied = occupiedLevels(nodeId,numLevels)[Level]

		if not isOccupied:
			continue

		particlesInState = numberOfParticles(nodeId)
		
		for i in range(0,len(times)):
			
			queue ='.//{GraphInfo}data[@key=\''+NodeKeys[i]+'\']'
			occupation = float(node.find(queue).text)
			numParticlesInLevel[i] += (occupation * particlesInState)
	
	return numParticlesInLevel;

#Wheigths the data with a quantity of the node.
def weightedNumberOfParticles(pathToData,weightingQuantity):
	
	root = getRoot(pathToData)

	systemData = root.find("{SystemInfo}SystemInformation")
	levels = systemData.findall("{SystemInfo}niveau")
	
	weightVector = [0]*len(levels)
	
	for i in range(0,len(levels)):
		weightVector[i] = float(levels[i].find("{SystemInfo}"+weightingQuantity).text)

	times,NodeKeys,EdgeKeys = getKeyFrames(root)
	graph = getGraph(root)

	nodes = graph.findall("{GraphInfo}node")

	weightedQuantity = np.zeros(len(times))
	
	for node in nodes:
		
		nodeId = int(node.attrib['id'])
		occupation = occupiedLevels(nodeId,len(levels))
		
		weight = 0

		for i in range(0,len(occupation)):
			if(occupation[i]):
				weight += weightVector[i]
		
		particlesInState = numberOfParticles(nodeId)
		
		for i in range(0,len(times)):
			
			queue ='.//{GraphInfo}data[@key=\''+NodeKeys[i]+'\']'
			occupation = float(node.find(queue).text)
			weightedQuantity[i] += (occupation * weight)
	
	return weightedQuantity;

print(weightedNumberOfParticles("../TunnelExperimentSaves/000100_VBC=0.020000_T=1.000000.xml",
							  "Spin"))


