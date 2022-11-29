/* c_cc characters */
#define VINTR    0
#define VQUIT    1
#define VERASE   2
#define VKILL    3
#define VEOF     4
#define VTIME    5
#define VMIN     6
#define VSWTC    7
#define VSTART   8
#define VSTOP    9
#define VSUSP    10
#define VEOL     11
#define VREPRINT 12
#define VDISCARD 13
#define VWERASE  14
#define VLNEXT   15
#define VEOL2    16

/* c_iflag bits */
#define IGNBRK   0000001  /* Ignore break condition.  */
#define BRKINT   0000002  /* Signal interrupt on break.  */
#define IGNPAR   0000004  /* Ignore characters with parity errors.  */
#define PARMRK   0000010  /* Mark parity and framing errors.  */
#define INPCK    0000020  /* Enable input parity check.  */
#define ISTRIP   0000040  /* Strip 8th bit off characters.  */
#define INLCR    0000100  /* Map NL to CR on input.  */
#define IGNCR    0000200  /* Ignore CR.  */
#define ICRNL    0000400  /* Map CR to NL on input.  */
#define IUCLC    0001000  /* Map uppercase characters to lowercase on input
                (not in POSIX).  */
#define IXON     0002000  /* Enable start/stop output control.  */
#define IXANY    0004000  /* Enable any character to restart output.  */
#define IXOFF    0010000  /* Enable start/stop input control.  */
#define IMAXBEL  0020000  /* Ring bell when input queue is full
                (not in POSIX).  */
#define IUTF8    0040000  /* Input is UTF8 (not in POSIX).  */

/* c_oflag bits */
#define OPOST    0000001  /* Post-process output.  */
#define OLCUC    0000002  /* Map lowercase characters to uppercase on output.
                (not in POSIX).  */
#define ONLCR    0000004  /* Map NL to CR-NL on output.  */
#define OCRNL    0000010  /* Map CR to NL on output.  */
#define ONOCR    0000020  /* No CR output at column 0.  */
#define ONLRET   0000040  /* NL performs CR function.  */
#define OFILL    0000100  /* Use fill characters for delay.  */
#define OFDEL    0000200  /* Fill is DEL.  */

#define VTDLY    0040000  /* Select vertical-tab delays:  */
#define   VT0    0000000  /* Vertical-tab delay type 0.  */
#define   VT1    0040000  /* Vertical-tab delay type 1.  */

/* tcsetattr uses these.  */
#define	TCSANOW		0
#define	TCSADRAIN	1
#define	TCSAFLUSH	2

/* tcflush() and TCFLSH use these */
#define	TCIFLUSH	0
#define	TCOFLUSH	1
#define	TCIOFLUSH	2

typedef unsigned char cc_t;
typedef unsigned int speed_t;
typedef unsigned int tcflag_t;

#define NCCS 32
struct termios {
    tcflag_t c_iflag;    /* input mode flags */
    tcflag_t c_oflag;    /* output mode flags */
    tcflag_t c_cflag;    /* control mode flags */
    tcflag_t c_lflag;    /* local mode flags */
    cc_t c_line;         /* line discipline */
    cc_t c_cc[NCCS];     /* control characters */
    speed_t c_ispeed;    /* input speed */
    speed_t c_ospeed;    /* output speed */
#define _HAVE_STRUCT_TERMIOS_C_ISPEED 1
#define _HAVE_STRUCT_TERMIOS_C_OSPEED 1
};

speed_t cfgetospeed(const struct termios *__termios_p);
speed_t cfgetispeed(const struct termios *__termios_p);
int cfsetospeed(struct termios *__termios_p, speed_t __speed);
int cfsetispeed(struct termios *__termios_p, speed_t __speed);
int tcgetattr(int __fd, struct termios *__termios_p);
int tcsetattr(int __fd, int __optional_actions,
                const struct termios *__termios_p);
int tcflush(int __fd, int __queue_selector);
int cfmakeraw(struct termios *__termios_p);
