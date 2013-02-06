#include "SparseSolverEigenBiCGSTAB.h"
#include "Eigen/src/IterativeLinearSolvers/BiCGSTAB.h"

SparseSolverEigenBiCGSTAB::SparseSolverEigenBiCGSTAB( int iterations, double tolerance )
{
	this->iterations = iterations;
	this->tolerance = tolerance;
}

std::vector<double> SparseSolverEigenBiCGSTAB::solve_Ax_b( SparseMatrix<double> A, SparseVector<double> b, int numel, std::vector<int> & uidx, const std::vector<int> * labels, const std::vector<int> * seeds, int active_label )
{
	VectorXd b_dense = b;

	// Eigen Conjugate Gradient Solver, by default Jacobi Preconditioning is used
	BiCGSTAB<SparseMatrix<double> > bcg;
	bcg.setMaxIterations(iterations);
	bcg.setTolerance(tolerance);

	bcg.compute(A);
	VectorXd x_dense = bcg.solve(b_dense);

	std::vector<double> xmat(numel);

	for (int i=0; i<x_dense.rows(); i++)
	{
		double val = x_dense(i);
		xmat[uidx[i]] = val;
	}

	for (size_t i=0; i<seeds->size(); i++)
	{
		if((*labels)[i] == active_label)
			xmat[(*seeds)[i]] = 1.0;
		else
			xmat[(*seeds)[i]] = 0.0;
	}

	return xmat;
}