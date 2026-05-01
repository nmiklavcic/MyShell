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

    int token_num = 0;
    int i = 0;
    int buff_size = strlen(buff);
    int is_legit = 0;

    while( buff[i] != '\0' )
    {
        // we replace spaces with \0 to make suibstrings
        if ( buff[i] == '#')
        {
            buff[i] == '\0';
            if( buff[i-1] == '\0' ) token_num--;
            break;
        }
        else if( buff[i] == ' ' )
        {
            buff[i] = '\0';
            if ( i == 0 )
            {
                i++;
                continue;
            }
            else if ( buff[i-1] == '\0' && i != 0 ) 
            {
                i++;
                continue;
            }
            i++;
            token_num++;
            continue;
        }
        else if ( buff[i] == '"' )
        {
            // Find next " and don't do anything with the spaces in between the two
            // DEBUG
            // printf("Currently inside of sttring recognition\n");
            // fflush(stdout);
            buff[i] = '\0';
            i++;
            while ( buff[i] != '"' )
            {
                // DEBUG
                // printf("Current symbol is : %c\n", buff[i]);
                // sleep(1);
                
                if ( i == buff_size - 2 ) 
                {
                    printf("Error! - reached end of line without second %c.\nFix input line %c should always come in pairs.",'"', '"');
                    fflush(stdout);
                    return 1;
                }
                i++;
            }
            buff[i] = '\0';
            i++;
            token_num++;
            continue;
        }
        else
        {
            is_legit = 1;
            i++;
            continue;
        }
    }
    token_num++;
    buff_size = strlen(buff);

    if(buff_size == 0 && !is_legit)
    {
        token_num = 0;
    }

    return token_num;
}

int print_tokens(char * buff, int token_num)
{
    

    int buff_size = strlen(buff);
    int k = 0;

    for ( int curr_token = 0; curr_token < token_num; curr_token++ )
    {
        if ( buff[k] == '\0' ) continue;
        
        printf("Token %d: '%s'\n", curr_token, &buff[k]);
        fflush(stdout);
        k += (strlen(&buff[k]) + 1 );
    }

    return 0;
}

int main(int argc, char * argv[]) 
{
    // for now lets imagine the read part as a constant while loop
    
    int token_count = 0;
    
    while (1)
    {
        // read from stdin
        char * buff = malloc( sizeof(char) * MAX_CHARS );
        if ( fgets( buff, MAX_CHARS, stdin ) == NULL )
        {
            free(buff);
            break;
        }
        int len = strlen(buff);  
        // do stuff only if somethiung actually read :)
        if (  len > 0 && buff[len - 1] == '\n')
        buff[len - 1] = '\0';
        token_count = tokenize(buff);

        // DEBUG
        printf("Token count : %d\n", token_count);

        print_tokens(buff, token_count);

        /* DEBU 
        printf("%s",buff);
        */
        
    }

    return 0;
}