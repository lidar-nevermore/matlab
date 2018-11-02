// copy all .hpp and .cpp files
// mex matlab_writelas.cpp -lLASlib.lib
#include <ctime>
#include <cstdio>
#include <cmath>
#include "laswriter.hpp"
#include "mex.h"

static double taketime()
{
    return (double)(clock()) / CLOCKS_PER_SEC;
}
void mexFunction(int nlhs, mxArray *plhs[],
        int nrhs, const mxArray *prhs[])
{
    (void)nlhs; /* unused parameters */
    (void)plhs;
    double *pointer;
    char *filename;
    double start_time = 0.0,sum_x=0.0,sum_y=0.0,sum_z=0.0;
    int num_pts,index;
    LASwriteOpener laswriteopener;
    
    pointer = mxGetPr(prhs[0]);
    num_pts=mxGetM(prhs[0]);
    if (mxGetN(prhs[0])!=11)
    {
        mexErrMsgIdAndTxt("MATLAB:cppfeature:invalidNumInputs",
                "Invalid input, required format: xyzirncuRGB!");
    }
    filename = mxArrayToString(prhs[1]);
    laswriteopener.set_file_name(filename);
    start_time = taketime();
    if (!laswriteopener.active())
    {
        mexErrMsgIdAndTxt("MATLAB:cppfeature:invalidNumInputs",
                "no output specified!");
    }
    
    for (index = 0; index < num_pts; index++)
    {
        sum_x+=pointer[index];
        sum_y+=pointer[index + num_pts];
        sum_z+=pointer[index + 2*num_pts];
    }
    // init header
    LASheader lasheader;
    lasheader.x_scale_factor = 0.01;
    lasheader.y_scale_factor = 0.01;
    lasheader.z_scale_factor = 0.01;
    lasheader.x_offset = round(sum_x/num_pts);
    lasheader.y_offset = round(sum_y/num_pts);
    lasheader.z_offset = round(sum_z/num_pts);
    lasheader.point_data_format = 2;
    lasheader.point_data_record_length = 26;
    
    // init point
    LASpoint laspoint;
    laspoint.init(&lasheader, lasheader.point_data_format, lasheader.point_data_record_length, 0);
    // open laswriter
    LASwriter* laswriter = laswriteopener.open(&lasheader);
    if (laswriter == 0)
	{
		mexErrMsgIdAndTxt("MATLAB:cppfeature:invalidNumInputs", "ERROR: could not open laswriter\n");
	}
    // write points, format: xyzirncuRGB
    for (index = 0; index < num_pts; index++)
    {
        laspoint.set_x(pointer[index]);
        laspoint.set_y(pointer[index + num_pts]);
        laspoint.set_z(pointer[index + 2*num_pts]);
        laspoint.set_intensity((U16)pointer[index + 3*num_pts]);
        laspoint.set_return_number((U8)pointer[index + 4*num_pts]);
        laspoint.set_number_of_returns((U8)pointer[index + 5*num_pts]);
        laspoint.set_classification((U8)pointer[index + 6*num_pts]);
        laspoint. set_user_data((U8)pointer[index + 7*num_pts]);
        laspoint.rgb[0]= (U16)pointer[index + 8*num_pts];
        laspoint.rgb[1]= (U16)pointer[index + 9*num_pts];
        laspoint.rgb[2]= (U16)pointer[index + 10*num_pts];
        
        laswriter->write_point(&laspoint);
        laswriter->update_inventory(&laspoint);
    }
    laswriter->update_header(&lasheader, TRUE);
    laswriter->close();
    mexPrintf("total time: %g sec for %ld points, format: xyzirncuRGB\n", taketime() - start_time, index);
    return;
}