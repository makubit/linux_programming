#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

void display_help()
{
  printf("Instructions:\n -> -#<int> optional(:<int>) mandatory parameter, number of blocks we want to get from producer\n");
  printf(" -> -s <float> | -r <float> mandatory, time\n");
  printf(" -> [<addr>:]port mandatory,  port address\n");
}

int convert_to_int(char* first_p, char* second_p)
{
  //convert to int
  int cnt = strtol(first_p, NULL, 0);
  if(cnt <= 0)
  {
    printf(" <cnt> has to be >= 0\n");
    display_help();
    exit(EXIT_FAILURE);
  }
  if(second_p)
  {
      int cnt_tmp = strtol(second_p, NULL, 0);
      if(cnt > cnt_tmp)
      {
          perror("-# error: second parameter cannot be lesser than first parameter\n");
          exit(EXIT_FAILURE);
      }

      srand(time(NULL));
      return (rand() % (cnt_tmp + 1 - cnt)) + cnt;
    }
    return cnt;
}

float convert_to_float(char* first_p, char* second_p)
{
  //convert to float
  float dly = strtod(first_p, NULL);
  if(dly <= 0)
  {
    printf(" <dly> has to be >= 0\n");
    display_help();
    exit(EXIT_FAILURE);
  }
  if(second_p)
  {
    float dly_tmp = strtod(second_p, NULL);
    if(dly > dly_tmp)
    {
        perror("-# error: second parameter cannot be lesser than first parameter\n");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));
    return ((float)rand()/RAND_MAX + (rand() % ((int)dly_tmp + 1 - (int)dly)) + (int)dly);
  }
  return dly;
}

char* convert_address(char* addr)
{
  char* first_p = strtok(addr, "port");
  first_p = strtok(first_p, "[");
  first_p = strtok(first_p, ":]");

  if(first_p == NULL)
  {
    return "localhost";
  }

  return first_p;
}


int main(int argc, char* argv[])
{
    int c;
    char* tempbuff = NULL;
    int cnt = 0;
    float dly = 0;
    char* first_p = NULL;
    char* second_p = NULL;
    char* port_addr = NULL;
    while((c = getopt(argc, argv, "#:r:s:")) != -1)
      {
        switch(c)
        {
            case '#':
                tempbuff = malloc(sizeof(optarg));
                strcpy(tempbuff, optarg);
                first_p = strtok(tempbuff, ":");
                second_p = strtok(NULL, ":");

                //convert to int
                cnt = convert_to_int(first_p, second_p);

                break;
            case 'r':
                if(dly > 0)
                {
                    printf(" -r error, cannot use both -r and -s\n");
                    display_help();
                    exit(EXIT_FAILURE);
                }

                tempbuff = malloc(sizeof(optarg));
                strcpy(tempbuff, optarg);
                first_p = strtok(tempbuff, ":");
                second_p = strtok(NULL, ":");

                //convert to float
                dly = convert_to_float(first_p, second_p);
                printf("%lf\n", dly);

                break;
            case 's':
                if(dly > 0)
                {
                    printf(" -r error, cannot use both -r and -s\n");
                    display_help();
                    exit(EXIT_FAILURE);
                }

                tempbuff = malloc(sizeof(optarg));
                strcpy(tempbuff, optarg);
                first_p = strtok(tempbuff, ":");
                second_p = strtok(NULL, ":");

                //convert to float
                dly = convert_to_float(first_p, second_p);

                break;
            case '?':
                break;
        }
      }

      if((cnt == 0) || (dly == 0) || (argv[optind] == NULL))
      {
        printf(" Mandatory parameters: <cnt>, <dly>, [<addr>]:port\n");
        display_help();
        exit(EXIT_FAILURE);
      }

      port_addr = convert_address(argv[optind]);

      char* t= "localhost";
      int tmp = 0;
      for(int i=0; i< strlen(t); i++)
      {
        tmp += t[i];
      }

      printf("%d", tmp);
      //int tmp = strtol(t, NULL, 0);








    return 0;
}
