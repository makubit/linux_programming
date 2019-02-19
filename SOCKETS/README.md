## PRODUCER - CONSUMER
Program demonstrates socket communication between producer and consumers.

Producer generates data stored in circular buffer, generates raport (new or lost connection, stock status) and sends data to consumers.

Consumer reads data received from producer, computes md5sum on this data and generates report.

### COMPILATION
producent.c: ```gcc -Wall producent.c -o p.out```  
konsument.c: ```gcc -Wall konsument.c -o k.out -lssl -lcrypto```  

### USAGE
**Instructions to producent.c**  
 - -r \<char*\> mandatory, path to ganerate raports  
 - -t \<int\> mandatory, \<int\> has to be in range(1, 8)  
 - addr:port mandatory,  port & IP address  
 
 **Instructions to konsument.c**  
 - -#\<int\> optional(:\<int\>) mandatory parameter, number of blocks we want to get from producer  
 - -s \<float\> | -r \<float\> mandatory, time  
 - addr:port mandatory,  port & IP address, port is mandatory 

