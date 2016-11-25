/**
 * @file test.c
 * @brief simple (useless) "overwrite" attribute framework test
 *
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */

#include FW_WRAPPER

int add_test(int a, int b) {
  //printf2("hello world");
  return a+b;   
}

__attribute__ ((overwrite ("printf")))
int test(int a, int b) {
  return add_test(a, b); 
}
