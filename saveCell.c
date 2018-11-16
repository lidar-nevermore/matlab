#include <stdlib.h>
#include <time.h>
#include "mex.h"

static double taketime()
{
    return (double)(clock()) / CLOCKS_PER_SEC;
}
static void exit_with_help()
{
    mexPrintf("Function: save matlab 2D cell to txt file;\n");
    mexPrintf("Usage: saveCell ( data, filename );\n");
}

static void fake_answer(int nlhs, mxArray *plhs[])
{
    int i;
    for(i=0;i<nlhs;i++)
        plhs[i] = mxCreateDoubleMatrix(0, 0, mxREAL);
}

void saveCell(const mxArray* data, const char *filename)
{
    FILE *fp = fopen(filename,"w");
    int num_of_records,i,j;
    double *line_data, start_time;
    mxArray *cell_ptr;
    mwSize num_of_elements_in_cell;
    start_time = taketime();
    if(fp == NULL)
    {
        mexPrintf("can't open output file %s\n",filename);
        return;
    }
    num_of_records = mxGetM(data);
    for (i = 0; i < num_of_records; ++i)
    {
        cell_ptr = mxGetCell(data, i);
        num_of_elements_in_cell = mxGetNumberOfElements(cell_ptr);
        line_data = mxGetPr(cell_ptr);        
        for (j = 0; j < num_of_elements_in_cell; ++j)
        {
            fprintf(fp,"%g ", line_data[j]);
        }
        fprintf(fp,"\n");
    }
    mexPrintf("total time: %g sec for %d records\n", taketime() - start_time, num_of_records);
    fclose(fp);
    return;
}

void mexFunction( int nlhs, mxArray *plhs[],
        int nrhs, const mxArray *prhs[] )
{
    if(nlhs > 0 || nrhs != 2 || !mxIsCell(prhs[0])|| !mxIsChar(prhs[1]))
    {
        exit_with_help();
        fake_answer(nlhs, plhs);
        return;
    }
    saveCell(prhs[0],mxArrayToString(prhs[1]));
    return;
}
