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

int main(int argc,char **argv) {
	Display *dpy;
	Window root;
	int vol,cpu_perc;
	long jif1,jif2,jif3,jift;
	long jif1l,jif2l,jif3l,jiftl;
	long bat_now,bat_full,bat_perc;
	long mtot,mfree,mbuff,mcache;
	char statnext[40], status[120];
	time_t current;
	FILE *audio, *memfile, *cpufile, *batfile;
	// get initial jiffies
	cpufile = fopen(CPU_FILE,"r");
	fscanf(cpufile,"cpu %ld %ld %ld %ld",&jif1l,&jif2l,&jif3l,&jiftl);
	fclose(cpufile);
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
		cpufile = fopen(CPU_FILE,"r");
		fscanf(cpufile,"cpu %ld %ld %ld %ld",&jif1,&jif2,&jif3,&jift);
		fclose(cpufile);
		if (jift>jiftl)
			cpu_perc = 100*((jif1-jif1l)+(jif2-jif2l)+(jif3-jif3l))/(jift-jiftl);
		else
			cpu_perc = 0;
		jif1l=jif1; jif2l=jif2; jif3l=jif3; jiftl=jift;
		if (cpu_perc > CPU_HI)
			sprintf(statnext,CPU_HI_STR,cpu_perc);
		else
			sprintf(statnext,CPU_STR,cpu_perc);
		strcat(status,statnext);
	// Memory use:
		memfile = fopen(MEM_FILE,"r");
		fscanf(memfile,"MemTotal: %ld kB\nMemFree: %ld kB\nBuffers: %ld kB\nCached: %ld kB\n",
			&mtot,&mfree,&mbuff,&mcache);
		fclose(memfile);
		sprintf(statnext,MEM_STR,100*mfree/mtot,100*mbuff/mtot,100*mcache/mtot);
		strcat(status,statnext);
	// Audio volume:
		audio = fopen(AUD_FILE,"r");
		fscanf(audio,"%d",&vol);
		fclose(audio);
		if (vol == -1)
			sprintf(statnext,VOL_MUTE_STR,vol);
		else
			sprintf(statnext,VOL_STR,vol);
		strcat(status,statnext);
	// Power / Battery:
		batfile = fopen(BATT_NOW,"r");
			fscanf(batfile,"%d\n",&bat_now);fclose(batfile);
		batfile = fopen(BATT_FULL,"r");
			fscanf(batfile,"%d\n",&bat_full);fclose(batfile);
		batfile = fopen(BATT_STAT,"r");
			fscanf(batfile,"%s\n",statnext);fclose(batfile);
		bat_perc = bat_now*100/bat_full;
		if (strncmp(statnext,"Charging",8) == 0) {
			sprintf(statnext,BAT_CHRG_STR,bat_perc);
		}
		else {
			if (bat_perc < BATT_LOW)
				sprintf(statnext,BAT_LOW_STR,bat_perc);
			else
				sprintf(statnext,BAT_STR,bat_perc);
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

