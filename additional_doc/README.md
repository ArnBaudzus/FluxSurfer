# Simulation of Finite-State-Quantumsystems

![Projectlogo](../../additional_doc/img/logo.jpg "Projectlogo")

## Introduction

This is a library that can be used as a backend for simulations of
Quantumsystems with a finite number of energy states. The Library includes
multiple analytical and numerical solvers for ODEs. Everything is implemented in
a way that should make it easy to setup simulations, run code with
parallelisation and save the result in a human and machine readable fashion.

## Interpretation of a Quantumsystem as Graph

A Graph is a number of nodes that are connected by edges. A quantumsystem can be
interpreted as a graph. The States of the System are the nodes in the graph. The
Occupation of a State at a given time can be stored in the nodes. The
edges in the graph represent the transition processes that can occur. The
edgewheights represent the rate at which these processes occur. The
following image shows an example of a system with one energy niveau that can
hold two electrons, one spin up, the other spin down.

![Stategraph example](../../additional_doc/img/example_Graph1.jpg "Stategraph example")

The System is charge coupled with a electron reservoir. The edges, connecting
the states represent tunneling from electorns in the niveau of the system or out
of the niveau. It is important to notice the difference between a state of the
System and the Niveaus that the system has. In many sources these terms are used
somewhat loosely.<br>
The Whole Physics of the system is encoded in the edges of the graph. The
Probabilities depend on the tunneling barrier, the density of states in the
reservoir and the Occupation.<br>
This second example highlights the role of the edges. In addition to the
tunnel-coupling spinflip processes are taken into account.

![Stategraph example](../../additional_doc/img/example_Graph2.jpg "Stategraph example")

Notice the two additional edges that are now present. <br>
Using graphs, we can create a intuitive way to break up the large problem of
simulating a system with multiple physical processes like (like e.g. tunneling
and spinflip) in to smaller problems: Finding functions that calculate the edge
weights.<br>
Another advantage is, that, once the edgeweights are known, the process of
getting a time-evolution of the occupation of states doesn't differ from system
to system. The Process of solving a system with known transition rates and given
initial occupation is implemented in this library. This way, the mathematical
problem of getting a solution is out of the way and one can focus on the
physics- Getting the weights of the edges.

## Library Structure

### Dependencys

This Library is coded in C++ and uses some content defined in the C++17
standart. It uses Armadillo as a backend for linear algebra calculations. It
also uses jsoncpp to export json files. Under Linux, compile with the following
command: 

	g++ your_main_class_here.cpp -std=cpp17 -O3 -larmadillo -lpthread -ljsoncpp -I experiment_layer -o your_desired_output_path.out

### The Quantum System

The QuantumSystem is the central class of the library. It holds a graph that
represents a QuantumSystem. It stores the occupation data and the weights of the
edges during the simulation and can save this data as graphml file. The big
advantage of saveing as graphml is, that the information of the physics involved
in the simulation is stored together with the desired solution; the occupation
data. All measured quantities can be optained with postprocessing.<br><p>

The Experiment class is the central interface to manage parallelprocessing and
the storage of metadata. The terminology works as follows: We think of the
simulation of one Quantumsystem as of the simulation of one timeevolution during
a experiment. Often experiments consist of multiple time-evolutions processes.
Like e.g. in conductance or capacitance spectorscopy, where the system is
charged and discharged multiple times.<br>
The Experiment represents a set of Measurements and the corresponding metadata.
The Experiment class also coordinates which quantumsystem is simulated with
which solver, and how many processes are running in parallel.<br><p>

Solver : The Solvers in this library can -form a structural standpoint- roughly
be divided in two classes. One class are solvers that work on the Masterequation
matrix of the system (Which is very similar to the adjacencymatrix of the
graph). In this class are the analytic solutions and the ODE Solvers.<br> The
other Class consists of Algorithms that work on the graph itself. At this time,
the only solver that does this is the montecarlo wanderer algorithm.<br><p>

To use the Experiment class and the Quantumsystem the user has to derive a
concrete implementation. (See tutorials)

## Structure of saved content

The Saved content looks as follows:<br><p>

![Saved_Content](../../additional_doc/img/folder.png "Saved Content")

The user has to specify a folder for the storage of the data. After the
simulation, the folder should contain two metadata files: METADATA.json and
METADATA.csv. In addition to that, there should be a number of .graphml files.
Most of the management is done automatically, if the interface for the
Experiment is implemented correctly.<br><p>

###METADATA.json
This file should be used for data for humans. Things in natural language, like
descriptions, motivations and expectations. The physical quantities that are
constant during the experiment are also stored here.

###METADATA.csv
In every experement there are quantiteis that are varied from measurement to
measurement (but are constant for one measurement). This could be f.e. the
varying biasvoltage in CV-Spectroscopy or Conductancespectroscopy. This file contains a table. The filename of the measurements and
the values of the changeing physical quantities are noted here.

###\*.graphml
The Data generated during the Simulation is saved as GraphML file. This File
contains the Nodes, Their time dependent occupation and the edges and their
timedependent wheights. This Way, not only the occupation but also the structure
of the system is saved. The great advantage of this format is, that the Physics,
that was taken into account during the simulation is also saved with the data.
The disadvantage is that one will have to extract measurement data one would see
in a experiment from this file via postporcessing. It is recomendet to use a
Software like Gephi or a script-language like python in conjunction with
networkX to extract the data. After the extraction postprocessing can be done in
every ones software of choice.<p>
This file also contains a small description of the system and its energyniveaus
written in xml.

##Parallelisation

A Experiment consists of multiple measurements, i.e. multiple timeevolutions of
the system under test. Since the parameters for the experiment are known
apriori, one can parallelise the simulation of the measurements. The
parallelisation in the project is schematically illustrated below:

![Parallelisation](../../additional_doc/img/Parallelisation.jpg "Parallelisation")

There are multiple threads running in parallel. All of which, except one are
worker threads. The coordination of those threads is done by using one boolean
variable and a queue in shared memory. There are Methods in the global scope to
manage access on the memory.<br>
Pending jobs are stored in the queue (WorkSource). The Workerthreads will wait, until there
are jobs in the queue. A job is a QuantumSystem and a Solver. Once there are
jobs, The WorkerThread will process them as follows: The QuantumSystem is solved
using the provided Solver. The Data is than saved to the FileSystem and Solver
and QuantumSystem are deleted, since they where stored as dynamic memory. When
there is no job in the Queue the Workerthread will continue to wait on new jobs.
The Waiting eventually ends, when the boolean, namend Running, is turend false
by the management thread. All workerthreads will than stop running and can be
joined with the management thread.<br>
The Management thread will observe the Status of the WorkSource Queue. If there
are to few jobs in the queue, the management thread will generate new ones after
a specification in the Experiment Class. New Jobs are allocated in dynamic
memory.<br>
Since (roughly spoken) only the jobs that are processed are stored in dynamic
memory, the whole process works somewhat inplace.

##Solvers and Solutions

As mentioned above (Library Structure) the Solvers in this project can be
roughly divided in two classes. One being the Algorithms that work on the
Masterequation of the system which shows great similarities with the adjacency
matrix of the Graph. The other class is working directly on the Graph.<br>
All solvers share a common parent class: Solver. This Class defines the things
that every solver should implement to be compatible with the library. If you are
interested in the exact way the other solvers are derived from Solver, take a
look at the inheritance diagram in its class file.

###Master-Equation-Approach

The Masterequation is a generalized rate-equation. It is a first order ODE.
However, there are a view cases where it is possible to obtain an analytical
solution to the Masterequation. Examples are Master equations with a constant
matrix and special cases such as master equations with periodic coeffitients,
where a solution can be optained via fourier transformation.

####Analytical Solution

Up to the current moment no analytic solution is implemented but there are plans
on implementing the solution for masterequation with constant coeffitients in
the near future.

####Numerical Solution using ODE-Solvers

In this library, the solvers are separated from the numerical stepschemes used.
There is an abstract class called SingleStepScheme that defines how a numerical
single step scheme should look like. You can create a sup-class of it if there
is a scheme you are missing. All solvers accept schemes that behave like the
SingleStepScheme class. This way you can use every solver with every scheme. Up
to now there are only two solvers. One is a ODE-Solver with fixed stepwidth. The
other is a adaptive stepwidth solver, which uses a method based on
richardson-extrapolation to guess a good stepwidht (i.e. it solves the ODE width
two stepwidhts and calculates a estimated value for the error and the ideal
stepwidth).

###Monte-Carlo-Wanderer

The MonteCarloWanderer is a algorithm that works on the graph itself. It is a
exploration algorithm. This algorithm has some huge drawbacks and some
advantages over the ODE-Solvers and the analytical solutions. <br>
The Main disadvantage is that it is a Monte Carlo-Method and as such is
inefficient, because it needs extensive calculation to archive convergence.<br>
The main advantage is, that it is a Monte Carlo-Method, which means that, (at
least inuitivly {Nothing is prooven yet..} it should also work good on
problems with no analytical Solutions that can't be solved by ODE Solvers.).<br>
But how does this algorithm even work? <br>
The Algorithm explores the graph at random. At each step it draws a random node
out of the graph. The probability that a node is chosen is wheighted by the
occupation at the begin of the timestep. It than takes a random step in the
graph, where the edgevalues are the weight for a transition to occur. The final
node after the transition is stored in a histogram. The values of the histogram
bins normalized with the number of transitions simulated this way equal the
occupation values for the next step.
The Picture below shows a stepwise graph exploration.

![Monte Carlo-Wanderer](../../additional_doc/img/graph_exploration.jpg "Monte Carlo-Wanderer")

## Notation: Identifying states

In the whole project a specific notation do denote QuantumSystem-states is used.
The notation assignes a unique number to every state.<br>
When a supclass of the Quantumsystem is created the Niveaus are specified by the user. The niveaus
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
important to keep track of this orientation in the metadata.

