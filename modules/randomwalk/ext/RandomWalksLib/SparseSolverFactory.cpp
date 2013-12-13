#include "SparseSolverFactory.h"
#include "SparseSolverEigenLLT.h"
#include "SparseSolverEigenCG.h"
#include "SparseSolverEigenBiCGSTAB.h"
#include "SparseSolverEigenCustom.h"

#ifdef RANDOMWALKSLIB_HAS_OPENCL
#include "SparseSolverViennaCPU.h"
#include "SparseSolverViennaGPU.h"
#endif

#include <iostream>

SparseSolverInterface * SparseSolverFactory::createSolver(std::string type, int iterations, double tolerance)
{
	this->iterations = iterations;
	this->tolerance = tolerance;

	if(type.compare("Eigen-LLT")==0)
	{
		return new SparseSolverEigenLLT();
	}
	else if(type.compare("Eigen-CG")==0)
	{
		SparseSolverInterface * solver = new SparseSolverEigenCG(iterations,tolerance);
		return solver;
	}
	else if(type.compare("Eigen-BiCGSTAB")==0)
	{
		SparseSolverInterface * solver = new SparseSolverEigenBiCGSTAB(iterations,tolerance);
		return solver;
	}
	else if(type.compare("Eigen-CG-Custom")==0)
	{
		SparseSolverInterface * solver = new SparseSolverEigenCustom(iterations,tolerance);
		return solver;
	}
	else
	{
		return new SparseSolverEigenLLT();
	}
}