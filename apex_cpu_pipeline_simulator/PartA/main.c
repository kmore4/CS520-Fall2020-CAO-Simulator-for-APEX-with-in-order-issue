/*
 * main.c
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


#include "apex_cpu.h"

int
main(int argc, char const *argv[])
{
    APEX_CPU *cpu;

    fprintf(stderr, "APEX CPU Pipeline Simulator v%0.1lf\n", VERSION);

    if (argc < 2 || argc > 4)
    {
        fprintf(stderr, "APEX_Help: Usage %s <input_file>\n", argv[0]);
        exit(1);
    }

    cpu = APEX_cpu_init(argv[1]);
    if(argc > 2)
    {
      const char* function_name = argv[2];
      //printf("%s\n",function_name);
      //function_name = tolower(function_name);
      printf("%s\n",function_name);
      printf("cycles=%s\n",argv[3]);
      int cycles = atoi(argv[3]);
      if (!cpu)
      {
          fprintf(stderr, "APEX_Error: Unable to initialize CPU\n");
          exit(1);
      }

      if(argc == 4)
      {
        if(strcmp(function_name, "simulate") == 0)
        {
          printf("Inside simulate and cycles = %d\n",cycles);
            APEX_cpu_simulate(cpu,cycles);
            APEX_cpu_stop(cpu);
            return 0;
        }

        if(strcmp(function_name, "display") == 0)
        {
          APEX_cpu_display(cpu);
          APEX_cpu_stop(cpu);
          return 0;
        }
      }
    }
    else
    {
      APEX_cpu_run(cpu);
      APEX_cpu_stop(cpu);
      return 0;
    }
}
