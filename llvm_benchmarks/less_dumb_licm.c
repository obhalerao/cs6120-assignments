#include <stdio.h>

int main(){
  int v;
  scanf("%i", &v);
  int a = 7;
  int b = 9;
  do{
    a = 5*v;
    b--;
  }while(b > 0);
  printf("%d\n", a);
  return 0;
}