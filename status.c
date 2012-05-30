#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <X11/Xlib.h>

int main(int argc,char **argv) {
	Display *dpy;
	Window root;
	int scr, vol;
	long jif1,jif2,jif3,jift;
	long jif1l,jif2l,jif3l,jiftl;
	int cpu_perc;
	char statnext[40], status[120];
	time_t current;
	FILE *audio;
	FILE *memfile;
	FILE *cpufile;
	long mtot,mfree,mbuff,mcache;
	// get initial jiffies
	cpufile = fopen("/proc/stat","r");
	fscanf(cpufile,"cpu %ld %ld %ld %ld",&jif1l,&jif2l,&jif3l,&jiftl);
	fclose(cpufile);
	// Setup X display and root window id:
	dpy=XOpenDisplay(NULL);
	if ( dpy == NULL) {
		fprintf(stderr, "ERROR: could not open display\n");
		exit(1);
	}
	scr = DefaultScreen(dpy);
	root = XRootWindow(dpy,scr);
	for (;;) {
		status[0]='\0';
	// CPU use:
		cpufile = fopen("/proc/stat","r");
		fscanf(cpufile,"cpu %ld %ld %ld %ld",&jif1,&jif2,&jif3,&jift);
		fclose(cpufile);
		if (jift>jiftl)
			cpu_perc = 100*((jif1-jif1l)+(jif2-jif2l)+(jif3-jif3l))/(jift-jiftl);
		else
			cpu_perc = 0;
		jif1l=jif1; jif2l=jif2; jif3l=jif3; jiftl=jift;
		if (cpu_perc > 50)
			sprintf(statnext,"Ü Ï %d%% Ü",cpu_perc);
		else
			sprintf(statnext,"Ü	 Ï %d%% Ü",cpu_perc);
		strcat(status,statnext);
	// Memory use:
		memfile = fopen("/proc/meminfo","r");
		fscanf(memfile,"MemTotal: %ld kB\nMemFree: %ld kB\nBuffers: %ld kB\nCached: %ld kB\n",
			&mtot,&mfree,&mbuff,&mcache);
		fclose(memfile);
		sprintf(statnext,"Ü %d%% Ý %d%% Ý %ld%% Ü",
			100*mfree/mtot,100*mbuff/mtot,100*mcache/mtot);
		strcat(status,statnext);
	// Audio volume:
		audio = fopen("/home/USERNAME/.status_info","r");
		fscanf(audio,"%d",&vol);
		fclose(audio);
		if (vol == -1)
			sprintf(statnext,"Ü	 Ô × Ü",vol);
		else
			sprintf(statnext,"Ü	 Ô %d% Ü",vol);
		strcat(status,statnext);
	// Power / Battery:
		sprintf(statnext,"Ü Ü");
		strcat(status,statnext);
	// Date & Time:
		time(&current);
		strftime(statnext,38,"Ü %a %b %d ÜÜ Õ %H:%M ",localtime(&current));
		strcat(status,statnext);
	// Set root name
		XStoreName(dpy,root,status);
		XFlush(dpy);
		sleep(1);
	}
	XCloseDisplay(dpy);
	return 0;
}

