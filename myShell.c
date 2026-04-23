// The point of this project is to create a working shell prototype in C

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int MAX_TOKENS = 1000;

void tokenize()
{
    
}

int main(int argc, char * argv[]) 
{
    // for now lets imagine the read part as a constant while loop
    while (1)
    {
        // read from stdin
        char * buff = malloc( sizeof(char) * MAX_TOKENS );
        int read_status = read( 0, buff, MAX_TOKENS + 1 );
        if ( read_status == -1 )
        {
            perror("Read: ");
            return errno;
        }
        printf("%s",buff);
    }


    return 0;
}