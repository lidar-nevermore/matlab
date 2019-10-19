//mex radiusSearch.cpp -I/usr/include /usr/lib/x86_64-linux-gnu/liblz4.so
//mex radiusSearch.cpp -IFLANN\include -ILZ4\include LZ4\lib\lz4.lib FLANN\lib\flann_cuda_s.lib ...
//  'C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/x64/cuda.lib' ...
//  'C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0/lib/x64/cudart.lib'


#define FLANN_USE_CUDA

#include <cstdlib>
#include <ctime>
#include "mex.h"
#include <flann/flann.hpp>
#include <vector>
#include <memory>
#include <functional>
using namespace std;

typedef float typeT; // only float is supported.

typedef shared_ptr<flann::Matrix<typeT>> flannMatrixPtr;
static double taketime()
{
	return (double)(clock()) / CLOCKS_PER_SEC;
}
static void fake_answer(int nlhs, mxArray *plhs[])
{
	int i;
	for (i = 0; i<nlhs; i++)
		plhs[i] = mxCreateDoubleMatrix(0, 0, mxREAL);
	string errormsg = "Function:find all the X points that are within distance radius of the Y points;\n";
	errormsg += "Usage: [idx,dist]=radiusSearchGPU(X,Y,radius);\n";
	errormsg += "       [idx,dist]=radiusSearchGPU(X,radius);\n";
	errormsg += "       idx=radiusSearchGPU(X,Y,radius);\n";
	errormsg += "       idx=radiusSearchGPU(X,radius);";
	mexErrMsgTxt(errormsg.c_str());
}
template <typename T>
mxArray* vec2mxArray(const vector<vector<T>>& vecdata)
{
	int num_of_records = vecdata.size();
	int num_elements = vecdata[0].size();
	mxArray* mxdata = mxCreateDoubleMatrix(num_of_records, num_elements, mxREAL);
	for (int i = 0; i < num_of_records; i++)
		for (int j = 0; j < num_elements; j++)
		{
			mxdata[i + num_of_records*j] = vecdata[i][j];
		}
	return mxdata;
}

template <typename T, class function>
mxArray* vec2mxCell(const vector<vector<T>>& vecdata, const function& func)
{
	int num_of_records = vecdata.size();
	mxArray* mxCell = mxCreateCellMatrix(num_of_records, 1);
	for (size_t i = 0; i < num_of_records; i++)
	{
		int num_elements = vecdata[i].size();
		mxArray* mxline_data = mxCreateDoubleMatrix(1, num_elements, mxREAL);
		double* temp = mxGetPr(mxline_data);
		for (int j = 0; j<num_elements; j++)
			temp[j] = func(vecdata[i][j]);
		mxSetCell(mxCell, i, mxline_data);
	}
	return mxCell;
}

flannMatrixPtr mxArray2flannMatrix(const mxArray* mxdata)
{
	size_t num_of_records = mxGetM(mxdata);
	size_t num_elements = mxGetN(mxdata);
	double* mxdata_ = mxGetPr(mxdata);
	//flann::Matrix<double> dataset(new double[num_of_records*num_elements], num_of_records, num_elements);
	flannMatrixPtr dataset = std::make_shared<flann::Matrix<typeT>>(new typeT[num_of_records*num_elements], num_of_records, num_elements);
	for (int i = 0; i < num_of_records; i++)
		for (int j = 0; j < num_elements; j++)
		{
			(*dataset)[i][j] = mxdata_[i + j*num_of_records];
		}
	return dataset;
}
void mexFunction(int nlhs, mxArray *plhs[],
	int nrhs, const mxArray *prhs[])
{
	if (nrhs<2 || nrhs>3 || nlhs>2 || !mxIsNumeric(prhs[0]) || !mxIsNumeric(prhs[nrhs - 2]) || !mxIsScalar(prhs[nrhs - 1]))
	{
		fake_answer(nlhs, plhs);
		return;
	}
	double sqr_radius_d = pow(mxGetScalar(prhs[nrhs - 1]), 2);
	float sqr_radius_f = (float)sqr_radius_d;
	if (sqr_radius_d != sqr_radius_f)
	{
		mexPrintf("Warning:precise of input floating-point radius...\n");
		mexPrintf("Input square_radius: %f; Used square_radius: %f.\n", sqr_radius_d, sqr_radius_f);
	}
	double start_time = taketime();
	flannMatrixPtr dataset, query;
	if (nrhs == 3)
	{
		dataset = mxArray2flannMatrix(prhs[0]);
		query = mxArray2flannMatrix(prhs[1]);
	}
	else
	{
		dataset = mxArray2flannMatrix(prhs[0]);
		query = dataset;
	}

	std::vector<std::vector<int>>indices;
	std::vector<std::vector<typeT>>dists;

	flann::Index< flann::L2_Simple<typeT> > flann_index(*dataset, flann::KDTreeCuda3dIndexParams(4));
	flann_index.buildIndex();
	flann_index.radiusSearch(*query, indices, dists, sqr_radius_f, flann::SearchParams(8));

	if (nlhs == 2)
	{
		plhs[0] = vec2mxCell(indices, [](double a) {return a + 1; });//matlab index starts from 1
		indices.clear();
		plhs[1] = vec2mxCell(dists, [](double a) {return sqrt(a); });
		dists.clear();
	}
	else
	{
		plhs[0] = vec2mxCell(indices, [](double a) {return a + 1; });
		indices.clear();
	}
	if (nrhs == 3)
	{
		delete[]dataset->ptr();
		delete[]query->ptr();
	}
	else
	{
		delete[]dataset->ptr();
	}
	mexPrintf("total time: %f sec\n", taketime() - start_time);
	return;
}
