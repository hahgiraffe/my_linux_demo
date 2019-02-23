#include <stdio.h>
int main(){
    int num=9;
    float *ptr=&num;
    printf("(1) int num is %d\n",num);//(1) int num is 9
    printf("(2) float num is %f\n",*ptr);//(2) float num is 0.000000
    *ptr=9.0;
    printf("(3) after changed, int num is %d\n",num);//(3) after changed, int num is 1091567616
    printf("(4) after changed, float num is %f\n",*ptr);//(4) after changed, float num is 9.000000
    return 0;
}