#Generates Terminal Textmenus

def inputNumber(text):
	
	while(True):
		
		toReturn = input(text)

		try:
			toReturn = int(toReturn)
		except:
			print("Error. Input needs to be a Number.")
			print("Please try again.")
			continue

		return toReturn

def menu(title,items):
	
	choice = ''

	print(title)
	for i in range(0,len(items)):
		print(i,'): ',items[i][0])
	
	while(True):
		choice = input("Please enter one of the above numbers:")
		
		choiceValid = False

		for i in range(0,len(items)):
			if choice == str(i):
				items[i][1]()
				choiceValid = True
				break
	
		if choiceValid:
			break
		else:
			print("The Input Need to match one of the Numbers above.")
			print("Please try again.")

def executeOnRecource(text,toExecute):

	while(True):
		pathToRecource = input(text+'\n')
		
		try:
			retval = toExecute(pathToRecource)
			return retval
		except:
			print("Oops. Seems like the action you tried to make doesn't work on the input path.")
			print("Please try again.")
			
