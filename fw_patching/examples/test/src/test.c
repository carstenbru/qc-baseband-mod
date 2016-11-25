#include FW_WRAPPER

__attribute__ ((overwrite ("add"))) int add_test(int a, int b) {
  //printf2("hello world");
  return a+b;   
}

__attribute__ ((overwrite ("printf2")))
int test(int a, int b) {
  return add_test(a, b); 
}
