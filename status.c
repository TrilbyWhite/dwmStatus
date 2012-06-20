#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <X11/Xlib.h>

// Below BATT_LOW% left on battery, the battery display turns red
#define BATT_LOW	11

// Above CPU_HI% cpu use, the cpu display turns red
//	Note, likely due to hyperthreading or some other thing I'm still learning
//  about, this can go well above "100%" during cpu-intensive processes.
#define CPU_HI		50

// Sleeps for INTERVAL seconds between updates
#define INTERVAL	1

// Files read for system info:
#define CPU_FILE		"/proc/stat"
#define MEM_FILE		"/proc/meminfo"
#define AUD_FILE		"/home/jmcclure/.status_info"
#define BATT_NOW		"/sys/class/power_supply/BAT1/charge_now"
#define BATT_FULL		"/sys/class/power_supply/BAT1/charge_full"
#define BATT_STAT		"/sys/class/power_supply/BAT1/status"
// Display format strings:
//  Defaults make extensive use of escape characters for colors which require
//  colorstatus patch.  There are also "extended" characters selected to work
//  with terminus2 font for symbols and trianlge "flags".
#define CPU_STR			"Ü	 Ï %d%% Ü"				// CPU percent when below CPU_HI%
#define CPU_HI_STR		"Ü Ï %d%% Ü"				// CPU percent when above CPU_HI%
#define MEM_STR			"Ü %d%% Ý %d%% Ý %ld%% Ü"	// memory, takes (up to) 3 integers: free, buffers, and cache
#define VOL_STR			"Ü	 Ô %d% Ü"				// volume when not muted  IMPORTANT! SEE NOTE IN README FOR AUDO INFO
#define VOL_MUTE_STR	"Ü	 Ô × Ü"					// volume when muted
#define BAT_STR			"Ü	 %d%% Ü"				// Battery, unplugged, above BATT_LOW%
#define BAT_LOW_STR		"Ü %d%% Ü"					// Battery, unplugged, below BATT_LOW% remaining
#define BAT_CHRG_STR	"Ü %d%% Ü"					// Battery, when charging (plugged into AC)
#define DATE_TIME_STR	"Ü %a %b %d ÜÜ Õ %H:%M "	// This is a strftime format string which is passed localtime

int main() {
	Display *dpy;
	Window root;
	int num;
	long jif1,jif2,jif3,jift;
	long lnum1,lnum2,lnum3,lnum4;
	char statnext[30], status[100];
	time_t current;
	FILE *infile;
	// get initial jiffies
	infile = fopen(CPU_FILE,"r");
	fscanf(infile,"cpu %ld %ld %ld %ld",&jif1,&jif2,&jif3,&jift);
	fclose(infile);
	// Setup X display and root window id:
	dpy=XOpenDisplay(NULL);
	if ( dpy == NULL) {
		fprintf(stderr, "ERROR: could not open display\n");
		exit(1);
	}
	root = XRootWindow(dpy,DefaultScreen(dpy));
// MAIN LOOP STARTS HERE:
	for (;;) {
		status[0]='\0';
	// CPU use:
		infile = fopen(CPU_FILE,"r");
		fscanf(infile,"cpu %ld %ld %ld %ld",&lnum1,&lnum2,&lnum3,&lnum4);
		fclose(infile);
		if (lnum4>jift)
			num = (int) 100*(((lnum1-jif1)+(lnum2-jif2)+(lnum3-jif3))/(lnum4-jift));
		else
			num = 0;
		jif1=lnum1; jif2=lnum2; jif3=lnum3; jift=lnum4;
		if (num > CPU_HI)
			sprintf(statnext,CPU_HI_STR,num);
		else
			sprintf(statnext,CPU_STR,num);
		strcat(status,statnext);
	// Memory use:
		infile = fopen(MEM_FILE,"r");
		fscanf(infile,"MemTotal: %ld kB\nMemFree: %ld kB\nBuffers: %ld kB\nCached: %ld kB\n",
			&lnum1,&lnum2,&lnum3,&lnum4);
		fclose(infile);
		sprintf(statnext,MEM_STR,100*lnum2/lnum1,100*lnum3/lnum1,100*lnum4/lnum1);
		strcat(status,statnext);
	// Audio volume:
		infile = fopen(AUD_FILE,"r");
		fscanf(infile,"%d",&num);
		fclose(infile);
		if (num == -1)
			sprintf(statnext,VOL_MUTE_STR,num);
		else
			sprintf(statnext,VOL_STR,num);
		strcat(status,statnext);
	// Power / Battery:
		infile = fopen(BATT_NOW,"r");
			fscanf(infile,"%ld\n",&lnum1);fclose(infile);
		infile = fopen(BATT_FULL,"r");
			fscanf(infile,"%ld\n",&lnum2);fclose(infile);
		infile = fopen(BATT_STAT,"r");
			fscanf(infile,"%s\n",statnext);fclose(infile);
		num = lnum1*100/lnum2;
		if (strncmp(statnext,"Charging",8) == 0) {
			sprintf(statnext,BAT_CHRG_STR,num);
		}
		else {
			if (num < BATT_LOW)
				sprintf(statnext,BAT_LOW_STR,num);
			else
				sprintf(statnext,BAT_STR,num);
		}
		strcat(status,statnext);
	// Date & Time:
		time(&current);
		strftime(statnext,38,DATE_TIME_STR,localtime(&current));
		strcat(status,statnext);
	// Set root name
		XStoreName(dpy,root,status);
		XFlush(dpy);
		sleep(INTERVAL);
	}
// NEXT LINES SHOULD NEVER EXECUTE, only here to satisfy my O.C.D.
	XCloseDisplay(dpy);
	return 0;
}

