#include<stdio.h>
#include<stdlib.h>




int main()
{
    int n;
    float x,y;
    srand(time(NULL));   // Initialization, should only be called once.

    printf("Enter amount number to create: ");
    scanf("%d", &n);

    FILE *fp = fopen("output.dat","w+");
    
    for (size_t i = 0; i < n; i++)
    {
        //  r = rand() % 99; 
        x = ((float)rand()/(float)(RAND_MAX)) * n;
        y= ((float)rand()/(float)(RAND_MAX)) * n;
        fprintf(fp, "%lf %lf\n", x,y); 
    }
   
    
    fclose(fp);

    return 0;

}