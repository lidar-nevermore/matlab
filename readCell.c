#include <stdlib.h>
#include <time.h>
#include "mex.h"

static char *line;
static int max_line_len;

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
    int i;
    for (i = 0; i < nlhs; i++)
        plhs[i] = mxCreateDoubleMatrix(0, 0, mxREAL);
}
static char *readline(FILE *input)
{
    int len;
    if (fgets(line, max_line_len, input) == NULL)
        return NULL;
    while (strrchr(line, '\n') == NULL)
    {
        max_line_len *= 2;
        line = (char *)realloc(line, max_line_len);
        len = (int)strlen(line);
        if (fgets(line + len, max_line_len - len, input) == NULL)
            break;
    }
    len = (int)strlen(line);
    if (line[len - 1] == '\n')
        line[len - 1] = '\0';
    return line;
}

void readCell(const char *filename, int nlhs, const mxArray *plhs[])
{
    FILE *fp = fopen(filename, "r");
    int num_of_records, i;
    char *endptr, *val;
    double *line_data, start_time, value;
    mwSize num_elements;
    mxArray *mxline_data, *cell_array_ptr;

    start_time = taketime();
    if (fp == NULL)
    {
        mexPrintf("can't open input file %s\n", filename);
        return;
    }
    num_of_records = 0;
    max_line_len = 1024;
    line = (char *)malloc(max_line_len * sizeof(char));

    while (readline(fp) != NULL)
    {
        num_of_records++;
    }    
    rewind(fp);

    cell_array_ptr = mxCreateCellMatrix(num_of_records, 1);
    line_data = (double *)malloc( max_line_len / 2 * sizeof(double));

    for (i = 0; i < num_of_records; i++)
    {
        readline(fp);
        val = strtok(line, " "); // value1
        if (val == NULL)
        {
            mexPrintf("Empty line at line %d\n", i + 1);
            fake_answer(nlhs, plhs);
            return;
        }
        line_data[0] = strtod(val, &endptr);
        num_elements = 1;
        while (1)
        {
            val = strtok(NULL, " "); // value 2:end
            if (val == NULL)
                break;
            errno = 0;
            value = strtod(val, &endptr);
            if (endptr == val || errno != 0 || *endptr != '\0')
            {
                mexPrintf("Wrong input format at line %d,column %d\n", num_of_records + 1, num_elements + 1);
                fake_answer(nlhs, plhs);
                return;
            }
            line_data[num_elements] = value;
            num_elements++;
        }
        mxline_data = mxCreateDoubleMatrix(1, num_elements, mxREAL);        
        memcpy_s(mxGetPr(mxline_data), num_elements * sizeof(double), line_data, num_elements * sizeof(double));
        mxSetCell(cell_array_ptr, i, mxline_data);
    }
    plhs[0] = cell_array_ptr;
    free(line);
    free(line_data);
    mexPrintf("total time: %g sec for %d records\n", taketime() - start_time, num_of_records);
    fclose(fp);
    return;
}

void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
{
    if (nlhs > 1 || nrhs > 1 || !mxIsChar(prhs[0]))
    {
        exit_with_help();
        fake_answer(nlhs, plhs);
        return;
    }
    readCell(mxArrayToString(prhs[0]), nlhs, plhs);
    return;
}
