

#include<stdio.h>


void test(int num)
{

    
    
    if (num % 1024 == 0 ) //starting at the begining at the block for what copies
    {
        printf("first one %d\n",num);
    }
    else if(num % 512== 0 && num % 1024 != 0 ) //starting halfway though block
    {
        printf("sec one %d\n",num);

    }
    else if(num % 256 == 0  && (((num-256) % 1024)!= 0 )) //starting at last quarter 768
    {
        printf("third one %d\n",num);

    }
    else //starting at second quarter 768
    {
        printf("fourth one %d\n",num);

    }

}

int main()
{

    int num = 768;
    test(num);

}
