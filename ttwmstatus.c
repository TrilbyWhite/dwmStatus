
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

/* Below BATT_LOW% left on battery, the battery display turns red */
#define BATT_LOW	11
/* Above CPU_HI% cpu use, the cpu display turns red */
#define CPU_HI		50
/* Sleeps for INTERVAL seconds between updates */
#define INTERVAL	1

/* Files read for system info: */
#define CPU_FILE		"/proc/stat"
#define MEM_FILE		"/proc/meminfo"
#define BATT_NOW		"/sys/class/power_supply/BAT1/charge_now"
#define BATT_FULL		"/sys/class/power_supply/BAT1/charge_full"
#define BATT_STAT		"/sys/class/power_supply/BAT1/status"
/* Display format strings: */
#define CPU_STR			"{#dddddd}CPU: %d%% | "
#define CPU_HI_STR		"{#f04758}CPU: %d%% | "
#define MEM_STR			"{#dddddd}MEM: %d%%/%d%%/%ld%% | "
#define BAT_STR			"{#dddddd}BATT: %d%% | "
#define BAT_LOW_STR		"{#f04758}BATT: %d%% | "
#define BAT_CHRG_STR	"{#38912b}BATT: %d%% | "
#define DATE_TIME_STR	"{#258f8f}%a %b %d | %H:%M"

int main() {
	int num;
	long jif1,jif2,jif3,jift;
	long lnum1,lnum2,lnum3,lnum4;
	char str[40];
	time_t current;
	FILE *infile;
	/* get initial jiffies */
	infile = fopen(CPU_FILE,"r");
	fscanf(infile,"cpu %ld %ld %ld %ld",&jif1,&jif2,&jif3,&jift);
	fclose(infile);
/* MAIN LOOP STARTS HERE: */
	for (;;) {
	/* CPU use: */
		infile = fopen(CPU_FILE,"r");
		fscanf(infile,"cpu %ld %ld %ld %ld",&lnum1,&lnum2,&lnum3,&lnum4);
		fclose(infile);
		if (lnum4>jift)
			num = (int) 100*(((lnum1-jif1)+(lnum2-jif2)+(lnum3-jif3))/(lnum4-jift));
		else
			num = 0;
		jif1=lnum1; jif2=lnum2; jif3=lnum3; jift=lnum4;
		if (num > CPU_HI) printf(CPU_HI_STR,num);
		else printf(CPU_STR,num);
	/* Memory use: */
		infile = fopen(MEM_FILE,"r");
		fscanf(infile,"MemTotal: %ld kB\nMemFree: %ld kB\nBuffers: %ld kB\nCached: %ld kB\n",
			&lnum1,&lnum2,&lnum3,&lnum4);
		fclose(infile);
		printf(MEM_STR,100*lnum2/lnum1,100*lnum3/lnum1,100*lnum4/lnum1);
	/* Power / Battery: */
/*
		infile = fopen(BATT_NOW,"r");
			fscanf(infile,"%ld\n",&lnum1);fclose(infile);
		infile = fopen(BATT_FULL,"r");
			fscanf(infile,"%ld\n",&lnum2);fclose(infile);
		infile = fopen(BATT_STAT,"r");
			fscanf(infile,"%s\n",str);fclose(infile);
		num = lnum1*100/lnum2;
		if (strncmp(str,"Charging",8) == 0) {
			sprintf(str,BAT_CHRG_STR,num);
		}
		else {
			if (num < BATT_LOW) printf(BAT_LOW_STR,num);
			else printf(BAT_STR,num);
		}
*/
	/* Date & Time: */
		time(&current);
		strftime(str,38,DATE_TIME_STR,localtime(&current));
		printf("%s\n",str);
	/* sleep between loops */
		fflush(stdout);
		sleep(INTERVAL);
	}
	return 0;
}

