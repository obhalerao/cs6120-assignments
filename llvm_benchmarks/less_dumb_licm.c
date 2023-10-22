#include <stdio.h>
#include <limits.h>

int main(){
  int v = 12;
  scanf("%i", &v);
  int w = v;
  int a = 0;
  long long reps = ((long) 1000) * 1000 * 1000;
  long long dumb = 13992847;
  int ans = 0;
  int modulus = 3;
  scanf("%i", &modulus);
  do{
    a += dumb * w;
    ans += w;
    reps -= 1;
  }while(reps > 0);
  printf("%d\n", ans);
  return 0;
}
