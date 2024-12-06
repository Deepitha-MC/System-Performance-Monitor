/**
 * Tony Givargis
 * Copyright (C), 2023
 * University of California, Irvine
 *
 * CS 238P - Operating Systems
 * main.c
 */

#include <signal.h>
#include "system.h"

/**
 * Needs:
 *   signal()
 */

static volatile int done;

static void
_signal_(int signum)
{
	assert( SIGINT == signum );

	done = 1;
}

double
cpu_util(const char *s)
{
	static unsigned sum_, vector_[7];
	unsigned sum, vector[7];
	const char *p;
	double util;
	uint64_t i;

	/*
	  user
	  nice
	  system
	  idle
	  iowait
	  irq
	  softirq
	*/

	if (!(p = strstr(s, " ")) ||
	    (7 != sscanf(p,
			 "%u %u %u %u %u %u %u",
			 &vector[0],
			 &vector[1],
			 &vector[2],
			 &vector[3],
			 &vector[4],
			 &vector[5],
			 &vector[6]))) {
		return 0;
	}
	sum = 0.0;
	for (i=0; i<ARRAY_SIZE(vector); ++i) {
		sum += vector[i];
	}
	util = (1.0 - (vector[3] - vector_[3]) / (double)(sum - sum_)) * 100.0;
	sum_ = sum;
	for (i=0; i<ARRAY_SIZE(vector); ++i) {
		vector_[i] = vector[i];
	}
	return util;
}

void network_stats(char *interface) {
    const char *const PROC_NET_DEV = "/proc/net/dev";
    FILE *net_file;
    char line[1024];
	int i;
    unsigned long received_bytes = 0, transmitted_bytes = 0;

    if (!(net_file = fopen(PROC_NET_DEV, "r"))) {
        TRACE("Error while opening the network file");
        return;
    }

    /* Skip the first two lines, which contain headers */ 
	for (i = 0; i < 2; ++i) {
		if (fgets(line, sizeof(line), net_file) == NULL) {
			fclose(net_file);
			TRACE("Error reading file header");
			return;
		}
	}

    /* Read each line corresponding to a network interface */ 
    while (fgets(line, sizeof(line), net_file)) {
        unsigned long r_bytes, t_bytes;
        char iface[32];

        /* Parse the line to extract the interface name and data */ 
        if (sscanf(line, "%31s %lu %*u %*u %*u %*u %*u %*u %*u %lu", iface, &r_bytes, &t_bytes) == 3) {
			if(strcmp(iface, interface) == 0) {
				received_bytes += r_bytes; /* Accumulate received bytes */ 
            	transmitted_bytes += t_bytes; /* Accumulate transmitted bytes */ 
			}
            
        }
    }

    fclose(net_file);

    /* Print the total transmitted and received data */ 
    printf(" | Total Received Bytes: %lu | Total Transmitted Bytes: %lu", received_bytes, transmitted_bytes);
}

void uptime_stats() {
	const char *const PROC_UPTIME = "/proc/uptime";
	FILE *uptime_file;
	char line[1024];
	float uptime, idle_time;

	if (!(uptime_file = fopen(PROC_UPTIME, "r"))) {
        TRACE("Error while opening the uptime file");
        return;
    } 
	
	while (fgets(line, sizeof(line), uptime_file)) {
        sscanf(line, "%f %f", &uptime, &idle_time);
	}

	fclose(uptime_file);
	printf(" | Total Uptime: %5.1fs | Idle Time: %5.1fs", uptime, idle_time);
}

void loadavg_stats() {
	const char *const PROC_LOADAVG = "/proc/loadavg";
	FILE *loadavg_file;
	char line[1024];
	int recent_pid, active_proc, total_proc;

	if (!(loadavg_file = fopen(PROC_LOADAVG, "r"))) {
        TRACE("Error while opening the load average file");
        return;
    }

	while (fgets(line, sizeof(line), loadavg_file)) {
        sscanf(line, "%*f %*f %*f %d/%d %d", &active_proc, &total_proc, &recent_pid);
	}

	fclose(loadavg_file);
	printf(" | Active/Total Processes: %d/%d | Recent PID: %d", active_proc, total_proc, recent_pid);
}

int
main(int argc, char *argv[])
{
	const char * const PROC_STAT = "/proc/stat";
	char line[1024];
	FILE *file;

	UNUSED(argc);
	UNUSED(argv);

	if (SIG_ERR == signal(SIGINT, _signal_)) {
		TRACE("signal()");
		return -1;
	}
	while (!done) {
		if (!(file = fopen(PROC_STAT, "r"))) {
			TRACE("fopen()");
			return -1;
		}
		if (fgets(line, sizeof (line), file)) {
			printf("\rCPU Utilization: %5.1f%%", cpu_util(line));
			fflush(stdout);
		}
		fclose(file);
		
		network_stats("lo:");
		loadavg_stats();
		uptime_stats();
		us_sleep(500000);
		
	}
	printf("\rDone!                                                                                                                                                                                                                                                                                                                                                                                      \n");
	return 0;
}
