#ifndef GNUPLOT_H
#define GNUPLOT_H

FILE *init_gp(void);
void close_gp(FILE *output);
void write_gp(FILE *stream, char *command);

#endif