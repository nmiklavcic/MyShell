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
    int i = 1;
    int buff_size = strlen(buff);
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

    int token_num = 0;

    for ( int k = 0; k < buff_size; k += (strlen(&buff[k]) + 1 ) )
    {
        printf("Token %d: '%s'\n", token_num, &buff[k]);
        fflush(stdout);
        token_num++;
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
        if ( fgets( buff, MAX_CHARS, stdin ) == NULL )
        {
            perror("fgets: ");
            return errno;
            break;
        }
        int len = strlen(buff);  
        // do stuff only if somethiung actually read :)
        if (  len > 0 && buff[len - 1] == '\n')
        buff[len - 1] = '\0';
        tokenize(buff);
        /* DEBU 
        printf("%s",buff);
        */
        
    }

    return 0;
}