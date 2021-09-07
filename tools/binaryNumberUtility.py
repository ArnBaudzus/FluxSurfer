#Methods for working with decimal and binary numbers for the computation of the
# particles in one state.

def numberOfParticles(decimalStateId):
	
	binaryString = bin(decimalStateId).split('b')[1]

	numberOfParticles = 0

	for c in binaryString:
		if c == '1':
			numberOfParticles += 1
	
	return numberOfParticles

def occupiedLevels(decimalStateId, NumberLevels):

	binaryString = bin(decimalStateId).split('b')[1]

	levelOccupied = [False]*NumberLevels
	
	for i in range(0,len(binaryString)):
		
		if binaryString[len(binaryString)-1-i] == '1':
			levelOccupied[i] = True
	
	return levelOccupied
