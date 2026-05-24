// The point of this project is to create a working shell prototype in C

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/utsname.h>

typedef int (*function)(char * buff, int token_num);

typedef struct {
    char * name;
    function fn;
} Builtin;

char PROMPT[9] = "mysh";

int MAX_CHARS = 1000;
int DEBUG_LVL = 0;
int IS_BUILTIN = 0;
int BACKGROUND = 0;

int STATUS = 0;
int EXIT = 0;

// section where builtins will live 

int my_debug(char * buff, int token_num)
{
    if ( token_num == 1 )
    {
        // only token is debug so we print what debug level we are on
        printf("%d\n", DEBUG_LVL);
        fflush(stdout);
    }
    else
    {
        int lvl_start = strlen(&buff[0]) + 1;
        DEBUG_LVL = atoi(&buff[lvl_start]);
    }

    return 0;
}

int my_prompt(char * buff, int token_num)
{
    if ( token_num == 1 )
    {
        // only token is prompt so we print what prompt is currently set
        printf("%s\n", &PROMPT[0]);
        fflush(stdout);
    }
    else
    {
        int prpt_start = strlen(&buff[0]) + 1;
        while ( buff[prpt_start] == '\0' ) prpt_start++;
        if ( strlen(&buff[prpt_start]) > 8 ) 
        {
            return 1;
        }
        strcpy(PROMPT, &buff[prpt_start]);
    }

    return 0;
}

int my_status(char * buff, int token_num)
{
    printf("%d\n", STATUS);
    fflush(stdout);
    return STATUS;
}

int my_exit(char * buff, int token_num)
{
    if ( token_num > 1 )
    {
        int ext_num_start = strlen(&buff[0]) + 1;
        while ( buff[ext_num_start] == '\0' ) ext_num_start++;
        STATUS = atoi(&buff[ext_num_start]);
    }

    // DEBUG
    // printf("Exit status: %d\n", STATUS);
    // fflush(stdout);
    EXIT = 1;
    return STATUS;
}

int my_print(char * buff, int token_num)
{
    // skip print 
    // start printing everything after that 
    // skip # 
    int k = strlen(&buff[0]) + 1;

    for ( int curr_token = 1; curr_token < token_num; curr_token++ )
    {
        if ( buff[k] == '\0' )
        {
            k += (strlen(&buff[k]) + 1 );
            curr_token--;
            continue;
        }
        
        printf("%s", &buff[k]);
        if ( curr_token < token_num - 1 ) printf(" ");
        fflush(stdout);
        k += (strlen(&buff[k]) + 1 );
    }

    return 0;
}

int my_echo(char * buff, int token_num)
{
    // skip echo 
    // start printing everything after that 
    // skip # 
    int k = strlen(&buff[0]) + 1;

    for ( int curr_token = 1; curr_token < token_num; curr_token++ )
    {
        if ( buff[k] == '\0' )
        {
            k += (strlen(&buff[k]) + 1 );
            curr_token--;
            continue;
        }
        
        printf("%s", &buff[k]);
        if ( curr_token < token_num - 1 ) printf(" ");
        k += (strlen(&buff[k]) + 1 );
    }

    printf("\n");
    fflush(stdout);

    return 0;
}

int my_len(char * buff, int token_num)
{
    // skip len
    // count everything else
    int k = strlen(&buff[0]) + 1;

    for ( int curr_token = 1; curr_token < token_num; curr_token++ )
    {
        if ( buff[k] == '\0' )
        {
            k += (strlen(&buff[k]) + 1 );
            curr_token--;
            continue;
        }   
        k += (strlen(&buff[k]) + 1);
    }
    // subtract length of len and the first space
    k -= (int)strlen(&buff[0]) + 1;
    // we also need to subtract the token num 
    // this acts as a sbtraction of spaces 
    k -= token_num - 1;

    printf("%d\n",k);

    return 0;
}

int my_sum(char * buff, int token_num)
{
    // skip len
    // count everything else
    int k = strlen(&buff[0]) + 1;
    int sum = 0;

    for ( int curr_token = 1; curr_token < token_num; curr_token++ )
    {
        if ( buff[k] == '\0' )
        {
            k += (strlen(&buff[k]) + 1 );
            curr_token--;
            continue;
        }   
        sum += atoi(&buff[k]);
        k += (strlen(&buff[k]) + 1);
    }

    printf("%d\n",sum);

    return 0;
}

int my_calc(char * buff, int token_num)
{
    // only accepts 3 arguments
    if ( token_num > 4 )
    {
        return 1;
    }

    int k = strlen(&buff[0]) + 1;

    int num1 = atoi(&buff[k]);
    
    k += strlen(&buff[k]) + 1;

    char op = buff[k];

    k += strlen(&buff[k]) + 1;

    int num2 = atoi(&buff[k]);

    // DEBUG
    // printf("%d %c %d\n", num1, op, num2);

    switch(op)
    {
        case '+':
            printf("%d\n", num1 + num2);
            break;
        case '-':
            printf("%d\n", num1 - num2);
            break;
        case '*':
            printf("%d\n", num1 * num2);
            break;
        case '/':
            printf("%d\n",num1 / num2);
            break;
        case '%':
            printf("%d\n",num1 % num2);
            break;
    }
    
    return 0;
}

int my_basename(char * buff, int token_num)
{
    if ( token_num == 1 ) return 1;

    char * basename = calloc(1024,sizeof(char));

   
    int start = (int)strlen(&buff[0]) + 1;
    // DEBUG
     printf("%d\n", (int)strlen(&buff[start]));

    for ( int k = start; k < strlen(&buff[0]) + 1 + strlen(&buff[start]); k++ )
    {
        // DEBUG
        // printf("%s\n",&buff[k]);
        // printf("%d\n",k);
        if ( buff[k] == '/' )
        {
            k += 1;
            strcpy(basename, &buff[k]);
        }  
    }

    printf("%s\n", basename);
    
    free(basename);
    return 0;
}

int my_dirname(char * buff, int token_num)
{
    if ( token_num == 1 ) return 1;

    char * dirname = calloc(1024,sizeof(char));
   
    int start = (int)strlen(&buff[0]) + 1;
    // DEBUG
     printf("%d\n", (int)strlen(&buff[start]));
    int last_slsh = 0;

    for ( int k = start; k < strlen(&buff[0]) + 1 + strlen(&buff[start]); k++ )
    {
        // DEBUG
        // printf("%s\n",&buff[k]);
        // printf("%d\n",k);
        if ( buff[k] == '/' )
        {
            last_slsh = k;
        }  
    }

    buff[last_slsh] = '\0';
    strcpy(dirname, &buff[start]);
    printf("%s\n", dirname);
    
    free(dirname);

    return 0;
}

int my_dirch(char * buff, int token_num)
{
    if ( token_num == 1 )
    {
        chdir("/");
        return 0;
    }

    int start = (int)strlen(&buff[0]) + 1;

    if ( -1 == chdir(&buff[start])) 
    {
        printf("dirch: %s\n", strerror(errno));
        return errno;
    }

    return 0;
}

int my_dirwd(char * buff, int token_num)
{
    char * curr_dir = calloc(1024, sizeof(char));
    getcwd(curr_dir, 1024);

    int start = (int)strlen(&buff[0]) + 1;
    char flag = (token_num > 1) ? buff[start] : 'b';

    if ( flag == 'f' )
    {
        printf("%s\n", curr_dir);
    }
    else if ( flag == 'b' )
    {
        int end = 0;
        for ( int k = 0; k < strlen(&curr_dir[0]); k++ )
        {
            if ( curr_dir[k] == '/' )
            {
                k += 1;
                end = k;
            }  
        }
        if (strlen(curr_dir) == 1) printf("%s\n", &curr_dir[end - 1]);
        else printf("%s\n", &curr_dir[end]);
    }

    fflush(stdout);
    return 0;
}

int my_dirmk(char * buff, int token_num)
{
    int start = (int)strlen(&buff[0]) + 1;
    
    if ( mkdir(&buff[start], 0755) == -1 )
    {
        printf("dirmk: %s\n",strerror(errno));
        return errno;
    }

    return 0;
}

int my_dirrm(char * buff, int token_num)
{
    int start = (int)strlen(&buff[0]) + 1;
    
    if ( rmdir(&buff[start]) == -1 )
    {
        printf("dirrm: %s\n",strerror(errno));
        return errno;
    }

    return 0;
}

int my_dirls(char * buff, int token_num)
{
    char *path;
    char cwd[1024];

    if ( token_num < 2 )
    {
        getcwd(cwd, 1024);
        path = cwd;
    }
    else
    {
        int start = (int)strlen(&buff[0]) + 1;
        while ( buff[start] == '\0' ) start++;
        path = &buff[start];
    }

    DIR *dir = opendir(path);
    if ( dir == NULL )
    {
        printf("dirls: %s: '%s'\n", strerror(errno), path);
        fflush(stdout);
        return errno;
    }

    struct dirent *entry;
    int first = 1;
    while ( (entry = readdir(dir)) != NULL )
    {
        if ( first )
        {
            printf("%s", entry->d_name);
            first = 0;
        }
        else
        {
            printf("  %s", entry->d_name);
        }
    }
    printf("\n");

    fflush(stdout);
    closedir(dir);
    return 0;
}

int my_unlink(char * buff, int token_num)
{
    if ( token_num < 2 ) return 1;

    int start = (int)strlen(&buff[0]) + 1;
    while ( buff[start] == '\0' ) start++;

    if ( unlink(&buff[start]) == -1 )
    {
        printf("unlink: %s: '%s'\n", strerror(errno), &buff[start]);
        fflush(stdout);
        return errno;
    }

    return 0;
}

int my_rename(char * buff, int token_num)
{
    if ( token_num < 3 ) return 1;

    int start = (int)strlen(&buff[0]) + 1;
    while ( buff[start] == '\0' ) start++;

    char *arg1 = &buff[start];

    int next = start + (int)strlen(arg1) + 1;
    while ( buff[next] == '\0' ) next++;

    char *arg2 = &buff[next];

    if ( rename(arg1, arg2) == -1 )
    {
        printf("rename: %s: '%s'\n", strerror(errno), arg1);
        fflush(stdout);
        return errno;
    }
    return 0;
}

int my_remove(char * buff, int token_num)
{
    if ( token_num < 2 ) return 1;

    int start = (int)strlen(&buff[0]) + 1;
    while ( buff[start] == '\0' ) start++;

    if ( remove(&buff[start]) == -1 )
    {
        printf("remove: %s: '%s'\n", strerror(errno), &buff[start]);
        fflush(stdout);
        return 1;
    }
    return 0;
}

int my_linkhard(char * buff, int token_num)
{
    if ( token_num < 3 ) return 1;

    int start = (int)strlen(&buff[0]) + 1;
    while ( buff[start] == '\0' ) start++;

    char *goal = &buff[start];

    int next = start + (int)strlen(goal) + 1;
    while ( buff[next] == '\0' ) next++;

    char *name = &buff[next];

    if ( link(goal, name) == -1 )
    {
        printf("linkhard: %s: '%s'\n", strerror(errno), goal);
        fflush(stdout);
        return 1;
    }
    return 0;
}

int my_linksoft(char * buff, int token_num)
{
    if ( token_num < 3 ) return 1;

    int start = (int)strlen(&buff[0]) + 1;
    while ( buff[start] == '\0' ) start++;

    char *goal = &buff[start];

    int next = start + (int)strlen(goal) + 1;
    while ( buff[next] == '\0' ) next++;

    char *name = &buff[next];

    if ( symlink(goal, name) == -1 )
    {
        printf("linksoft: %s: '%s'\n", strerror(errno), goal);
        fflush(stdout);
        return 1;
    }
    return 0;
}

int my_linkread(char * buff, int token_num)
{
    if ( token_num < 2 ) return 1;

    int start = (int)strlen(&buff[0]) + 1;
    while ( buff[start] == '\0' ) start++;

    char target[1024];

    ssize_t len = readlink(&buff[start], target, sizeof(target) - 1);
    if ( len == -1 )
    {
        printf("linkread: %s: '%s'\n", strerror(errno), &buff[start]);
        fflush(stdout);
        return 1;
    }

    target[len] = '\0';
    printf("%s\n", target);
    fflush(stdout);
    return 0;
}

int my_linklist(char * buff, int token_num)
{
    if ( token_num < 2 ) return 1;

    int start = (int)strlen(&buff[0]) + 1;
    while ( buff[start] == '\0' ) start++;

    char *path = &buff[start];

    struct stat target_stat;
    if ( stat(path, &target_stat) == -1 )
    {
        printf("linklist: %s: '%s'\n", strerror(errno), path);
        fflush(stdout);
        return 1;
    }

    char cwd[1024];
    getcwd(cwd, 1024);

    DIR *dir = opendir(cwd);
    if ( dir == NULL )
    {
        printf("linklist: %s\n", strerror(errno));
        fflush(stdout);
        return 1;
    }

    struct dirent *entry;
    struct stat entry_stat;
    char entry_path[2048];
    int first = 1;

    while ( (entry = readdir(dir)) != NULL )
    {
        snprintf(entry_path, sizeof(entry_path), "%s/%s", cwd, entry->d_name);
        if ( stat(entry_path, &entry_stat) == -1 ) continue;

        if ( entry_stat.st_ino == target_stat.st_ino && entry_stat.st_dev == target_stat.st_dev )
        {
            if ( first )
            {
                printf("%s", entry->d_name);
                first = 0;
            }
            else
            {
                printf("  %s", entry->d_name);
            }
        }
    }
    printf("\n");
    fflush(stdout);
    closedir(dir);
    return 0;
}

int my_cpcat(char * buff, int token_num)
{
    int in, out;

    if ( token_num == 1 )
    {
        in = 0;
        out = 1;
    }
    else if ( token_num == 2 )
    {
        int start = (int)strlen(&buff[0]) + 1;
        while ( buff[start] == '\0' ) start++;
        in = open(&buff[start], O_RDONLY);
        if ( in == -1 )
        {
            printf("cpcat: %s\n", strerror(errno));
            fflush(stdout);
            return errno;
        }
        out = 1;
    }
    else if ( token_num == 3 )
    {
        int start = (int)strlen(&buff[0]) + 1;
        while ( buff[start] == '\0' ) start++;
        char *arg1 = &buff[start];

        int next = start + (int)strlen(arg1) + 1;
        while ( buff[next] == '\0' ) next++;
        char *arg2 = &buff[next];

        in = open(arg1, O_RDONLY);
        if ( in == -1 )
        {
            printf("cpcat: %s\n", strerror(errno));
            fflush(stdout);
            return errno;
        }
        out = open(arg2, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if ( out == -1 )
        {
            printf("cpcat: %s\n", strerror(errno));
            fflush(stdout);
            close(in);
            return errno;
        }
    }
    else return 1;

    char c;
    while ( read(in, &c, 1) > 0 )
    {
        if ( write(out, &c, 1) == -1 )
        {
            printf("cpcat: write error: %s\n", strerror(errno));
            fflush(stdout);
            if ( in != 0 ) close(in);
            if ( out != 1 ) close(out);
            return errno;
        }
    }

    if ( in != 0 ) close(in);
    if ( out != 1 ) close(out);

    return 0;
}

int my_pid(char * buff, int token_num)
{
    pid_t pid = getpid();   
    printf("%d\n", pid);
    fflush(stdout);
    
    return 0;
}

int my_ppid(char * buff, int token_num)
{
    pid_t ppid = getppid();
    printf("%d\n", ppid);
    fflush(stdout);
    return 0;
}

int my_uid(char * buff, int token_num)
{
    uid_t uid = getuid();
    printf("%d\n", uid);
    fflush(stdout);

    return 0;
}

int my_euid(char * buff, int token_num)
{
    uid_t euid = geteuid();
    printf("%d\n", euid);
    fflush(stdout);

    return 0;
}

int my_gid(char * buff, int token_num)
{
    uid_t gid = getgid();
    printf("%d\n", gid);
    fflush(stdout);

    return 0;
}

int my_egid(char * buff, int token_num)
{
    uid_t egid = getegid();
    printf("%d\n", egid);
    fflush(stdout);

    return 0;
}

int my_sysinfo(char * buff, int token_num)
{
    struct utsname info;
    uname(&info);
    printf("Sysname: %s\n", info.sysname);
    printf("Nodename: %s\n", info.nodename);
    printf("Release: %s\n", info.release);
    printf("Version: %s\n", info.version);
    printf("Machine: %s\n", info.machine);
    fflush(stdout);
  
    return 0;
}

Builtin BUILTINS[] = {
    {"debug", my_debug},
    {"prompt", my_prompt},
    {"status", my_status},
    {"exit", my_exit},
    {"print", my_print},
    {"echo", my_echo},
    {"len", my_len},
    {"sum", my_sum},
    {"calc", my_calc},
    {"basename", my_basename},
    {"dirname", my_dirname},
    {"dirch", my_dirch},
    {"dirwd", my_dirwd},
    {"dirmk", my_dirmk},
    {"dirrm", my_dirrm},
    {"dirls", my_dirls},
    {"rename", my_rename},
    {"unlink", my_unlink},
    {"remove", my_remove},
    {"linkhard", my_linkhard},
    {"linksoft", my_linksoft},
    {"linkread", my_linkread},
    {"linklist", my_linklist},
    {"cpcat", my_cpcat},
    {"pid", my_pid},
    {"ppid", my_ppid},
    {"uid", my_uid},
    {"euid", my_euid},
    {"gid", my_gid},
    {"egid", my_egid},
    {"sysinfo", my_sysinfo}
};

int BUILTIN_NUM = 31;

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
        if ( buff[i] == '#' && i != 0 && buff[i-1] == '\0')
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
        int k = 0;
        printf("External command '");
        fflush(stdout);
        for ( int i = 0; i < token_num - options_num; i++ )
        {
            printf("%s", &buff[k]);
            fflush(stdout);
            if ( i < token_num - options_num - 1 ) printf(" ");
            fflush(stdout);
            k += strlen(&buff[k]) + 1;
        }
        printf("'\n");
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
            STATUS = BUILTINS[i].fn(buff, token_num);
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

    return 0;
}

int main(int argc, char * argv[]) 
{
    // for now lets imagine the read part as a constant while loop
    
    int token_num = 0;
    
    while (EXIT == 0)
    {
        // reset pub variables
        IS_BUILTIN = 0;
        BACKGROUND = 0;
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
        free(buff);
    }

    return STATUS;
}