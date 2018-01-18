// Region_growing implemented in matlab-C++


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

inline bool	comparePair (std::pair<float, int> i, std::pair<float, int> j)
{
	return (i.first < j.first);
}

bool validatePoint (
	int initial_seed, 
	int point, 
	int nghbr, 
	bool& is_a_seed,
	const mxArray *normals_,   
	float theta_threshold,
	float curvature_threshold) 
{
    double* data;
    mwIndex m;    
    m=mxGetM(normals_);
    data=mxGetPr(normals_);
	is_a_seed = true;
	float cosine_threshold = cosf (theta_threshold);	    
	//check the angle between normals	       
	float dot_product = static_cast<float>(data[point]*data[nghbr]+data[m+point]*data[m+nghbr]+data[m*2+point]*data[m*2+nghbr]);
	if (fabsf(dot_product) < cosine_threshold)
	{
		return (false);
	}	
	// check the curvature
	if (data[m*3+nghbr] > curvature_threshold)
	{
		is_a_seed = false;
	}
	return (true);
}

int growRegion (int initial_seed, int segment_number,
	const mxArray *normals_,
    const mxArray *point_neighbours, //cell_array_ptr
	vector<int>& point_labels_,
	vector<int>& num_pts_in_segment_,
	float theta,
	float curvature)
{
    const mxArray *cell_element_ptr;  //cell_element_ptr = mxGetCell(cell_array_ptr, index);
	mwSize total_num_of_elements;//total_num_of_elements = mxGetNumberOfElements(cell_element_ptr);  
    double* data;//data = mxGetPr(cell_element_ptr);  
    std::queue<int> seeds;
	seeds.push (initial_seed);
	point_labels_[initial_seed] = segment_number;

	int num_pts_in_segment = 1;

	while (!seeds.empty ())
	{
		int curr_seed;
		curr_seed = seeds.front ();		
		seeds.pop ();		
		size_t i_nghbr = 0;
        cell_element_ptr = mxGetCell(point_neighbours, curr_seed);
        total_num_of_elements = mxGetNumberOfElements(cell_element_ptr); 
        
        data = mxGetPr(cell_element_ptr);  
		while ( i_nghbr < total_num_of_elements )
		{
			int index = static_cast<int>(data[i_nghbr]);   // matlab index starts from 1;         
			if (point_labels_[index] != -1)
			{
				i_nghbr++;
				continue;
			}

			bool is_a_seed = false;
			bool belongs_to_segment = validatePoint (initial_seed, curr_seed, index, is_a_seed,normals_,theta,curvature);

			if (belongs_to_segment == false)
			{
				i_nghbr++;
				continue;
			}

			point_labels_[index] = segment_number;
			num_pts_in_segment++;

			if (is_a_seed)
			{
				seeds.push (index);
			}
			i_nghbr++;
		}// next neighbour
	}// next seed
	return (num_pts_in_segment);
}
void applySmoothRegionGrowingAlgorithm (	
	const mxArray * normals_,
	const mxArray * point_neighbours,
	vector<int>& point_labels_,
	vector<int> & num_pts_in_segment_,
	float theta,
	float curvature	)
{
    double* nor_data;
    mwIndex m;    
    m=mxGetM(normals_);
    nor_data=mxGetPr(normals_);
	int num_of_pts = static_cast<int> (m);
    mexPrintf("num_of_pts:%d\n",num_of_pts);
	point_labels_.resize (num_of_pts, -1);

	std::vector< std::pair<float, int> > point_residual;
	std::pair<float, int> pair;
	point_residual.resize (num_of_pts, pair);
	
	for (int i_point = 0; i_point < num_of_pts; i_point++)
	{			
		point_residual[i_point].first = static_cast<float>(nor_data[3*m+i_point]);        
		point_residual[i_point].second = i_point;
	}
	std::sort (point_residual.begin (), point_residual.end (), comparePair);
	
	int seed_counter = 0;
	int seed = point_residual[seed_counter].second;

	int segmented_pts_num = 0;
	int number_of_segments = 0;
	while (segmented_pts_num < num_of_pts)
	{
		int pts_in_segment;		
		pts_in_segment = growRegion (seed, number_of_segments,normals_,point_neighbours,point_labels_,num_pts_in_segment_,theta,curvature);
		segmented_pts_num += pts_in_segment;       
		num_pts_in_segment_.push_back (pts_in_segment);
		number_of_segments++;

		//find next point that is not segmented yet
		for (int i_seed = seed_counter + 1; i_seed < num_of_pts; i_seed++)
		{
			int index = point_residual[i_seed].second;
			if (point_labels_[index] == -1)
			{
				seed = index;
				break;
			}
		}
	}
}


void assembleRegions (	
	const mxArray* point_neighbours,
    vector<int>& point_labels_,
	vector<int> & num_pts_in_segment_,		        
	int min_pts_per_cluster_,
	int max_pts_per_cluster_   )
{
	int number_of_segments = static_cast<int> (num_pts_in_segment_.size ());
	int number_of_points = static_cast<int> (point_labels_.size ());

	vector<int> segment;
    
	vector<vector<int>> clusters_(number_of_segments, segment);
	for (int i_seg = 0; i_seg < number_of_segments; i_seg++)
	{
		clusters_[i_seg].resize ( num_pts_in_segment_[i_seg], 0);
	}
	std::vector<int> counter;
	counter.resize (number_of_segments, 0);
	for (int i_point = 0; i_point < number_of_points; i_point++)
	{
		int segment_index = point_labels_[i_point];
		if (segment_index != -1)
		{
			int point_index = counter[segment_index];
			clusters_[segment_index][point_index] = i_point;
			counter[segment_index] = point_index + 1;
		}
	}
	//vector<vector<int>> clusters(clusters_.size (), segment);
	for (int i=0;i<point_labels_.size();i++)
	{
		point_labels_[i]=0;
	}
	//auto cluster_iter_input = clusters.begin ();
	int num=0;
	for (auto cluster_iter = clusters_.begin (); cluster_iter != clusters_.end (); cluster_iter++)
	{
		if ((static_cast<int> (cluster_iter->size ()) >= min_pts_per_cluster_) &&
			(static_cast<int> (cluster_iter->size ()) <= max_pts_per_cluster_))
		{
	//		*cluster_iter_input = *cluster_iter;
	//		cluster_iter_input++;
			num++;
			for (auto it=cluster_iter->begin();it!=cluster_iter->end();it++)
			{
				point_labels_[*it]=num;
			}
		}
	}	
	//clusters.resize(num);	
}

void
mexFunction( int nlhs, mxArray *plhs[],
             int nrhs, const mxArray *prhs[] )
{  
  (void) nlhs;     /* unused parameters */    
  double  *pointer; 
  mwSize index;
  vector<int> num_pts_in_segment_;
  vector<int> point_labels_;
  
  float theta;//=10.0 / 180.0 * M_PI;
  float curvature;//=0.03;
  int min_pts;//=50;
  int max_pts;//=20000;  
  theta = static_cast<float>(mxGetScalar(prhs[2]));
  curvature = static_cast<float>(mxGetScalar(prhs[3]));
  min_pts = static_cast<int>(mxGetScalar(prhs[4]));
  max_pts = static_cast<int>(mxGetScalar(prhs[5]));
  
  mexPrintf("Region growing parameters:\n");
  mexPrintf("theta_threshold: %f\n",theta);
  mexPrintf("curvature_threshold: %f\n",curvature);
  mexPrintf("min_pts_per_cluster: %d\n",min_pts);
  mexPrintf("max_pts_per_cluster: %d\n",max_pts);
  
  applySmoothRegionGrowingAlgorithm (prhs[0],prhs[1],point_labels_,num_pts_in_segment_,theta,curvature);
  assembleRegions (	prhs[1],point_labels_,num_pts_in_segment_,min_pts,max_pts );
  plhs[0] = mxCreateNumericMatrix(point_labels_.size(), 1, mxDOUBLE_CLASS, mxREAL);
  pointer = mxGetPr(plhs[0]);  
  /* Copy data into the mxArray */
  for ( index = 0; index < point_labels_.size(); index++ )
  {
      pointer[index] = point_labels_[index];
  }
 }
