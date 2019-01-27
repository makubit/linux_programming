#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

void display_help()
{
  printf("Instructions:\n -> -r <char*> mandatory, path to ganerate raports\n");
  printf(" -> -t <int> mandatory, <int> has to be in range(1, 8)\n");
  printf(" -> [<addr>:]port mandatory,  port address\n");
}

float convert_to_float18(char* int_buff)
{
  char* pEnd = NULL;
  float temp = strtod(int_buff, &pEnd);

  if((temp <=0) || (temp >8) || (*pEnd != '\0'))
  {
    printf(" converting to int error: -t parameter has to be an integer in range(1, 8)\n");
    display_help();
    exit(EXIT_FAILURE);
  }

  //convert min/96 to secs
  //1temp --- 60/96sek
  return (temp*60/96);
}

int convert_address(char* addr)
{
  char* first_p = strtok(addr, "port");
  first_p = strtok(first_p, "[");
  first_p = strtok(first_p, ":]");

  int temp = 0;
  if(first_p == NULL)
  {
    char* loc = "localhost";
    for(int i=0; i< strlen(loc); i++)
        temp += loc[i];

    return temp;
  }
  temp = strtol(first_p, NULL, 0);

  if(temp <= 0)
  {
    printf(" port error: wrong port number\n");
    display_help();
    exit(EXIT_FAILURE);
  }

  return temp;
}

int main(int argc, char* argv[])
{
    int c;
    char* tempbuff = NULL;
    char* raport_path = NULL;
    float pace_val = 0;
    int port_addr = 0;

    while((c = getopt(argc, argv, "r:t:")) != -1)
      {
        switch(c)
        {
            case 'r':
                raport_path = malloc(sizeof(optarg));
                strcpy(raport_path, optarg);
                break;
            case 't':
                tempbuff = malloc(sizeof(optarg));
                strcpy(tempbuff, optarg);
                pace_val = convert_to_float18(tempbuff);
                break;
            case '?':
                break;
        }
      }


      printf("%lf\n", pace_val);

      if((raport_path == NULL) || (pace_val == 0) || (argv[optind] == NULL))
      {
        printf(" Mandatory parameters: -r <path>, -t <val>, [<addr>]:port\n");
        display_help();
        exit(EXIT_FAILURE);
      }

      port_addr = convert_address(argv[optind]);
      printf("--%d\n", port_addr);






    return 0;
}
