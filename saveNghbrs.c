#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mex.h"


void exit_with_help()
{
    mexPrintf("Function: save neighbours to txt file, ID starts from 1;\n");
	mexPrintf("Usage: saveNghbrs ( neighbours_cell, filename );\n");
}

static void fake_answer(int nlhs, mxArray *plhs[])
{
	int i;
	for(i=0;i<nlhs;i++)
		plhs[i] = mxCreateDoubleMatrix(0, 0, mxREAL);
}

void saveNghbrs(const mxArray* point_neighbours, const char *filename)
{
	FILE *fp = fopen(filename,"w");    
	int num_of_pts,i,j;	
	double *nghbrs;
	mxArray *cell_ptr;
    mwSize num_of_elements_in_cell;
    
	if(fp == NULL)
	{        
		mexPrintf("can't open output file %s\n",filename);			
		return;
	}	
    num_of_pts = mxGetM(point_neighbours);   
    for (i = 0; i < num_of_pts; ++i)
	{
        cell_ptr = mxGetCell(point_neighbours, i);
		num_of_elements_in_cell = mxGetNumberOfElements(cell_ptr);
        nghbrs = mxGetPr(cell_ptr);
		for (j = 0; j < num_of_elements_in_cell; ++j)
		{
            fprintf(fp,"%d ", (int)nghbrs[j]);		
        }	
        fprintf(fp,"\n");
	}   
	fclose(fp);
	return;
}

void mexFunction( int nlhs, mxArray *plhs[],
		int nrhs, const mxArray *prhs[] )
{
	if(nlhs > 0 || nrhs != 2 || !mxIsCell(prhs[0]) || !mxIsChar(prhs[1]))
	{
		exit_with_help();
		fake_answer(nlhs, plhs);        
		return;
    }   
    saveNghbrs(prhs[0],mxArrayToString(prhs[1]));
}
