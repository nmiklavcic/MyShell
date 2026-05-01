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
        if ( buff[i] == '#' && buff[i-1] == '\0')
        {
            // buff[i] = '\0';
            // token_num--;
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
    
    if ( i > 0 && buff[i-1] != '\0' ) token_num++;

    buff_size = strlen(buff);

    if(buff_size == 0 && !is_legit)
    {
        token_num = 0;
    }

    return token_num;
}

int check_redirect(char * buff, int token_num)
{
    // parses tokens for redirect or background execution
    int k = 0;
    int start_token = 0;

    if ( token_num >= 3 )
    {
        start_token = token_num - 3;
        for ( int i = 0; i < start_token; i++) k += (strlen(&buff[k]) + 1 );
    }

    int options_num = 0;

    for ( int curr_token = start_token ; curr_token < token_num; curr_token++ )
    {
   
        if ( buff[k] == '\0' )
        {
            k += (strlen(&buff[k]) + 1 );
            curr_token--;
            continue;
        }
        else if ( ( buff[k] == '<' || buff[k] == '>' ) && strlen(&buff[k]) > 1 )
        {
            options_num++;
        }
        else if ( buff[k] == '&' && strlen(&buff[k]) == 1 )
        {
            options_num++;
        }
        else
        {
            // DEBUG
            // printf("Curr token : %c\n", buff[k]);
        }
        k += (strlen(&buff[k]) + 1 );
    }

    return options_num;
}

int print_tokens(char * buff, int token_num, int options_num)
{

    int k = 0;

    for ( int curr_token = 0; curr_token < token_num; curr_token++ )
    {
        if ( buff[k] == '\0' )
        {
            k += (strlen(&buff[k]) + 1 );
            curr_token--;
            continue;
        }
        
        printf("Token %d: '%s'\n", curr_token, &buff[k]);
        fflush(stdout);
        k += (strlen(&buff[k]) + 1 );
    }

    if ( options_num > 0 )
    {
        int start_char = 0;
        int start_token = token_num - options_num;
        
        for ( int i = 0; i < start_token; i++) start_char += (strlen(&buff[start_char]) + 1 );
        

        for ( int curr_token = start_token; curr_token < token_num; curr_token++ )
        {
            if ( buff[start_char] == '\0' )
            {
                start_char += (strlen(&buff[start_char]) + 1 );
                curr_token--;
                continue;
            }
            if ( buff[start_char] == '<' )
            {
                printf("Input redirect: '%s'\n", &buff[start_char + 1]);
                fflush(stdout);
            }
            else if ( buff[start_char] == '>' )
            {
                printf("Output redirect: '%s'\n", &buff[start_char + 1]);
                fflush(stdout);
            }
            else if ( buff[start_char] == '&' )
            {
                printf("Background: %d\n", 1 /*Probably have to implement multiple background tasks in future*/);
                fflush(stdout);
            }
            else
            {
                printf("Error in options count! %c is unknown!\n", buff[start_char]);
                fflush(stdout);
                return 1;
            }

            start_char += (strlen(&buff[start_char]) + 1 );
        }
    }

    return 0;
}

int parse(char * buff, int token_num)
{
    // DEBUG
    // printf("Token count : %d\n", token_num);

    int options_num = check_redirect(buff, token_num);
    // DEBUG
    // printf("Options count : %d\n", options_num);

    print_tokens(buff, token_num, options_num);
}

int main(int argc, char * argv[]) 
{
    // for now lets imagine the read part as a constant while loop
    
    int token_num = 0;
    
    while (1)
    {
        // read from stdin
        char * buff = calloc( MAX_CHARS, sizeof(char) );
        if ( fgets( buff, MAX_CHARS, stdin ) == NULL )
        {
            free(buff);
            break;
        }
        int len = strlen(buff);  
        
        // do stuff only if something is given as input 
        if (  len > 0 && buff[len - 1] == '\n')
        buff[len - 1] = '\0';
        
        token_num = tokenize(buff);

        parse(buff, token_num);

        /* DEBU 
        printf("%s",buff);
        */
        
    }

    return 0;
}