#include "CDCL_Solver.h" 

CDCLSolver::CDCLSolver()
{	
	//Blank
};

CDCLSolver::~CDCLSolver()
{
	Clauses_gen.clear();
	Resolvent_Clause.clear();
}

void CDCLSolver::Init(int clause_limit)
{
	//Start at level 1
	Current_DL=1;
	Backprop_DL=1;

	//Set clause generation limit
	this->clause_limit=clause_limit;
}

void CDCLSolver::Load(const char* filename)
{
	//DIMACs format only
	std::ifstream file;
	file.open(filename,std::ifstream::in);

	char c;
	std::string s;

	while(true)
	{
		file>>c;
		if(c=='c')	//Comment
		{
			getline(file,s);	//ignore
		}
		else if(c=='p')
		{
			file>>s;	//cnf
			break;
		}
	}
	//std::cout<<"Input literal count:\n";
	file>>literal_count;
	//std::cout<<"Input clause count:\n";
	file>>clause_count;
	//No literals assigned yet
	assigned_lits=0;
	//No conflict clauses yet
	confl_ante=-1;
	already_unsat=false;

	literals.clear();
	literals.resize(literal_count,0);

	liter_freq.clear();
	liter_freq.resize(literal_count,0);

	liter_pol.clear();
	liter_pol.resize(literal_count,0);

	orig_formula.clear();
	orig_formula.resize(clause_count);

	literal_ante.clear();
	literal_ante.resize(literal_count,-1);

	int literal;
	int lit_count_in_clause;


	lis.clear();
	lis.resize(literal_count,0);
	//li
	for(unsigned int li_iter=0;li_iter<literals.size();li_iter++)
	{
		lis[li_iter]=li_iter+1;
	}

	for(int i=0;i<clause_count;i++)
	{
		lit_count_in_clause=0;
		//std::cout<<"Enter clause"<<i+1<<"\n";
		while(true)
		{
			file>>literal;
			if(literal>0)
			{
				liter_freq[literal-1]++;
				liter_pol[literal-1]++;
				orig_formula[i].push_back(literal);
			}
			else if(literal<0)
			{
				liter_freq[-literal-1]++;
				liter_pol[-literal-1]--;
				orig_formula[i].push_back(literal);
			}
			else
			{
				if(lit_count_in_clause==0)
				{
					already_unsat=true;
				}
				break;
			}
			lit_count_in_clause++;
		}
	}
	orig_lit_freq=liter_freq;	//Used for restoring frequencies later

	//Print clauses for debug purposes
	std::cout<<"Clause formula\n";
	for(unsigned int c=0;c<orig_formula.size();c++)
	{
		
		for(unsigned int li=0;li<orig_formula[c].size();li++)
		{
			std::cout<<orig_formula[c][li]<<" ";
		}
		std::cout<<"\n";
	}
}

void CDCLSolver::Read()
{
	char c;
	std::string s;

	while(true)
	{
		std::cin>>c;
		if(c=='c')	//Comment
		{
			getline(std::cin,s);	//ignore
		}
		else if(c=='p')
		{
			std::cin>>s;		//cnf
			break;
		}
	}
	//std::cout<<"Input literal count:\n";
	std::cin>>literal_count;
	//std::cout<<"Input clause count:\n";
	std::cin>>clause_count;
	//No literals assigned yet
	assigned_lits=0;
	//No conflict clauses yet
	confl_ante=-1;
	already_unsat=false;

	literals.clear();
	literals.resize(literal_count,0);

	liter_freq.clear();
	liter_freq.resize(literal_count,0);

	liter_pol.clear();
	liter_pol.resize(literal_count,0);

	orig_formula.clear();
	orig_formula.resize(clause_count);

	literal_ante.clear();
	literal_ante.resize(literal_count,-1);

	int literal;
	int lit_count_in_clause;


	lis.clear();
	lis.resize(literal_count,0);
	//li
	for(unsigned int li_iter=0;li_iter<literals.size();li_iter++)
	{
		lis[li_iter]=li_iter+1;
	}

	for(int i=0;i<clause_count;i++)
	{
		lit_count_in_clause=0;
		std::cout<<"Enter clause"<<i+1<<"\n";
		while(true)
		{
			std::cin>>literal;
			if(literal>0)
			{
				liter_freq[literal-1]++;
				liter_pol[literal-1]++;
				orig_formula[i].push_back(literal);
			}
			else if(literal<0)
			{
				liter_freq[-literal-1]++;
				liter_pol[-literal-1]--;
				orig_formula[i].push_back(literal);
			}
			else
			{
				if(lit_count_in_clause==0)
				{
					already_unsat=true;
				}
				break;
			}
			lit_count_in_clause++;
		}
	}
	orig_lit_freq=liter_freq;	//Used for restoring frequencies later

	//Print clauses for debug purposes
	std::cout<<"Clause formula\n";
	for(unsigned int c=0;c<orig_formula.size();c++)
	{
		
		for(unsigned int li=0;li<orig_formula[c].size();li++)
		{
			std::cout<<orig_formula[c][li]<<" ";
		}
		std::cout<<"\n";
	}
}

int CDCLSolver::Solve()
{	
	int status;
	//Load the original formula
	curr_formula=orig_formula;

	Current_DL=1;			//start at level 1
	if(already_unsat)		//if at start either of clauses is empty
	{
		std::cout<<"Unsat at start there exists a null clause\n";
		return UNSAT;
	}

	//Perform unit propagation
	int UP_Res=UnitProp();
	//Started at level 1 and conflict means UNSAT
	if(UP_Res==CONFL)
	{
		Solver_Status=UNSAT;
		return UNSAT;
	}

	//Run loop until all literals are assigned
	while(!All_Assigned())
	{
		//Pick literal
		int literal_picked=PickLiteral();

		//Increment Decision level
		Current_DL++;

		AssignLit(literal_picked,Current_DL,-1);

		while(true)
		{	
			//Unit propagate at current DL
			UP_Res=UnitProp();
			if(UP_Res==CONFL)
			{
				if(Current_DL==1)	//level 1 UNSAT
				{
					Solver_Status=UNSAT;
					return UNSAT;
				}
				//else 
				//perform conflict analysis
				status=Conflict();

				//Perform backtracking
				status=BackTrack();
			}
			//If no conflict found break from this loop and pick
			//new literal
			else
			{
				break;
			}
		}
	}

	//meaning all literals are assigned succesfully
	//So formula is SAT
	Solver_Status=SAT;
	return SAT;
}

int CDCLSolver::UnitProp()
{	
	//Initially unit clause not found
	bool uc_found=false;

	//False literals assigned in a clause
	//Useful for finding the conflict
	unsigned int false_count=0;

	//Unset literals in clause
	//Useful for finding unit literals
	unsigned int unset_count=0;

	//SATisfiability status of current clause
	bool sat_clause=false;

	//Keep track of last unset literal
	int last_unset=-1;
	//literal index
	int li;

	//While unit clause is not found
	do
	{
		uc_found=false;
		//search all clauses
		for(unsigned int i=0;i<curr_formula.size() && !uc_found;i++)
		{
			std::cout<<"Clause "<<i+1<<"\n";

			false_count=0;
			unset_count=0;
			sat_clause=false;

			//Go through all literals
			for(unsigned int j=0;j<curr_formula[i].size();j++)
			{	
				//Literal index 0-indexed
				li=mod(curr_formula[i][j])-1;

				//If unassigned increase unset count
				if(literals[li]==0)
				{
					std::cout<<"Literal x"<<li+1<<" unset\n";
					unset_count++;
					last_unset=j;	//store this
				}
				//If falsely assigned then it's a false assignment
				else if((literals[li]>0 && curr_formula[i][j]<0) || (literals[li]<0 && curr_formula[i][j]>0))
				{
					false_count++;
				}
				//If none of the above occurs the values are assigned
				//without conflicts so satisfied so continue with next clause
				else
				{
					sat_clause=true;
					break;
				}
			}
			//if clause is satisfied
			if(sat_clause)
			{	
				//continue with next clause
				continue;
			}
			//If there's exactly one unset literal, then it is a unit literal
			if(unset_count==1)
			{
				//assign literal at this decision level and set it's antecedent
				//Assign antecedent as clause i;
				std::cout<<"Unit literal x"<<curr_formula[i][last_unset]<<" found in clause:"<<i+1<<"\n";
				AssignLit(curr_formula[i][last_unset],Current_DL,i);

				uc_found=true;	//So reiterate and apply this new value
				break;
			}
			else if(false_count==curr_formula[i].size())
			{
				//UNSAT clause so this is conflict
				std::cout<<"Conflict found\n";
				confl_ante=i;	//Conflict antecedent from clause i
				return CONFL;	//Return conflict status
			}
		}
	}
	//repeat as long there is a unit clause
	while(uc_found);

	//If it reached here there's no conflict clause so
	//set it as -1
	confl_ante=-1;
	//Return "continue" value to continue 
	return CONT;
}

//Assigning takes sign from the clause
//Assign literal at DL and with antecedent clauses
void CDCLSolver::AssignLit(int literal,int DL,int ante)
{
	if(literal>0)
	{
		literals[literal-1]=DL;
		literal_ante[literal-1]=ante;
		liter_freq[literal-1]=-1;
	}
	else
	{
		literals[-1-literal]=-DL;
		literal_ante[-1-literal]=ante;
		liter_freq[-1-literal]=-1;
	}
	//Increment count assigned literals
	assigned_lits++;
}

//li is 0-indexed
//Unassign literal
void CDCLSolver::UnassignLit(int li)
{
	literals[li]=0;
	literal_ante[li]=-1;
	liter_freq[li]=orig_lit_freq[li];
	//Decrement count assigned literals
	assigned_lits--;
}

int CDCLSolver::Conflict()
{
	std::vector<int> new_clause=curr_formula[confl_ante];
	std::cout<<"Conflict clause:";
	std::cout<<new_clause;
	std::cout<<"\n";
	//literals at current decision level
	int lits_at_this_lvl=0;
	//resolver literal
	int resolver_lit;
	//literal index
	int li;
	std::cout<<"Literal antes:"<<literal_ante<<"\n";
	do
	{
		lits_at_this_lvl=0;
		for(unsigned int i=0;i<new_clause.size();i++)
		{
			li=mod(new_clause[i])-1;
			//Conflict at current decision level
			if(mod(literals[li])==Current_DL)
			{
				lits_at_this_lvl++;
				//This is not assigned in the clause
				if(literal_ante[li]!=-1)
				{
					resolver_lit=li;
					std::cout<<"Resolver literal:"<<resolver_lit<<"\n";
				}
			}
		}
		if(lits_at_this_lvl==1)
		{
			std::cout<<"UIP reached\n";
			break;
		}
		//Generate new clause with resolver literal

		//std::cout<<"Before resolution\n";
		new_clause=Resolve(new_clause,resolver_lit);
		std::cout<<new_clause<<"\n";
	}
	while(true);
	//While Unit Implication point(UIP) is found

	Resolvent_Clause=new_clause;
	if(clause_count<clause_limit);
	{
		Clauses_gen.push_back(new_clause);
		curr_formula.push_back(new_clause);
		clause_count++;

		//Update polarities and frequency
		for(unsigned int i=0;i<new_clause.size();i++)
		{
			li=mod(new_clause[i])-1;
			if(new_clause[i]>0)
			{
				liter_pol[li]+=1;
			}
			else
			{
				liter_pol[li]-=1;
			}
			if(liter_freq[li]!=-1)
			{
				liter_freq[li]++;
			}
			orig_lit_freq[li]++;
			clause_count++;
		}
	}
	return CONT;
}

int CDCLSolver::BackTrack()
{
	int li;
	//decision level of literal
	int dli;
	Backprop_DL=1;

	if(Resolvent_Clause.size()!=1)
	{
		for(unsigned int i=0;i<Resolvent_Clause.size();i++)
		{
			li=mod(Resolvent_Clause[i])-1;
			dli=mod(literals[li]);

			//Max of literal decision level
			if(dli!=Current_DL && dli>Backprop_DL)
			{
				Backprop_DL=dli;
			}
		}
	}

	//Unassign literals after this level
	for(unsigned i=0;i<literals.size();i++)
	{
		if(mod(literals[i])>Backprop_DL)
		{
			//0-indexed literal
			UnassignLit(i);
		}
	}

	//Update current decision level to backprop level
	Current_DL=Backprop_DL;

	return Backprop_DL;
}

std::vector<int> CDCLSolver::Resolve(std::vector<int> clause,int resolver_lit)
{
	//Other clause to resolve with
	std::cout<<"literal ante:"<<literal_ante[resolver_lit]<<"\n";
	std::vector<int> other_clause=curr_formula[literal_ante[resolver_lit]];

	std::cout<<"Resolution started\n";
	std::cout<<"First clause:"<<clause<<"\n";
	std::cout<<"Other clause:"<<other_clause<<"\n";
	//Concat two
	clause.insert(clause.end(),other_clause.begin(),other_clause.end());
	//Remove resolver literal
	for(unsigned int i=0;i<clause.size();i++)
	{
		if(mod(clause[i])==resolver_lit+1)	//remove the resolver literal
		{
			clause.erase(clause.begin()+i);
			i--;
		}
	}
	//Along with duplicates
	sort(clause.begin(),clause.end());
	clause.erase(unique(clause.begin(),clause.end()),clause.end());

	std::cout<<"Final resolvent"<<clause<<"\n";

	//Return the resolvent
	return clause;
}

//Pick literal based on Heuristics and other methods
//Or from Random Distribution
//Or pick literal based on frequency of occurences
int CDCLSolver::PickLiteral()
{	
	//Pick random literal
	int li= *(select_randomly(lis.begin(),lis.end()));

	std::vector<int> signs{1,-1};
	int sign= *(select_randomly(signs.begin(),signs.end()));

	std::cout<<"Picked literal:"<<li*sign<<"\n";
	return li*sign;
}

//Condition for algorithm working
bool CDCLSolver::All_Assigned()
{
	return literal_count==assigned_lits;
}

void CDCLSolver::ShowResult()
{
	if(Solver_Status==SAT)
	{
		//Print out assignments
		for(unsigned int i=0;i<literals.size();i++)
		{
			if(literals[i]>0)
			{
				std::cout<<"x"<<i+1<<" ";
			}
			else if(literals[i]<0)
			{
				std::cout<<"~x"<<i+1<<" ";
			}
			else
			{
				std::cout<<"don't care-x"<<i+1<<" ";
			}
		}
	}
	else
	{
		std::cout<<"Formula is UNSAT\n";
		return;
	}
}