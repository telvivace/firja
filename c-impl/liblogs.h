#ifndef ORBLOG_C
#define ORBLOG_C
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "settings.h"
/*
This logger's behaviour is governed by the macro ORB_VERBOSE, which you have to set to 0 or 1 before including it.
*/
enum log_priorities {
    PRIORITY_ERR,
    PRIORITY_WARN,
    PRIORITY_OK,
    PRIORITY_MSG,
    PRIORITY_DBUG, 
    PRIORITY_TRACE,
    PRIORITY_BLANK,
    PRIORITY_NUM
};
static char* log_priority_colors[7] = {
    "\033[31m",      //red (error)
    "\033[33m",      //yellow (warning)
    "\033[32m",      //green (ok)
    "\033[30;47;1m", //black on white, bold (message)
    "\033[36m",      //cyan (debugging)
    "\033[35m",      //magenta (trace variables)
    ""        //never called (no formatting applied)

};
static char* log_priority_strings[7] = {
    " [ ERROR ] ",
    " [ WARN  ] ",                
    " [  OK   ] ",
    "           ", //message (blank with space)
    " [ DEBUG ] ",
    " [ TRACE ] ",
    "",            //user's formatting  (blank)
};
static char* log_priority_pieces[7] = {
    "\033[31m" " [ ERROR ] " "\033[0m" "%s", //error (red, stderr)
    "\033[33m" " [ WARN  ] " "\033[0m" "%s", //warning (yellow)
    "\033[32m" " [  OK   ] " "\033[0m" "%s", //ok (green)
    "           " "\033[30;47;1m" "%s" "\033[0m", //message (black bold, white bg)
    "\033[36m" " [ DEBUG ] " "\033[0m" "%s", //debug (cyan, enabled if verbose)
    "\033[35m" " [ TRACE ] " "\033[0m" "%s", //trace (violet, enabled if verbose)
    "%s"                       //just paste the string (if user wants fancy formatting, enabled if verbose)
};

/*return the length of the argument list. Works for up to 32 arguments,
which is more than enough for our purposes. Copied from Modern C:
Jens Gustedt. Modern C. Manning, 2019, 9781617295812. hal-02383654, page 231
*/
#define ARGLEN(...) ARGLEN0(__VA_ARGS__,         \
0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18,  \
0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10,  \
0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08,  \
0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00)

#define ARGLEN0( \
_00, _01, _02, _03, _04, _05, _06, _07,    \
_08, _09, _0A, _0B, _0C, _0D, _0E, _0F,    \
_10, _11, _12, _13, _14, _15, _16, _17,    \
_18, _19, _1A, _1B, _1C, _1D, _1E, _1F, ...) _1F 

#define TEST0(...) TEST1(__VA_ARGS__, 0)
#define TEST1(...) __VA_ARGS__

#define STRGY(...) STRGY0(__VA_ARGS__)
#define STRGY0(...) #__VA_ARGS__

#define DELFIRST(...) DELFIRST0(__VA_ARGS__, 0)
#define DELFIRST0(_0, ...) __VA_ARGS__  //you have to ignore last argument later!!

#define GETFIRST(...) GETFIRST0(__VA_ARGS__, 0)
#define GETFIRST0(_0, ...) _0
/*
Usage: orb_log(PRIORITY, STRING_LITERAL, [NUM, NUM, ...])
priorities: PRIORITY_ERR, PRIORITY_WARN, PRIORITY_OK, PRIORITY_MSG, PRIORITY_DBUG,  PRIORITY_TRACE
The string literal is escaped, so format strings are ineffective. Use orb_logf instead. (TODO)

Examples:
orb_log(PRIORITY_OK) | prints your string preceded by a status message
orb_log(PRIORITY_ERR, "Oh no!") | prints an error status message, then your string. Puts everything in stderr.
orb_log(PRIORITY_TRACE, "Variable a:", a) | prints the string and the value appended after it. Additional values have commas in between them. 
                                            Doesn't do anything if not in verbose mode.
*/
#define orb_log(priority, ...)  log_func(priority+0U, "" GETFIRST(__VA_ARGS__) "", (const double long[ARGLEN(__VA_ARGS__)]){DELFIRST(__VA_ARGS__)}, ARGLEN(__VA_ARGS__)-1U)

static inline void log_func(unsigned priority, const char string[static 1], const double long* args, unsigned argnum) {
    FILE* stream;
    if(priority == PRIORITY_ERR){
        stream = stderr;
    }
    else{
        stream = stdout;
    }

    #if ORB_VERBOSE == 1
    fprintf(stream, log_priority_pieces[priority], string);
    for(size_t i = 0; i < argnum; ++i){ //print the numbers, ignore last number (look into macro wrapper)
        fprintf(stream, " %Lg", args[i]);
    }
    fputc('\n', stream);
    #else
    if(priority == PRIORITY_ERR || priority == PRIORITY_WARN || priority == PRIORITY_MSG || priority == PRIORITY_OK){
        fprintf(stream, log_priority_pieces[priority], string);
        for(size_t i = 0; i < argnum; ++i){ //print the numbers, ignore last number (look into macro wrapper)
            fprintf(stream, " %Lg", args[i]);
        }
        fputc('\n', stream);
    }
    #endif
}
/*avoid using PRIORITY_MSG, because it doesn't work properly here. TODO probably never*/
#define orb_logf(priority, ...)  if(logf_checkverb(priority+0U)){ printf("" GETFIRST(__VA_ARGS__) "%.0d\n", DELFIRST(__VA_ARGS__)); }

static inline int logf_checkverb(unsigned priority){
    #if ORB_VERBOSE == 1

    printf("%s%s\033[0m", log_priority_colors[priority], log_priority_strings[priority]);
    return true;

    #else

    if(priority == PRIORITY_DBUG || priority == PRIORITY_TRACE || priority == PRIORITY_BLANK){
        return false;
    }
    printf("%s%s\033[0m", log_priority_colors[priority], log_priority_strings[priority]);
    return true;
    
    #endif
}
#endif