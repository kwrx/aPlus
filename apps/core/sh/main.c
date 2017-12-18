#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <getopt.h>
#include <glob.h>

#include <sys/ioctl.h>
#include <sys/termio.h>
#include <sys/termios.h>


#include "sh.h"

#if HAVE_LOGIN
#   include "md5.h"
#endif



static char buf[BUFSIZ];
static struct winsize ws;


static void show_usage(int argc, char** argv) {
    printf(
        "Use: sh [options] ...\n"
        "aPlus Shell.\n\n"
        "   -c, --command               execute a command\n"
        "   -h, --help                  show this help\n"
        "   -v, --version               print version info and exit\n"
    );
    
    exit(0);
}

static void show_version(int argc, char** argv) {
    printf(
        "%s (aPlus coreutils) 0.1\n"
        "Copyright (c) 2016 Antonino Natale.\n"
        "Built with gcc %s (%s)\n",
        
        argv[0], __VERSION__, __TIMESTAMP__
    );
    
    exit(0);
}



static void cmd_cd(char** argv) {
    char* p = argv[1];
    if(argv[1] == NULL)
        p = "/";

    if(chdir(p))
        perror(p);
}
    


static void sigwinch(int sig) {
    ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
    signal(SIGWINCH, sigwinch);
}



int main(int argc, char** argv, char** env) {

    signal(SIGWINCH, sigwinch);

    sh_alias("la", (void*) "ls -lh", SH_ALIAS_TYPE_STRING);
    sh_alias("cd", (void*) cmd_cd, SH_ALIAS_TYPE_FUNC);



    static struct option long_options[] = {
        { "command", no_argument, NULL, 'c'},
        { "help", no_argument, NULL, 'h'},
        { "version", no_argument, NULL, 'v'},
        { NULL, 0, NULL, 0 }
    };
    
 
    
    int c, idx;
    while((c = getopt_long(argc, argv, "chv", long_options, &idx)) != -1) {
        switch(c) {
            case 'c':
                sh_cmdline(argv[optind]);
                return 0;
            case 'v':
                show_version(argc, argv);
                break;
            case 'h':
            case '?':
                show_usage(argc, argv);
                break;
            default:
                abort();
        }
    }
    


    ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);

        
    char* username = strdup(getenv("USER"));
    char* hostname = sh_gethostname();


    do {
        CLRBUF();
        char* cwd = getcwd(buf, BUFSIZ);
        if(getenv("HOME"))
            if(strcmp(cwd, getenv("HOME")) == 0)
                cwd = "~";


        time_t t0 = time(NULL);
        struct tm* tm = localtime(&t0);

        fprintf(stdout, "\033[%dC[\033[33m%02d/%02d \033[31m%02d:%02d:%02d\033[39m]\033[%dD", ws.ws_col - 17, tm->tm_mday, tm->tm_mon + 1, tm->tm_hour, tm->tm_min, tm->tm_sec, ws.ws_col - 1);
        fprintf(stdout, "\033[36m[%s@%s %s]%c\033[39m ", username, hostname, cwd, geteuid() == 0 ? '#' : '$');
        fflush(stdout);

        CLRBUF();
        if(fgets(buf, BUFSIZ, stdin) > 0) {
            buf[strlen(buf) - 1] = '\0';
            sh_cmdline(buf);
        }
    } while(1);
}