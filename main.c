
#include <stdio.h>


void usage(char* name, FILE *out) {
  fprintf(out, "\n");
  fprintf(out, "%s start|stop|status|pat\n", name);
  fprintf(out, "    -d device: Device file to open (default: /dev/watchdog)\n");
  fprintf(out, "    -t timeout: Reboot after this many seconds with no pat (default: 30)\n");
  fprintf(out, "\n");
}



int main(int argc, char** argv) {

  // TODO
  printf("Program not yet implemented\n");

  usage(argv[0], stdout);


  return 0;
}
