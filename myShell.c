// The point of this project is to create a working shell prototype in C

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int MAX_CHARS = 1000;

int tokenize(char * buff)
{   
    printf("Input line: '%s'\n", buff);
    fflush(stdout);
    int i = 0;
    while( buff[i] != '\0' )
    {
        // we replace spaces with \0 to make suibstrings
        if( buff[i] == ' ' )
        {
            buff[i] = '\0';
            i++;
            continue;
        }
        else
        {
            i++;
            continue;
        }
    }
    return 0;
}

int main(int argc, char * argv[]) 
{
    // for now lets imagine the read part as a constant while loop
    while (1)
    {
        // read from stdin
        char * buff = malloc( sizeof(char) * MAX_CHARS );
        int read_status = read( 0, buff, MAX_CHARS );
        // do stuff only if somethiung actually read :)
        if ( 0 != (strcmp(buff, "")) )
        {
            buff[read_status - 1] = '\0';

            if ( read_status == -1 )
            {
                perror("Read: ");
                return errno;
            }    

            tokenize(buff);
        }
        
        /* DEBU 
        printf("%s",buff);
        */
       
    }


    return 0;
}