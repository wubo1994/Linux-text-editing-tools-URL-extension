#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>
#include <sys/reg.h>
#include <sys/syscall.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

const int long_size = sizeof(long);
void getdata(pid_t child, long addr,
             char *str, int len)
{   char *laddr;
    int i, j;
    union u {
            long val;
            char chars[long_size];
    }data;
    i = 0;
    j = len / long_size;
    laddr = str;
    while(i < j) {
        data.val = ptrace(PTRACE_PEEKDATA,
                          child, addr + i * 4,
                          NULL);
        memcpy(laddr, data.chars, long_size);
        ++i;
        laddr += long_size;
    }
    j = len % long_size;
    if(j != 0) {
        data.val = ptrace(PTRACE_PEEKDATA,
                          child, addr + i * 4,
                          NULL);
        memcpy(laddr, data.chars, j);
    }
    str[len] = '\0';
}

void putdata(pid_t child, long addr,
             char *str, int len)
{   char *laddr;
    int i, j;
    union u {
            long val;
            char chars[long_size];
    }data;
    i = 0;
    j = len / long_size;
    laddr = str;
    while(i < j) {
        memcpy(data.chars, laddr, long_size);
        ptrace(PTRACE_POKEDATA, child,
               addr + i * 4, data.val);
        ++i;
        laddr += long_size;
    }
    j = len % long_size;
    if(j != 0) {
        memcpy(data.chars, laddr, j);
        ptrace(PTRACE_POKEDATA, child,
               addr + i * 4, data.val);
    }
}

bool isURL(char *name) {
	if (strlen(name) < 7) {
		return false;
	} else {
		char *prefix = malloc(8 * sizeof(char));
		int i;
		for (i=0; i<7; i++) {
			*(prefix+i) = *(name+i);
		}
		if (strcmp(prefix, "http://") == 0) {
			free(prefix);
			return true;
		} else {
			free(prefix);
			return false;
		}
	}
}

int main(int argc, char **argv)
{
	pid_t child;
	long orig_eax, eax;
	long params[3];
    	int status;
	int insyscall = 0;
	struct user_regs_struct regs;
	child = fork();
	char *realprogram = argv[1];
	char *filename = argv[2];
	char *prefix1 = "/bin/";
	char *prefix2 = "/usr/bin/";
	char *fullpath1 = malloc(strlen(prefix1)+strlen(realprogram)+1);
	char *fullpath2 = malloc(strlen(prefix2)+strlen(realprogram)+1);
	strcpy(fullpath1, prefix1);
	strcat(fullpath1, realprogram);
	strcpy(fullpath2, prefix2);
	strcat(fullpath2, realprogram);
	if (child == 0) {
		ptrace(PTRACE_TRACEME, 0, NULL, NULL);
		int res = execl(fullpath1, realprogram, filename, NULL);
		if (res == -1) {
			printf("Failed to find executable in /bin.\n");
			res = execl(fullpath2, realprogram, filename, NULL);
			if (res == -1) {
				printf("Failed to find executable in /usr/bin.\n");
			}
		}
	} else {
		// char *str;
		while(1) {
          wait(&status);
          if(WIFEXITED(status))
              break;
          orig_eax = ptrace(PTRACE_PEEKUSER,
                            child, 4 * ORIG_EAX,
                            NULL);
          if(orig_eax == SYS_open || orig_eax == SYS_stat64) {
              if(insyscall == 0) {
                 /* Syscall entry */
                 insyscall = 1;
                 ptrace(PTRACE_GETREGS, child,
                        NULL, &regs);
                 //printf("Write called with "
                 //       "%ld, %ld, %ld\n",
                 //       regs.ebx, regs.ecx,
                 //       regs.edx);
		 char *str;
		 str = (char *)malloc(50);
		 getdata(child, regs.ebx, str, 50);
		 if (isURL(str)) {
			/* call wget to download page */
		 	// printf("We have received a URL, let's do something!\n");
			char command[60];
			strcat(command, "wget -O .thepage ");
			strcat(command, str);
			// printf("The command we get is: %s.\n", &command[0]);
			system(&command[0]);
			/* start changing parameter of syscall */
			putdata(child, regs.ebx, ".thepage", strlen(str)+1);
		 }
		 // printf("The filename is: %s.\n", str);
		 free(str);
             }
             else { /* Syscall exit */
                 eax = ptrace(PTRACE_PEEKUSER,
                              child, 4 * EAX,
                              NULL);
                 //printf("Write returned "
                 //       "with %ld\n", eax);
                 insyscall = 0;
             }
          }
          ptrace(PTRACE_SYSCALL, child,
                 NULL, NULL);
       }
	}
	free(fullpath1);
	free(fullpath2);
	system("rm .thepage");
	return 0;
}
