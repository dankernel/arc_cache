/*
 * =====================================================================================
 *
 *       Filename:  print_mag.h
 *
 *    Description:  print mes
 *
 *        Version:  1.0
 *        Created:  2014년 08월 18일 02시 56분 28초
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         author:   (), 
 *   organization:  
 *
 * =====================================================================================
 */

/** print msg OK or FAIL */
#define print_f(str) printf("[FAIL] %s \n", str);
#define print_o(str) printf("[ OK ] %s \n", str);

/** negative number is FAIL, else OK */
#define err_test(val, str) \
  do{\
    (val < 0) ?   \
    printf("[FAIL] %s : %d fail... \n", str, val) :    \
    printf("[ OK ] %s : %d ok... \n", str, val);    \
  } while(0)

