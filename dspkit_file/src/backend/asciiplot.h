/** @file A simple ASCII-plot function  */

#ifndef ASCIIPLOT_H_
#define ASCIIPLOT_H_

#include <stddef.h>

struct asciiplot_s {
	size_t cols;			//!<- Number of columns to print (length of x-axis)
	size_t rows;			//!<- Number of rows to print (length of y-axis)
	float ** const xdata;	//!<- Pointer to pointers of x-data. Set to array of NULL's to imply xdata = {0, 1, 2, 3, ...}
	float ** const ydata;	//!<- Pointer to pointers of y-data
	size_t * data_len;		//!<- Pointer to array of lengths of xdata's and ydata's
	size_t num_plots;		//!<- Number of plots to draw (length of xdata, ydata, data_len)
	char * const xlabel;	//!<- String to print as x-axis label (or NULL to disable)
	char * const ylabel;	//!<- String to print as y-axis label (or NULL to disable)
	char * const title;		//!<- String to print as title (or NULL to disable)
	char * const markers;	//!<- Array of length num_plot with characters to use as markers for plots
	char ** const legend;	//!<- Array of strings to print as legend (or NULL to disable)
	float * axis;			//!<- Point to array to control axis extents [xmin, xmax, ymin, ymax]. Set any element to NaN to enable auto-scaling for the relevant element(s). MUST BE AN ARRAY OF LENGTH 4!
	int label_prec;		//!<- Set to the number of decimals to include for the axis extent labels. 2-5 are typical values.
};

/** @brief Draws a plot to stdout */
void asciiplot_draw(struct asciiplot_s * const plot);


#endif /* ASCIIPLOT_H_ */
