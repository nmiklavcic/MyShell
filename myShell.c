// The point of this project is to create a working shell prototype in C

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

typedef int (*function)(char * buff, int token_num);

typedef struct {
    char * name;
    function fn;
} Builtin;


int MAX_CHARS = 1000;
int DEBUG_LVL = 0;
int IS_BUILTIN = 0;
int BACKGROUND = 0;

// section where builtins will live 

int debug(char * buff, int token_num)
{
    if ( token_num == 1 )
    {
        // only token is debug so we print what debug level we are on
        printf("%d\n", DEBUG_LVL);
    }
    else
    {
        int lvl_start = strlen(&buff[0]) + 1;
        DEBUG_LVL = atoi(&buff[lvl_start]);
    }

    return 0;
}

Builtin BUILTINS[] = {
    {"debug", debug}
};

int BUILTIN_NUM = 1;

int tokenize(char * buff)
{   
    if ( DEBUG_LVL > 0 )
    {
        printf("Input line: '%s'\n", buff);
        fflush(stdout); 
    }

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
                
                if ( i == buff_size - 1 ) 
                {
                    printf("Error! - reached end of line without second %c.\nFix input line %c should always come in pairs.\n",'"', '"');
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
                BACKGROUND = 1;
                printf("Background: %d\n", BACKGROUND /*Probably have to implement multiple background tasks in future*/);
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

    // TODO 
    // Add recognition of builtin vs external commands and printline 
    // Executing builtin X / External command X
    if ( IS_BUILTIN == 0 )
    {
        if ( BACKGROUND == 0 )
        {
            printf("Executing builtin '%s' in foreground\n", &buff[0]);
            fflush(stdout);
        }
        else
        {
            printf("Executing builtin '%s' in background\n", &buff[0]);
            fflush(stdout);
        }
    }
    else
    {
        printf("External command '%s'\n", "placeholder"/*Print the input line commands without redirects and comments*/ );
        fflush(stdout);
    }

    return 0;
}

int check_builtin(char * token)
{
    // Check if command is builtin 
    // return 0 on yes, 1 on no 
    for ( int i = 0; i < BUILTIN_NUM; i++ )
    {
        if ( strcmp(token, BUILTINS[i].name) == 0 )
        {
            // the command is builtin
            return 0;
        }
    } 
    return 1;
}

int execute_builtin(char * buff, int token_num)
{
    for ( int i = 0; i < BUILTIN_NUM; i++ )
    {
        if ( strcmp(&buff[0], BUILTINS[i].name) == 0 )
        {
            BUILTINS[i].fn(buff, token_num);
        }
    }
    return 0;
}

int execute_external(char * buff)
{

    return 0;
}

int parse(char * buff, int token_num)
{
    // DEBUG
    // printf("Token count : %d\n", token_num);

    int options_num = check_redirect(buff, token_num);
    
    // DEBUG
    // printf("Options count : %d\n", options_num);

    // check if command is builtin, pass the first token of the buffer
    IS_BUILTIN = check_builtin(&buff[0]);

    // print tokens will only be called if debug is enabled ( DEBUG_LVL > 0 )
    if ( DEBUG_LVL > 0 ) print_tokens(buff, token_num, options_num);

    // execute command
    if ( IS_BUILTIN == 0 )
    {
        execute_builtin(buff, token_num);
    }
    else
    {
        execute_external(buff);
    }

}

int main(int argc, char * argv[]) 
{
    // reset pub variables
    IS_BUILTIN = 0;
    BACKGROUND = 0;

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