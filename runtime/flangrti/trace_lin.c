/*
 * Copyright (c) 2006-2018, NVIDIA CORPORATION.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "flangrti_config.h"
#include <signal.h>
#if defined(HAVE_GREGSET_T)
#include <sys/ucontext.h>
#endif
#include <execinfo.h>
#include <stdioInterf.h>
#include "dumpregs.h"
#include <stdlib.h>
#include <string.h>

/* codes and strings for signals */

struct cods {
  int code;  /* signal code */
  char *str; /* string */
};

#define CODNULL ((struct cods *)0)

static struct cods codill[] = {{ILL_ILLOPC, "illegal opcode"},
                               {ILL_ILLOPN, "illegal operand"},
                               {ILL_ILLADR, "illegal addressing mode"},
                               {ILL_ILLTRP, "illegal trap"},
                               {ILL_PRVOPC, "privileged opcode"},
                               {ILL_PRVREG, "privileged register"},
                               {ILL_COPROC, "coprocessor error"},
                               {ILL_BADSTK, "internal stack error"},
                               {0, NULL}};

static struct cods codfpe[] = {{FPE_INTDIV, "integer divide by zero"},
                               {FPE_INTOVF, "integer overflow"},
                               {FPE_FLTDIV, "floating point divide by zero"},
                               {FPE_FLTOVF, "floating point overflow"},
                               {FPE_FLTUND, "floating point underflow"},
                               {FPE_FLTRES, "floating point inexact result"},
                               {FPE_FLTINV, "floating point invalid operation"},
                               {FPE_FLTSUB, "subscript out of range"},
                               {0, NULL}};

static struct cods codsegv[] = {
    {SEGV_MAPERR, "address not mapped to object"},
    {SEGV_ACCERR, "invalid permissions for mapped object"},
    {0, NULL}};

static struct cods codbus[] = {{BUS_ADRALN, "invalid address alignment"},
                               {BUS_ADRERR, "non-existent physical address"},
                               {BUS_OBJERR, "object specific hardware error"},
                               {0, NULL}};

/* signals handled and message strings */

struct sigs {
  int sig;          /* signal value */
  struct cods *cod; /* address of optional code info */
  char *str;        /* message string */
};

static struct sigs sigs[] = {
    {SIGINT, CODNULL, "interrupt"},
    {SIGILL, codill, "illegal instruction"},
    {SIGABRT, CODNULL, "abort"},
    {SIGFPE, codfpe, "floating point exception"},
    {SIGSEGV, codsegv, "segmentation violation"},
    {SIGTERM, CODNULL, "software termination"},
    {SIGPIPE, CODNULL, "write on a pipe with no one to read it"},
    {SIGSYS, CODNULL, "bad argument to system call"},
    {SIGHUP, CODNULL, "hangup"},
    {SIGBUS, codbus, "bus error"},
    {SIGQUIT, CODNULL, "quit"},
    {SIGTRAP, CODNULL, "trace trap"},
    {SIGIOT, CODNULL, "IOT instruction"},
    {0, CODNULL, NULL} /* end of list */
};

#if defined(HAVE_GREGSET_T)
static gregset_t *regs; /* pointer to regs at signal  */
#else
static void *regs; /* pointer to regs at signal  */
#endif

extern char **__io_get_argv();
static char ** saved_argv;

/* walk the stack back */

#define MAXTRACE (32 * 1024)

void
print_back_trace_line(char * bt_str, void const * const addr)
{
  char addr2line_cmd[512], cmd_out[1024];
  char *ptr_qmark, *ptr_colon, *ptr_lparen, *ptr_rparen, *ptr_plus;
  static char buffer[4096];
  FILE *fp;

  saved_argv = __io_get_argv();
  sprintf(addr2line_cmd,"addr2line -e %s %p", saved_argv[0], addr);

  fp = popen(addr2line_cmd, "r");
  if (fp != NULL) {
    if (fgets(cmd_out, sizeof(cmd_out) - 1, fp) != NULL) {
      if (cmd_out[strlen(cmd_out) - 1] == '\n')
        cmd_out[strlen(cmd_out) - 1] = '\0';
      ptr_qmark = strchr(cmd_out, '?');
      ptr_colon = strchr(cmd_out, ':');
      ptr_lparen=strchr(bt_str, '(');
      ptr_rparen=strchr(bt_str, ')');
      if (ptr_lparen != NULL && ptr_lparen != NULL && ptr_lparen < ptr_rparen)
        ptr_plus = strchr(ptr_lparen, '+');
      if(ptr_qmark != NULL) {
        fprintf(__io_stderr(), "  %s\n", bt_str);
      } else if (ptr_colon != NULL && ptr_plus != NULL && ptr_plus < ptr_rparen) {
        /* replace offset by line number */
        strncpy(buffer, bt_str, ptr_plus - bt_str);
        sprintf(buffer + (ptr_plus - bt_str), "%s%s", ptr_colon, ptr_rparen);
        fprintf(__io_stderr(), "  %s\n", buffer);
      }
    } else {
      fprintf(__io_stderr(), "  %s\n", bt_str);
    }
    fclose(fp);
  } else {
    fprintf(__io_stderr(), "  %s\n", bt_str);
  }
}

void
__abort_trace(int skip)
{
  void *array[MAXTRACE];
  size_t size;
  char **strings;
  size_t i;

#if defined(HAVE_GREGSET_T)
  if (regs != (gregset_t *)0) {
    dumpregs(regs);
  }
#endif

  size = backtrace(array, MAXTRACE);
  if (skip + 1 >= size) {
    fprintf(__io_stderr(), "  --- traceback not available\n");
    return;
  }
  strings = backtrace_symbols(array, size);
  if (size < 100) {
    for (i = skip + 1; i < size; i++)
      print_back_trace_line(strings[i], array[i]);
  } else {
    for (i = skip + 1; i < 40; i++)
      print_back_trace_line(strings[i], array[i]);
    fprintf(__io_stderr(), "  --- skipping traceback entries\n");
    for (i = size - 40; i < size; i++)
      print_back_trace_line(strings[i], array[i]);
  }
  free(strings);
}

/*
 * this routine is a signal handler, it prints a message and terminates the
 * process
 */

static void
#if defined(HAVE_GREGSET_T)
__abort_sig_hand(int sig, siginfo_t *in, ucontext_t *u)
#else
__abort_sig_hand(int sig, siginfo_t *in, void *u)
#endif
{
  char *p;
  char b[128];
  int n, m;
  struct sigaction new;
  struct sigaction old;

  new.sa_sigaction = (void (*)(int, siginfo_t *, void *))SIG_DFL;
  sigemptyset(&new.sa_mask);
  new.sa_flags = SA_SIGINFO;
  n = 0;
  while (sigs[n].sig != 0) {
    sigaction(sigs[n].sig, &new, &old);
    n++;
  }

  regs = getRegs(u);

  n = 0;
  while ((sigs[n].sig != 0) && (sigs[n].sig != sig)) {
    n++;
  }
  if (sigs[n].sig == 0) {
    sprintf(b, "signal %d", sig);
    p = b;
  } else {
    p = sigs[n].str;
    m = 0;
    if (sigs[n].cod != CODNULL) {
      while ((sigs[n].cod[m].code != 0) &&
             (sigs[n].cod[m].code != in->si_code)) {
        m++;
      }
      if (sigs[n].cod[m].code != 0) {
        sprintf(b, "%s, %s", p, sigs[n].cod[m].str);
        p = b;
      }
    }
  }
  __abort(3, p);
}

/*
 * this routine initializes the signal handlers
 */

void
__abort_sig_init(void)
{
  struct sigaction new;
  struct sigaction old;
  int n;

  new.sa_sigaction = (void (*)(int, siginfo_t *, void *))__abort_sig_hand;
  sigemptyset(&new.sa_mask);
  new.sa_flags = SA_SIGINFO;
  n = 0;
  while (sigs[n].sig != 0) {
    sigaction(sigs[n].sig, &new, &old);
    n++;
  }
}


