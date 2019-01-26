#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>


int main(int argc, char* argv[])
{
    int c;
    char* tempbuff = NULL;
    int cnt, cnt_tmp;
    while((c = getopt(argc, argv, "#:r:s:")) != -1)
        switch(c)
        {
            case '#':
                tempbuff = malloc(sizeof(optarg));
                strcpy(tempbuff, optarg);
                char* first_p = strtok(tempbuff, ":");
                char* second_p = strtok(NULL, ":");
                
                //convert to int
                cnt = strtol(first_p, NULL, 0);
                if(second_p)
                {
                    cnt_tmp = strtol(second_p, NULL, 0);
                    if(cnt > cnt_tmp)
                    {
                        perror("-# error: second parameter cannot be lesser than first parameter\n");
                        exit(EXIT_FAILURE);
                    }

                    srand(time(NULL));
                    cnt = (rand() % (cnt_tmp + 1 - cnt)) + cnt;
                    printf("%d\n", cnt);
                }
                break;
            case 'r':
                break;
            case 's':
                break;
            case '?':
                break;

        }



    return 0;
}
