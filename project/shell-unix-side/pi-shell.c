// engler: trivial shell for our pi system.  it's a good strand of yarn
// to pull to motivate the subsequent pieces we do.
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "pi-shell.h"
#include "demand.h"
#include "../bootloader/unix-side/support.h"
#include "../bootloader/shared-code/simple-boot.h"

#define PI_BUF_SIZE 1024 // buffer size for message from pi

// have pi send this back when it reboots (otherwise my-install exits).
static const char pi_done[] = "PI REBOOT!!!";
// pi sends this after a program executes to indicate it finished.
static const char cmd_done[] = "CMD-DONE";
// other commands
const char reboot_cmd[] = "reboot\n";
const char esp_cmd[] = "esp\n";
const char esp_welcome_msg[] = "Welcome!";


/************************************************************************
 * provided support code.
 */
static void write_exact(int fd, const void *buf, int nbytes) {
        int n;
        if((n = write(fd, buf, nbytes)) < 0) {
		panic("i/o error writing to pi = <%s>.  Is pi connected?\n", 
					strerror(errno));
	}
        demand(n == nbytes, something is wrong);
}

// write characters to the pi.
static void pi_put(int fd, const char *buf) {
	int n = strlen(buf);
	demand(n, sending 0 byte string);
	write_exact(fd, buf, n);
}

// read characters from the pi until we see a newline.
int pi_readline(int fd, char *buf, unsigned sz) {
	for(int i = 0; i < sz; i++) {
		int n;
        if((n = read(fd, &buf[i], 1)) != 1) {
            note("got %s res=%d, expected 1 byte\n", strerror(n),n);
            note("assuming: pi connection closed.  cleaning up\n");
                exit(0);
        }
		if(buf[i] == '\n') {
			buf[i] = 0;
			return 1;
		}
	}
	panic("too big!\n");
}

// read characters from the pi until we see a newline.
int pi_CRLF_readline(int fd, char *buf, unsigned sz) {
	for(int i = 0; i < sz; i++) {
		int n;
        if((n = read(fd, &buf[i], 1)) != 1) {
            note("got %s res=%d, expected 1 byte\n", strerror(n),n);
            note("assuming: pi connection closed.  cleaning up\n");
                exit(0);
        }
	if(buf[i] == '\n') {
	  buf[i] = 0;
	  return 1;
	}
	}
	panic("too big!\n");
}

#define expect_val(fd, v) (expect_val)(fd, v, #v)
static void (expect_val)(int fd, unsigned v, const char *s) {
	unsigned got = get_uint(fd);
	if(v != got)
        panic("expected %s (%x), got: %x\n", s,v,got);
}

// print out argv contents.
static void print_args(const char *msg, char *argv[], int nargs) {
	note("%s: prog=<%s> ", msg, argv[0]);
	for(int i = 1; i < nargs; i++)
		note("<%s> ", argv[i]);
	note("\n");
}

// anything with a ".bin" suffix is a pi program.
static int is_pi_prog(char *prog) {
	int n = strlen(prog);

	// must be .bin + at least one character.
	if(n < 5)
		return 0;
	return strcmp(prog+n-4, ".bin") == 0;
}

/***********************************************************************
 * implement the rest.
 */

// catch control c: set done=1 when happens.  
static sig_atomic_t done = 0;
static void caught_control_c(int sig_no) {
    done = 1;
}
static void catch_control_c(void) {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = &caught_control_c;
    sigaction(SIGINT, &action, NULL);
}


// fork/exec/wait: use code from homework.
static int exit_code(int pid) {
    int status;
    waitpid(pid, &status, 0);
    return status;
}

static int do_unix_cmd(char *argv[], int nargs) {
    note("unix cmd: %s\n", argv[0]);

    // Fork
	int pid = 0;
    if ((pid = fork()) < 0) {
    	note("fork failed\n");
        return 0;
    }

    // For child process, execute
    if (!pid) {
        if (execvp(argv[0], argv) < 0) {
    	    note("child %d: unix cmd not executable\n", pid);
            exit(1);
        }
    }

    // For parent, wait
    int status = exit_code(pid);
    if (!WIFEXITED(status)) {
    	note("child %d: unix program terminated unexpectedly: %d\n", pid, status);
        return 0;
    } else {
    	//note("child %d: unix program exited with: %d\n", pid, status);
        return 1;
    }
}

int esp(int pi_fd, char *argv[], int nargs) {
    note("esp cmd\n");
	char buf[PI_BUF_SIZE];
    buf[0] = 0; // clean up

    // Send to pi to start ESP
    pi_put(pi_fd, esp_cmd);
    esp_note("Please reset your ESP8266...\n");

    // Read welcome message from ESP
    pi_readline(pi_fd, buf, PI_BUF_SIZE);
    if (strcmp(esp_welcome_msg, buf) != 0) {
        esp_note("Error... received %s from ESP.", buf);
        return -1;
    }
    esp_note("%s\n", buf);

    // ESP shell
    int pi_done = 0;
    esp_note("> ");
	while (!done && !pi_done && fgets(buf, sizeof buf, stdin)) {
        // Replace \n with \r\n at the end
		int n = strlen(buf);
        buf[n-1] = '\r';
        buf[n] = '\n';
        buf[n+1] = 0;
        n++;

        // Send to pi
        pi_put(pi_fd, buf);

        // Check if is end
        if (strncmp(buf, "exit", 4) == 0)
            break;

        // Read from pi
        while (pi_readline(pi_fd, buf, PI_BUF_SIZE)) {
	  if (strncmp(buf, "END", 3) == 0)
	    break;
	  note("pi echoed: <%s>\n", buf);
        }
        note("finish reading\n");
		esp_note("> ");
    }

    return 1;
}

static int do_esp(int pi_fd, char *argv[], int nargs) {
    if (nargs > 0) {
        if (strncmp(argv[0], "esp", 3) == 0)
            return esp(pi_fd, argv, nargs);
    }
    return 0;
}

static void send_prog(int fd, const char *name) {
	int nbytes;

    unsigned *code = (void*) read_file(&nbytes, name);
	assert(nbytes % 4 == 0);
	
    expect_val(fd, ACK);

    // Setup
    put_uint(fd, code[0]); // version
    put_uint(fd, code[1]); // addr 
    put_uint(fd, nbytes); // nbytes
    put_uint(fd, crc32(code, nbytes));
	
    expect_val(fd, ACK);

    // Send program
    send_binary(fd, code, nbytes);
    put_uint(fd, EOT);
    
    expect_val(fd, ACK);
}

// ship pi program to the pi.
static int run_pi_prog(int pi_fd, char *argv[], int nargs) {
    if (!is_pi_prog(argv[0]))
        return 0;
    note("pi program: %s\n", argv[0]);

    pi_put(pi_fd, "run\n");
    send_prog(pi_fd, argv[0]);

    // Read from pi
    char buf[PI_BUF_SIZE];
    if (pi_readline(pi_fd, buf, PI_BUF_SIZE)) {
        note("%s\n", buf);
    }

    return 1;
}

// run a builtin: reboot, echo, cd
int reboot(int pi_fd) {
    note("builtin cmd: reboot\n");

    // Send to pi
    pi_put(pi_fd, reboot_cmd);

    // Read from pi
	char buf[PI_BUF_SIZE];
    if (pi_readline(pi_fd, buf, PI_BUF_SIZE)) {
        note("%s\n", buf);
        if (strcmp(buf, pi_done) == 0) {
            note("pi rebooted.  shell done.\n");
            exit(0);
            return 1;
        }
    }
    return 0;
}

int echo(int pi_fd, char *argv[], int nargs) {
    note("builtin cmd: echo\n");
	char buf[PI_BUF_SIZE];
    buf[0] = 0; // clean up

    // Check if echoing something
    if (nargs < 2)
        return 0;
    
    // Send to pi
    int n = 0;
    for (int i = 0; i < nargs; i++) { // include the command
        strcpy(buf + n, argv[i]);
        n += strlen(argv[i]);
        buf[n] = ' '; // add back string separating spaces
        buf[n+1] = 0; // add back string separating spaces
        n++;
    }
    buf[n-1] = '\n'; // last char should be newline 
    pi_put(pi_fd, buf);

    // Read from pi
    if (pi_readline(pi_fd, buf, PI_BUF_SIZE)) {
        note("pi echoed: <%s>\n", buf);
        return 1;
    }
    return 0;
}

static int do_builtin_cmd(int pi_fd, char *argv[], int nargs) {
  note("buildin cmd\n");
    if (nargs > 0) {
        if (strncmp(argv[0], "echo", 4) == 0)
            return echo(pi_fd, argv, nargs);
        else if (strncmp(argv[0], "reboot", 6) == 0)
            return reboot(pi_fd);
    }
    return 0;
}

/*
 * suggested steps:
 * 	1. just do echo.
 *	2. add reboot()
 *	3. add catching control-C, with reboot.
 *	4. run simple program: anything that ends in ".bin"
 *
 * NOTE: any command you send to pi must end in `\n` given that it reads
 * until newlines!
 */
static int shell(int pi_fd, int unix_fd) {
	const unsigned maxargs = 32;
	char *argv[maxargs];
	char buf[8192];

	catch_control_c();

	// wait for the welcome message from the pi?  note: we 
	// will hang if the pi does not send an entire line.  not 
	// sure about this: should we keep reading til newline?
	note("> ");
	while(!done && fgets(buf, sizeof buf, stdin)) {
		int n = strlen(buf)-1;
		buf[n] = 0;

		int nargs = tokenize(argv, maxargs, buf);
		// empty line: skip.
		if(!nargs)
			;
		// is it a builtin?  do it.
		else if (do_builtin_cmd(pi_fd, argv, nargs))
            ;
        else if (do_esp(pi_fd, argv, nargs))
            ;
		// if not a pi program (end in .bin) fork-exec
		else if(!run_pi_prog(pi_fd, argv, nargs)) {
		    do_unix_cmd(argv, nargs);
        }
		note("> ");
	}

	if(done) {
		note("\ngot control-c: going to shutdown pi.\n");
        reboot(pi_fd);
	}
	return 0;
}

int main(void) {
	return shell(TRACE_FD_HANDOFF, 0);
}
