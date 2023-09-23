/*
----------------------------------------------------------------------------------
Name: 	SreekashUS
----------------------------------------------------------------------------------
*/


//Library includes
#include <iostream>
#include <cmath>
#include <vector>
#include <cstring>


//Project includes
#include <CDCL_Solver.h>


int main(int argc, char** argv) 
{

	//User interface related printing
	//std::cout<<argc<<"\n";
	//std::cout<<argv[0]<<argv[1]<<"\n";

	if(argc==1 || strcmp(argv[1],"-h")==0)
	{
		std::cout<<"Usage: [options] [file_path]\n";
		std::cout<<"options:\n";
		std::cout<<"\t-c = command line input of DIMACS format\n";
		std::cout<<"\t-f = file input of DIMACS format\n";
		std::cout<<"\t-s = folder input for checking all DIMACs files\n";
		std::cout<<"file_path:\t(Only available for -f and -s modes)\n";
		std::cout<<"\t File/Folder that is to be read from\n";

		return EXIT_FAILURE;
	}

	std::string filename;

	CDCLSolver solver; 				// create the solver
	solver.Init(20);  				// initialize

	//Load the formula(i.e clauses from DIMACs file or command prompt)
	if(strcmp(argv[1],"-c")==0)
	{
		std::cout<<"Input the cnf formula:\n";
		solver.Read();
	}
	else if(strcmp(argv[1],"-f")==0)
	{
		std::cout<<"Input filename:";
		filename=argv[2];
		solver.Load(filename.c_str());
	}

	int status=solver.Solve();       			// solve

	solver.ShowResult();			//Show result

	return EXIT_SUCCESS;
}