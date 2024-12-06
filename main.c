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

static volatile int done;

/**
 * Signal handler for SIGINT that sets a flag to exit the main loop.
 */
static void
_signal_(int signum)
{
	assert(SIGINT == signum);

	done = 1;
}

/**
 * Calculates CPU utilization based on data from /proc/stat.
 * 
 * @param s Pointer to the string containing CPU statistics.
 * @return CPU utilization percentage as a double.
 */
double
cpu_util(const char *s)
{
	static unsigned sum_, vector_[7];
	unsigned sum, vector[7];
	const char *p;
	double util;
	uint64_t i;

	/* Parse CPU statistics line and extract values into vector. */
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

	/* Calculate total CPU time. */
	sum = 0.0;
	for (i = 0; i < ARRAY_SIZE(vector); ++i) {
		sum += vector[i];
	}

	/* Calculate CPU utilization using difference from previous values. */
	util = (1.0 - (vector[3] - vector_[3]) / (double)(sum - sum_)) * 100.0;

	/* Update stored values for next calculation. */
	sum_ = sum;
	for (i = 0; i < ARRAY_SIZE(vector); ++i) {
		vector_[i] = vector[i];
	}
	return util;
}

/**
 * Reads and displays network statistics (received and transmitted bytes)
 * for the specified network interface from /proc/net/dev.
 * 
 * @param interface Pointer to the string containing the network interface name.
 */
void network_stats() {
    const char *const PROC_NET_DEV = "/proc/net/dev";
    FILE *net_file;
    char line[1024];
	int i;
    unsigned long received_bytes = 0, transmitted_bytes = 0;

    /* Open the network file. */
    if (!(net_file = fopen(PROC_NET_DEV, "r"))) {
        TRACE("Error while opening the network file");
        return;
    }

    /* Skip the first two lines, which contain headers. */ 
	for (i = 0; i < 2; ++i) {
		if (fgets(line, sizeof(line), net_file) == NULL) {
			TRACE("Error reading file header");
			fclose(net_file);
			return;
		}
	}

    /* Read and parse each line corresponding to a network interface. */ 
    while (fgets(line, sizeof(line), net_file)) {
        unsigned long r_bytes, t_bytes;
        char iface[32];

        /* Extract interface name and statistics. */ 
        if (sscanf(line, "%31s %lu %*u %*u %*u %*u %*u %*u %*u %lu", iface, &r_bytes, &t_bytes) == 3) {
			/* Accumulate statistics only for the specified interface. */
			received_bytes += r_bytes;
			transmitted_bytes += t_bytes;
        }
    }

    if (fclose(net_file) != 0) {
        TRACE("Error closing network file");
    }

    /* Print the total transmitted and received data. */ 
    printf(" | Total Received Bytes: %lu | Total Transmitted Bytes: %lu", received_bytes, transmitted_bytes);
}

/**
 * Reads and displays system uptime and idle time from /proc/uptime.
 */
void uptime_stats() {
	const char *const PROC_UPTIME = "/proc/uptime";
	FILE *uptime_file;
	char line[1024];
	float uptime, idle_time;

	/* Open the uptime file. */
	if (!(uptime_file = fopen(PROC_UPTIME, "r"))) {
        TRACE("Error while opening the uptime file");
        return;
    } 
	
	/* Parse uptime and idle time from the file. */
	while (fgets(line, sizeof(line), uptime_file)) {
        if (sscanf(line, "%f %f", &uptime, &idle_time) != 2) {
            TRACE("Error parsing uptime data");
        }
	}

    if (fclose(uptime_file) != 0) {
        TRACE("Error closing uptime file");
    }

	printf(" | Total Uptime: %5.1fs | Idle Time: %5.1fs", uptime, idle_time);
}

/**
 * Reads and displays system load averages, active processes,
 * total processes, and the most recent PID from /proc/loadavg.
 */
void loadavg_stats() {
	const char *const PROC_LOADAVG = "/proc/loadavg";
	FILE *loadavg_file;
	char line[1024];
	int recent_pid, active_proc, total_proc;

	/* Open the load average file. */
	if (!(loadavg_file = fopen(PROC_LOADAVG, "r"))) {
        TRACE("Error while opening the load average file");
        return;
    }

	/* Parse active processes, total processes, and recent PID. */
	while (fgets(line, sizeof(line), loadavg_file)) {
        if (sscanf(line, "%*f %*f %*f %d/%d %d", &active_proc, &total_proc, &recent_pid) != 3) {
            TRACE("Error parsing loadavg data");
        }
	}

    if (fclose(loadavg_file) != 0) {
        TRACE("Error closing load average file");
    }

	printf(" | Active/Total Processes: %d/%d | Recent PID: %d", active_proc, total_proc, recent_pid);
}

/**
 * Main function that periodically displays CPU utilization, 
 * network statistics, uptime, and load averages until interrupted.
 */
int
main(int argc, char *argv[])
{
	const char * const PROC_STAT = "/proc/stat";
	char line[1024];
	FILE *file;

	UNUSED(argc);
	UNUSED(argv);

	/* Set up signal handler for SIGINT. */
	if (SIG_ERR == signal(SIGINT, _signal_)) {
		TRACE("signal()");
		return -1;
	}

	/* Main loop: periodically gather and display system stats. */
	while (!done) {
		/* Open and read CPU statistics from /proc/stat. */
		if (!(file = fopen(PROC_STAT, "r"))) {
			TRACE("fopen()");
			return -1;
		}
		if (fgets(line, sizeof(line), file)) {
			printf("\rCPU Utilization: %5.1f%%", cpu_util(line));
			fflush(stdout);
		} else {
            TRACE("Error reading CPU stats");
        }
        if (fclose(file) != 0) {
            TRACE("Error closing /proc/stat file");
        }
		
		/* Gather and display other system stats. */
		network_stats();
		loadavg_stats();
		uptime_stats();
		
		/* Sleep for half a second. */
		us_sleep(500000);
	}
	
	/* Clean exit message after SIGINT is received. */
	printf("\rDone!                                                                                                                                                                                                                                                                                                                                                                                      \n");
	return 0;
}
