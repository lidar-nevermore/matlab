// copy all .hpp and .cpp files
// mex matlab_readlas.cpp LASlib.lib
#include <ctime>
#include <cstdio>
#include "lasreader.hpp"
#include "mex.h"

static double taketime()
{
	return (double)(clock()) / CLOCKS_PER_SEC;
}
static void exit_with_help()
{
    mexPrintf("Function: read lasfile,format: xyzirncuRGB\n");
    mexPrintf("Usage: data = matlab_readlas ( filename );\n");
}
void mexFunction(int nlhs, mxArray *plhs[],
				 int nrhs, const mxArray *prhs[])
{
    if (nlhs != 1 || nrhs != 1 || !mxIsChar(prhs[0]))
    {
        exit_with_help();        
        return;
    }
    double *pointer;
    char *filename;
    double start_time = 0.0;
    mwSize index;
    int num_pts;
    LASreadOpener lasreadopener;
    filename = mxArrayToString(prhs[0]);
    lasreadopener.set_file_name(filename);
    start_time = taketime();
    if (!lasreadopener.active())
    {
    	mexErrMsgIdAndTxt("MATLAB:cppfeature:invalidNumInputs", "ERROR: no input specified\n");
    }
    LASreader *lasreader = lasreadopener.open();
    if (lasreader == 0)
    {
    	mexErrMsgIdAndTxt("MATLAB:cppfeature:invalidNumInputs", "ERROR: could not open lasreader\n");
    }
    num_pts = lasreader->header.number_of_point_records;
    plhs[0] = mxCreateNumericMatrix(num_pts, 11, mxDOUBLE_CLASS, mxREAL);
    nlhs = 1;
    pointer = mxGetPr(plhs[0]);

    index = 0;
    while (lasreader->read_point())
    {
    	// read the point: xyzirncuRGB/
    	pointer[index] = lasreader->point.get_x();
    	pointer[index + num_pts] = lasreader->point.get_y();
    	pointer[index + 2 * num_pts] = lasreader->point.get_z();
    	pointer[index + 3 * num_pts] = lasreader->point.intensity;
    	pointer[index + 4 * num_pts] = lasreader->point.return_number;
    	pointer[index + 5 * num_pts] = lasreader->point.number_of_returns;
    	pointer[index + 6 * num_pts] = lasreader->point.classification;
    	pointer[index + 7 * num_pts] = lasreader->point.user_data;
    	pointer[index + 8 * num_pts] = lasreader->point.rgb[0];
    	pointer[index + 9 * num_pts] = lasreader->point.rgb[1];
    	pointer[index + 10 * num_pts] = lasreader->point.rgb[2];
    	++index;
    }
    mexPrintf("total time: %g sec for %ld points, format: xyzirncuRGB\n", taketime() - start_time, index);
    return;
}
