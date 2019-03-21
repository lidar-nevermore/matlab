#include <time.h>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include "mex.h"
using namespace std;
static double taketime()
{
    return (double)(clock()) / CLOCKS_PER_SEC;
}
static void exit_with_help()
{
    mexPrintf("Function: read matlab 2D cell from txt file;\n");
    mexPrintf("Usage: data = readCell ( filename );\n");
}
static void fake_answer(int nlhs, mxArray *plhs[])
{    
    for (int i = 0; i < nlhs; i++)
        plhs[i] = mxCreateDoubleMatrix(0, 0, mxREAL);
}
int split(const string &line,double* line_data)
{
	int num_of_elements=0;
	string dlm=" ";	
	size_t current =line.find_first_not_of(dlm,0);
	size_t next =line.find_first_of(dlm,current);    
    while (current!=string::npos)
    {            
        line_data[num_of_elements] = atof(line.data()+current);
        //line_data[num_of_elements] = atof(line.substr(current,next-current).c_str());
        num_of_elements++;
        current =line.find_first_not_of(dlm,next);
		next =line.find_first_of(dlm,current);
    }      	
	return num_of_elements;
}
static void readCell(const char *filename, int nlhs, mxArray *plhs[])
{    
    size_t max_line_len;
    int num_of_records, i;    
    double *line_data, start_time,value;
    mwSize num_of_elements;
    mxArray *mxline_data, *cell_array_ptr;

    start_time = taketime();
    ifstream ifs(filename);
    string str;
    str.reserve(1024);
    if (!ifs.is_open())
    {
        mexPrintf("can't open input file: %s\n", filename);
        return;
    }
    num_of_records = 0;
    max_line_len = 0;
    while (getline(ifs,str))
    {        
        max_line_len=max(max_line_len,str.size());
        num_of_records++;
    }
    ifs.clear();
    ifs.seekg(0);
    cell_array_ptr = mxCreateCellMatrix(num_of_records, 1);
    line_data = new double[max_line_len / 2 ];   
    for (i = 0; i < num_of_records; i++)
    {        
        getline(ifs,str);
        if (str.empty())
        {
            mexPrintf("Empty line at line %d\n", i + 1);
            fake_answer(nlhs, plhs);
            return;
        } 
        num_of_elements=split(str,line_data);       
        mxline_data = mxCreateDoubleMatrix(1, num_of_elements, mxREAL);        
        memcpy_s(mxGetPr(mxline_data), num_of_elements * sizeof(double), line_data, num_of_elements * sizeof(double));
        mxSetCell(cell_array_ptr, i, mxline_data);        
    }
    
    plhs[0] = cell_array_ptr;
    delete[] line_data;
     
    mexPrintf("total time: %g sec for %d records\n", taketime() - start_time, num_of_records);
    ifs.close();
    return;
}

void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
{
    if (nlhs != 1 || nrhs != 1 || !mxIsChar(prhs[0]))
    {
        exit_with_help();
        fake_answer(nlhs, plhs);
        return;
    }
    readCell(mxArrayToString(prhs[0]), nlhs, plhs);
    return;
}
