#include<armadillo>
#include<map>
#include<vector>
#include<utility>
#include<fstream>

/**
* In this unoverwritten state this class mainly exists for the user. It holds
* information of the energy niveaus in the QuantumSystem that will be saved in
* the Quantumsystems datafile. This way it is easyer to identify the origin of
* the data.
* It is also intended to be overwritten in actual programs using this library.
* You could store additional information in it like f.e. wavefunktion data. This
* class may also be used as a assistive datastructure during the preprocessing
* or the calculation of the transition probabilities.
*/
class Niveau
{
	protected:

	/**
	* Name of the niveau... depends on the system that is simulated. In a
	* atom-like system maybe something like "s1","s2","p1"... This information
	* purely exists for the creation of readable data.
	*/
	std::string Name;
	
	/**
	* The potential Energy of the quantumobject occupying this niveau. Something
	* like
	*/
	double Energy; //in eV
	
	/**
	* The Measured absolute of the spin vector in this niveau. For a electron it
	* could be something like +0.5 or -0.5.
	*/
	double Spin;

	public:
	
	Niveau(
		std::string name,
		double energy,
		double spin
	):
		Name(name),
		Energy(energy),
		Spin(spin)
	{}
	
	/**
	* Writes the data that is held by this class as xml.
	*/
	void writeToFile(std::ofstream& file,std::string indent)
	{
		file << indent << "<niveau>\n";
		
		file << indent << "\t<name> " << Name << "</name>\n";
		file << indent << "\t<energy_in_eV> " << Name << "</energy_in_eV>\n";
		file << indent << "\t<Spin> " << Spin << "</Spin>\n";
		
		file << indent << "</niveau>\n";
	}

};

/**
* This Class represents a Quantumsystem with discrete finite states. It
* encapsulates all the physics that make up the System. The System is Stored as
* a Graph. The Nodes in the graph are the states the system can be in. The edges
* are the different transitions that can occur. The weight of the edges is the
* probability that a transition occurs in a Unit time.<br>
* This Class handles all access on the physical parameters of the system, be it
* the request of a masterequation or transition probabilities for single states.
* This way, the QuantumSystem object can keep Track of the access and can create
* a "on the fly" look-up-table so that already known physical data must not be
* computed twice. This way, assuming the evaluation of the physical equations
* describing the system is expensive, runtime can be saved. This assumption
* seems legit, since most of the time one is dealing with exponentials when
* describing quantum Systems.<br>
* All concrete quantum Systems should be subclasses of this abstract base. All
* solvers should be implemented using the abstract base. This way, it is save,
* that they all can operate on every Quantumsystem that is described using this
* library.<br>
* The informations about the physics of the System are strored in the Graph,
* representing the system and its dynamics. A subclass must override the actualisationNeedet
* and the calculateProbabilities functions. The first of which is checking if
* the transition probabilities for a given state at a given time should be
* calculated. The second adds the edges to the graph.
* This class also saves the state of the System i.e. the occupation of the
* states and the transition probabilities at a given time. It can write this
* data to a graphml file. 
*/
class QuantumSystem
{
	protected:
	

	class State;
	
	/*
	* This Class represents a (directed) edge in the Graph describing the physics of this
	* quantumsystem. 
	*/	
	struct Edge
	{
		/**
		* The State on which the edge is pointing. This is the endstate of the
		* transition the edge is symbolizing.
		*/
		State& targetState;
		
		/**
		* The Probability that the Transition the Edge is Symbolizing is
		* occurring.
		*/
		double transitionProbabilitie;
		
		/**
		* This is the name of the edge. Currently its not implemented. This
		* string could store data that identifies the edge. Could f.e. be the
		* physical origin of the edge like : "tunneling", "spinflip",
		* "relaxation", ...
		*/
		std::string Id;
		
		/**
		* This is the "history" to the edge. The key-string in the map holds a
		* timestamp and the double is the transitionProbabilitie at this time.
		*/
		std::map<std::string,double> rate;

		Edge(
			State& p_targetState,
			double p_transitionProbabilitie,
			std::string id
		):
			targetState(p_targetState),
			transitionProbabilitie(p_transitionProbabilitie),
			Id(id)
		{}

		/**
		* Outputs the values held in rate as a graphml snippet to the file.
		*/
		void writeToFile(std::ofstream& file,int indentTabs,int originState)
		{
			std::string indent = "";
			for(int i = 0;i<indentTabs;i++)
			{
				indent += '\t';
			}
			
			file << indent << "<edge ";
			file << "id=\"" << Id << "\" ";
			file << "source=\"" << originState << "\" ";
			file << "target=\"" << targetState.number() << "\">\n";
			
			for(auto pair : rate)
			{
				file << indent << '\t' << "<data key=\"" << pair.first << "\">";
				file << pair.second;
				file << "</data>" << '\n';
			}

			file << indent << "</edge>\n";
		}
	};
	
	/**
	* This class represents a state the quantumsystem can be found in. It
	* simultaneously acts as a node in the Graph describing the Quantumsystem.
	*/
	class State
	{
		protected:
		
		/**
		* This Variable is true if the transition probabilities for this State
		* have been calculated before for some time.
		*/
		bool IsInitialized = false;
		
		/**
		* The History of the Node is stored here. The Keys in this map, i.e. the
		* strings are timestamps. The map holds the corresponding occupation
		* values to the time that is encoded by the timestamp.
		*/
		std::map<std::string,double> Occupation;

		/**
		* Every state has a unique number. What the numbers physically mean is
		* depends on the subclass of this class. important is that they reach
		* from 0 to the number of possible states the system can be found in.
		* <br>
		* One Way to assign statenumbers is to number the energy niveaus in the
		* system and sort them after their numbers. If we than use a 0 if the
		* Niveau is not occupied in a given state and a 1 if its occupied we
		* create a unique binary string that has a representation as decimal
		* number for every state.
		*/
		int StateNumber;
		
		/**
		* The Possible transitions from this state to other states of the
		* system.
		*/
		std::vector<Edge> Edges;
		
		/**
		* The systemtime which corresponds to the last calculation of the
		* transitionprobabilities.
		*/
		double LastActualisation;

		public:
		
		State(
			int p_stateNumber,
			double initialTime
		):
			StateNumber(p_stateNumber),
			LastActualisation(initialTime)
		{}
		
		/**
		* This method is used by the QuantumSystem to store the calculated edges
		* on actualisation of this node.
		*
		* @param time The in simulation time that corresponds to the transitions
		* and probabilities.
		* @param newEdges The edges encoding the transitions that are set to the
		* state. 
		*/
		void storeEdges(double time,std::vector<Edge>& newEdges)
		{
			for(Edge e : newEdges)
			{
				Edges.push_back(e);
			}

			LastActualisation = time;
			IsInitialized=true;
		}
		
		/**
		* Returns the time value where the last actualisation of the
		* edge-transition values for the edges that point away from this point
		* occured.
		*/
		double lastActualisation()
		{
			return LastActualisation;
		}
		
		/**
		* Returns the edges that point away from this node.
		*/
		std::vector<Edge>& edges()
		{
			return Edges;
		}
		
		
		/**
		* returns the statenumber, i.e. the identifier of this state.
		*/
		int number()
		{
			return StateNumber;
		}
		
		/**
		* Saves the given occupation value to the Occupation map. This Method is
		* used by the solver to save the state of the system for a given
		* keyframe.
		*/
		void logOccupation(std::string timeKey,double occ)
		{
			Occupation.insert({timeKey,occ});
		}
		
		/**
		* Outputs the Occupation and other parameters that make up this node as
		* graphml snippet.
		*/
		void writeToFile(std::ofstream& file,int indentTabs)
		{
			std::string indent = "";
			for(int i = 0;i<indentTabs;i++)
			{
				indent += '\t';
			}
			file << indent <<	"<node id=\"" << StateNumber << "\">" << "\n";
			
			for(auto pair : Occupation)
			{
				file << indent << '\t' << "<data key=\"" << pair.first << "\">";
				file << pair.second;
				file << "</data>" << '\n';
			}
			file << indent <<	"</node>" << "\n";

		}

		/**
		* Returns the initialisation status of the node. Initialized means that
		* all the edges are generated and their transition probabilities are
		* calculated. The method is used by the Quantumsystem to keep track of
		* the calculated values.
		*/
		bool isInitialized()
		{
			return IsInitialized;
		}

	};
	
	/**
	* This vector holds all States the system can be found in. It is used as a
	* lookup-table.
	*/
	std::vector<State> allStates;
	
	/**
	* This method should return true if the probabilities of a state should be
	* actualized at a given time. Therefore it can compare the last
	* actualisation time of the state, which corresponds to the states values
	* with the time given. 
	* Note that is must not check if the state is initilised or not since this
	* check is already been done at the point this method is called.
	* Also see getProbabilities.
	*/
	virtual bool actualisationNeedet(double time,State& s)=0;
	
	/**
	* This method is called if actualisationNeedet evaluates to true.
	* It should actualize the given edge.
	*
	* IMPORTANT NOTE: The trivial transition, i.e. the system remains in its
	* initial state must not be added here. The Solvers or the
	* Masterequation-generator handle it.
	*/
	virtual void calculateProbabilities(double time,Edge& e,State& origin) = 0;
	
	/**
	* This method creates all edges and initializes their transition values. It
	* is used to initialize a state.
	*/	
	virtual void createEdges(double time,State& s)=0;

	/**
	* This Method returns the Transitions and Probabilities of the state s. It
	* is used as a central internal hub for requests of the physical behaviour.
	* It checks if the values stored should be actualized or not.
	*/
	std::vector<Edge> getProbabilities(double time,State& s)
	{
		if(!s.isInitialized())
		{
			createEdges(time,s);
		}
		else
		{
			if(actualisationNeedet(time,s))
			{
				for(Edge& e: s.edges())
					calculateProbabilities(time,e,s);	
			}
		}

		return s.edges();
	}
	
	std::string PathToSave;
	
	/**
	* The name of the system and the filename for the data generated by the
	* system.
	*/
	std::string SystemDesignator;

	/**
	* The Energy Niveaus that make up this system. The order the niveaus are
	* listed in this vector make up the order that is relevant for the notation
	* of states.
	*/
	std::vector<Niveau> Niveaus;
	
	public:

	QuantumSystem(
		std::vector<Niveau> niveaus,
		double initialTime,
		std::string pPathToSave,
		std::string pSystemDesignator = ""
	):
		Niveaus(niveaus),
		PathToSave(pPathToSave),
		SystemDesignator(pSystemDesignator)
	{
		
		int numberOfStates = pow(2,niveaus.size());

		for(int stateNum =0; stateNum< numberOfStates;stateNum++)
		{
			allStates.push_back(State(stateNum,initialTime));
		}
	}

	/**
	* This method provides external access on the transitions and their
	* probabilities for a single state. Especially it is used by methods that
	* work on the Graph it self like the MonteCarloWanderer Method. 
	*
	* @param originStateNum The identification-number of the state in question.
	* @param time The time that corresponds to the transition values in
	* question.
	* @return a std::vector of std::pair<int,double>, where each pair represents
	* a transition. The integer number is the end-state of the transition. The
	* double is the probability that the transition occurs.
	*/
	std::vector<std::pair<int,double>> possibleTransitions(int originStateNum,double time)
	{
		//This works because by construction the number of the state is its
		//index in the vecor
		State& s = allStates[originStateNum];

		getProbabilities(time,s);

		std::vector<Edge>& edges = s.edges();

		std::vector<std::pair<int,double>> toReturn;

		for(Edge& e : edges)
		{
			toReturn.push_back(std::make_pair(e.targetState.number(),e.transitionProbabilitie));
		}

		return toReturn;
	}
	
	protected:
	
	/**
	* The Matrix that is the main part of the systems masterequation.
	*/	
	arma::Mat<double> W;
	
	/**
	* Is true if the Matrix W has been initialized
	*/
	bool WInitialized=false;

	/**
	* The Timekeys that encode the time where the systemstate was saved.
	*/
	std::vector<std::string> saveTimeKeys;
	
	/**
	* The specifier for the keys of the edgevalues in the Graphml file
	*/
	std::string edgeKeySpecifier = SystemDesignator+"Wheights_At_t=";
	
	/**
	* The specifier for the keys of the node occupation values in the Graphml file
	*/
	std::string nodeKeySpecifier = SystemDesignator+"Occupation_At_t=";
	
	/**
	* The specifier for the keys of the edgevalues in the edges in the graphml
	* file.
	*/
	std::string edgeDataSpecifier = SystemDesignator+"Wheights_@_";
	
	/**
	* The specifiers for the Node-Occupation values saved in the node snippets
	* in the graphml file.
	*/
	std::string nodeDataSpecifier = SystemDesignator+"Occupation_@_";
	
	public:
	
	/**
	* Makes a snapshot of the system at a given time.
	*/
	void logMoment(double time,std::vector<double> occupation)
	{	
		std::string timeKey = std::to_string(time);
		
		saveTimeKeys.push_back(timeKey);
		
		for(State& s : allStates)
		{
			getProbabilities(time,s);

			std::vector<Edge>& edges = s.edges();
			for(Edge& e : edges)
			{
				e.rate.insert({edgeDataSpecifier+timeKey,e.transitionProbabilitie});
			}

			s.logOccupation(nodeDataSpecifier+timeKey,occupation[s.number()]);
		}
	}

	std::string pathToSave()
	{
		return PathToSave;
	}	

	/**
	* This Method is used to access the Masterequation of this quantumsystem. It
	* is mainly used by odesolvers or analytical solutions of the system if they
	* exist.
	*/
	arma::Col<double> ODE(double time,arma::Col<double> probabilities)
	{
		if(!WInitialized)
		{
			W=arma::mat(allStates.size(),allStates.size(),arma::fill::zeros);
			WInitialized=true;
		}

		for(State& s : allStates)
		{
			getProbabilities(time,s);

			double pGo = 0;

			std::vector<Edge>& edges = s.edges();
			
			for(Edge& e : edges)
			{
				W(s.number(),e.targetState.number()) = e.transitionProbabilitie;
				pGo -= e.transitionProbabilitie;

			}

			W(s.number(),s.number()) =pGo;
		}
		
		arma::Col<double> to_return = W*probabilities;

		return to_return;
	}
	
	/**
	* Writes the system-saves to a graphml file that contains the Systemgraph
	* and the Values of the edges and nodeoccupation on the key-frame-times.
	*/
	void writeToFile()
	{
		std::ofstream file(PathToSave);
		
		file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n";
		
		file << "<SystemInformation>\n";
		for(Niveau & n : Niveaus)
		{
			n.writeToFile(file,"\t");
		}
		file << "</SystemInformation>\n\n";

		file << "<graphml xmlns=\"http://graphml.graphdrawing.org/xmlns\">\n";
		
		for(std::string t : saveTimeKeys)
		{
			//Node Keys
			file << '\t' << "<key attr.name=\""<< nodeKeySpecifier << t;
			file << "\" attr.type=\"float\" for=\"node\" id=\"";
			file << nodeDataSpecifier << t << "\"/> \n";
			// Edge Keys
			file << '\t' << "<key attr.name=\""<< edgeKeySpecifier << t;
			file << "\" attr.type=\"float\" for=\"edge\" id=\"";
			file << edgeDataSpecifier << t << "\"/> \n";

		}
					
		file << "<graph edgedefault=\"directed\">\n";
		for(State& s : allStates)
		{
			s.writeToFile(file,1);
			for(Edge& e: s.edges())
			{
				e.writeToFile(file,2,s.number());
			}
		}

		file << "</graph>\n";
		file << "</graphml>";
		
		file.close();
	}
	
	/**
	* This Method returns the number of different states the system can be found
	* in.
	*/
	int numberOfStates()
	{
		return allStates.size();
	}	
};