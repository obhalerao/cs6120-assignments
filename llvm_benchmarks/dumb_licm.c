#include <stdio.h>

int main(){
  int a = 7;
  int b = 9;
  do{
    a = 6;
    b--;
  }while(b > 0);
  printf("%d\n", a);
  return 0;
}