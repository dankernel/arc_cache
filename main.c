/**
 * =====================================================================================
 *
 *          @file:  main.c
 *         @brief:  Main file
 *
 *          @date:  2014년 10월 30일 19시 48분 57초
 *        @author:  Jun-Hyung Park (), google@dankook.ac.kr
 *        Version:  1.0
 *
 *       Revision:  none
 *       Compiler:  gcc
 *   organization:  Dankook Univ.
 *    Description:  :
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "./dkh/arc.c"

/**
 * Main function
 * @return error code
 */
int main(int argc, char *argv[]){

  FILE *fp;

  srandom(time(NULL));

  /* Set workload file */
  fp = open_workload(argv[1]);
  if (!fp)
    printf("FAIL open\n");
  else
    printf("OK open\n");

  /* Read MAIN function */
  /* read_workload(fp, atol(argv[2]) * 1024 * 1024); */
  read_workload(fp, atol(argv[2]) * 1024 * 1024);

end:
  fclose(fp);
  return 0;

}
