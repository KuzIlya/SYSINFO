#include <stdio.h>
#include <stdlib.h>

FILE *init_gp() {
  FILE *output;
  output = popen("gnuplot", "w");

  if (!output)
    {
      fprintf(stderr, "Gnuplot failed to open!!!\n");
      exit(EXIT_FAILURE);
    }

  return output;
}


void close_gp(FILE *output) {
  if (pclose(output) != 0)
    {
      fprintf(stderr,
               "...Error was encountered in running Gnuplot!!!\n");
      exit(EXIT_FAILURE);
    }
}


void write_gp(FILE *stream, char *command) {
  fprintf(stream, "%s\n", command);
  fflush(stream);

  if (ferror(stream))
    {
      fprintf (stderr, "...Output to stream failed!!!\n");
      exit (EXIT_FAILURE);
    }

}