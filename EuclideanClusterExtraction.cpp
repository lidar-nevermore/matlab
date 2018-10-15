#include <stdio.h>
#include <string.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <sstream>
#include <fstream>
#include <queue>
#include "mex.h"

using namespace std;

void extractEuclideanClusters(
	const mxArray* point_neighbours, 
	vector<int>& point_labels,
	int min_pts_per_cluster,
	int max_pts_per_cluster)
{
	size_t i, j;
	int num_of_pts,id, sq_idx;
	mwIndex m;
	const mxArray *cell_element_ptr;
	mwSize total_num_of_elements;
	double* nghbrs;

	m = mxGetM(point_neighbours);	
	num_of_pts = static_cast<int> (m);
	std::vector<bool> processed(num_of_pts, false);
	point_labels.assign(m, 0);
	id = 1;
	for (i = 0; i < num_of_pts; ++i)
	{
		if (processed[i])
			continue;
		std::vector<int> seed_queue;
		sq_idx = 0;
		seed_queue.push_back(i);
		processed[i] = true;
		while (sq_idx < static_cast<int> (seed_queue.size()))
		{
			//get nghbrs
			cell_element_ptr = mxGetCell(point_neighbours, seed_queue[sq_idx]);
			total_num_of_elements = mxGetNumberOfElements(cell_element_ptr);
			nghbrs = mxGetPr(cell_element_ptr);// matlab index starts from 1;        
			for (j = 0; j < total_num_of_elements; ++j)
			{
				if (processed[nghbrs[j] - 1])// matlab index starts from 1;    
					continue;
				// Perform a simple Euclidean clustering
				seed_queue.push_back(nghbrs[j] - 1);
				processed[nghbrs[j] - 1] = true;
			}
			sq_idx++;//next seed
		}

		// If this queue is satisfactory, add to the clusters
		if (seed_queue.size() >= min_pts_per_cluster && seed_queue.size() <= max_pts_per_cluster)
		{
			for (size_t j = 0; j < seed_queue.size(); ++j)
				point_labels[seed_queue[j]] = id;
			++id;
		}
	}
}

void
mexFunction(int nlhs, mxArray *plhs[],
	int nrhs, const mxArray *prhs[])
{
	(void)nlhs;     /* unused parameters */
	double  *pointer;
	mwSize index;
	
	vector<int> point_labels_;

	int min_pts;//=50;
	int max_pts;//=20000;  
	min_pts = static_cast<float>(mxGetScalar(prhs[1]));
	max_pts = static_cast<int>(mxGetScalar(prhs[2]));

	mexPrintf("Euclidean Cluster parameters:\n");
	mexPrintf("min_pts_per_cluster: %d\n", min_pts);
	mexPrintf("max_pts_per_cluster: %d\n", max_pts);

	extractEuclideanClusters(prhs[0], point_labels_, min_pts, max_pts);
	plhs[0] = mxCreateNumericMatrix(point_labels_.size(), 1, mxDOUBLE_CLASS, mxREAL);
	pointer = mxGetPr(plhs[0]);
	/* Copy data into the mxArray */
	for (index = 0; index < point_labels_.size(); index++)
	{
		pointer[index] = point_labels_[index];
	}
}