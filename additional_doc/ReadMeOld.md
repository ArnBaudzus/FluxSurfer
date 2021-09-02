# Montecarlo-Quantumdots Simulation

![Projectlogo](../../additional_doc/img/logo.jpg "Projectlogo")

## Introduction
This is a simulation of the tunneling dynamics of tunneling between Quantumdots
and a electron Source. The program strictly focusses on the simulation of the
tunneling behaviour. Physical phenomena like the calculation of the actual gate
voltage with e.g. the leverarm-approach or the current-voltage response of a
semiconductordevice with the dots need to be calculated in pre- or
post-processing procedures.<br>
What the program does do is the simulation of single quantumdots or whole
ensembles of quantumdots. 

## Basic Overview

The following graph shows the basic overview of the program. 


![Program architecture](../../additional_doc/img/program_architecture.jpg "Program architecture")

The core of the program is the simulation-layer. The physical calculations are
realized in the quantumdot-class and the experiment-class. The experiment is a
wrapper for one charge or discharge process. All Inputs are ether constant or
time dependent. The Experiment can simulate tunnelling in a whole ensemble of
quantumdots simultaniouslie. It can have multiple threads to parallelise the
simulation. Each thread handles a part of the simulated ensemble. <br>
The experiment also saves the optained data to a specified path and logs data on
the Progress of the simulation.<br>
The Hardware Management (HW-Management) Layer coordinates the Experiments. Let's have a look at
the following example: If one would like to simulate a conductance-spectroscopy
measurement where the gatevoltage is raised step by step for a fixed increment
and the charge of the Dots is observed until a steady state is reached. In this
program, every charge process after the incrementation of the voltage would be
realised as a single experiment. The Experimentcoordinator, which is the
central class on the HW-Management layer would create those experiments. It can
also run experiments parrallel if desired. The Experimentcoordinator also
creates the directories for saveing the date and generates the metadata of the
measurement. It also processes the logged data by the Experiments on the current
state and progress of the simulation to monitor the progress of the calculation
in real time. <br>
The main class of the program is the UI. The UI represents a GUI. The user
inputs data on the simulation here and configures the program. After physical
parameters and information on the desired thread-count, load-per-thread and
nature of the parallelisation (ether on the experiment level or with parallel
experiments or both) are entered, the UI creates a instance of the
experiment-coordinator which executes the simulation. The GUI than changes to a
monitor that shows the progress and estimated remaining simulationtime to the
user.

##State Graph Quantumdots

One of the key classes to gain maximum performance in the montecarlo-simulation
is the stategraph-quantumdot. This class represents a quantumdot, that can be
used in the Experiment class. In this particular implementation the states and
transitions between the states are represeted by a directed graph. All states of
the dot are nodes in the so called stategraph. All transitions are directed
edges in this graph. The weights of the edges are the probabilities for a
transition to occur. The following image shows the stategraph for a quantumdot
with one Niveau that is degenerated for spin-Up and spin-Down.<br>

![Stategraph example](../../additional_doc/img/example_Graph1.jpg "Stategraph example")

One useful sideeffect is that the adjacency-matrix of this graph is the
system-matrix for masterequation calculation. The other, more relevant fact for
the motecarloimplementation is, that the montecarlo-simulation of the dot can be
realised as a graph-exploration-algorithm. The main goal here is to keep the
number of functioncalls for functions like the fermifunction which require
evaluation of the exponential function as small as possible. This way one can
save recources. A example of how this algorithm could work on the 2-Layer 
quantumdot is presented:

![Graph exploration example](../../additional_doc/img/graph_exploration.jpg "Graph exploration example")

The Dot is initialized in the uncharged state (In the program the current state
is represented by a pointer) (1). The current state is indicated by the blue marker.
On initialisation the 2 Edges to the adjacent nodes are created, and the
probabilities are calculated. The adjacent nodes are created but not
initialisized. <br>
Eventually the dot transitions towards the state where a single
spin-up electron is inside the dot (2). After the Transition the node, representing
this state is initialized. This means in this example, that two more
probabilities are calculated.<br>
Note, that, if the system transitions back to the initial state (empty dot), no
probabilities need to be calculated (As long as Backcontact-voltage and
temperature do not change).<br>
Eventually the other states are occupied and the Graph is completely
initialized. In this state the simulation can continue without the need to
calculate new probabilities, keeping the number of evaluations of the
fermifunction as well as the densety of states and the energylevels as small as
possible.<br>
Another strength of the graph-exploration algorithm is that, for one
backcontact-voltage and one temperature only a view states of the dot are
occupied. With the graph exploration method only those probabilities and states
are evaluated, that are actually used, and no probability is evaluated twice.

##Notation: Identifying states

In the whole project a specific notation do denote quantumdot-states is used.
The notation assignes a unique number to every state.<br>
When a QuantumDot is created the Niveaus are specified by the user. The niveaus
are passed as an array. Thus, every Niveau can be identified with its position
in the array.<br>
To assign a number to a state a binary string is constructed. For every State
each niveau is evaluated. if the niveau is occupied in the given state, the
corresponding digit of the binary string 1. If the niveau is not occupied it is
0. the corresponding bit is the one that is at the same index as the position of
the Niveau in the Array. The decimal representation of this binarystring is the
indentification number of the state. Here are some examples: <br>
These are the given niveaus: [s1,s2,p1,p2,p3,p4]

	* The state where no Niveau is occupied would be denoted as binary 000000 or decimal 0
	* The state where every Niveau is occupied would be binary 111111 or decimal 63
	* The state where only Niveau p4 is occupied would be binary 000001 or decimal 1
	* The state where only Niveau s1 is occupied would be binary 100000 or decimal 32
	* The state where niveaus p2 and s1 are occupied would be binary 100100 or decimal 35

Obviously the numbers are only usefull if the position of the Niveaus in the
array is known and they can change if this position is altered. So it is
importand to keep track of this orientation in the metadata.
