// Author: Deven Bibikar
/* Note that all the items are stored on the heap so that multiple functions can access them,
	in an ideal case scenario, we run the file read procedure once we get a signal that a file is CHANGED;
	(or even better, we just have a thread updating values based on signals directly from the car)
	it's probably best run this state machine via multiple threads,
	i.e. to have (1) thread running that checks the file for changes, 
		another thread that updates each value, and maybe a lock for protecting / ensuring thread safety on those values 
		all in all, best done in RUST. 
	--> this code does not reflect multi-threading, but instead reads the values at the beginning 
		and runs a one-pass through the BMS State Machine */

#include <stdbool.h> 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Import Sleep package for easier viewability */ 
// Import packages specific to both WINDOWS
#define SLEEP true;
#ifdef _WIN32 || _WIN64	
#include <windows.h>
#endif

// Import packages specific to LINUX / MACOS
#ifdef __linux__ || __APPLE__	
#include <unistd.h>
#endif

/* Run fault sequence */
void run_fault_sequence() {
	printf("Running Fault Sequence:\n");
	printf("Disabling Tractive system\n");
	printf("Turning on BMS Indicator Light (RED)");
	printf("Turning on Tractive System Indicator Light");
}

/* Check current and set different kinds of cooling */
void turn_on_cooling_for_discharge(int current) {
	if (current <= 60) {
		printf("No cooling turned on (OFF)\n");
	} else if (current <= 120) {
		printf("Forced Air Cooling ON\n");
	} else if (current <= 180) {
		printf("10 sec. pulse, fuse limited.\n");
	}
}

/* Check conditions to see if each item is in acceptable range */
bool check_conditions(float temp, 
			float voltage, 
			float current, 
			bool fuse_OK, 
			bool overcurrent_OK) {

	printf("State: Checking all conditions of BMS \n");

	/* Check Temperature values */
	bool temp_OK = ((temp >= -20) && (temp <= 60));
	printf("Temperature: %s\n", temp_OK ? "OK" : "NOT OK");
	
	/* Check Volage value */
	bool voltage_OK = ((voltage >= 2.5) && (voltage <= 4.2));
	printf("Voltage: %s\n", voltage_OK ? "OK" : "NOT OK");
		
	/* Check current value */ 
	bool current_OK = (current <= 180);
	printf("Current: %s\n", current_OK ? "OK" : "NOT OK");
    turn_on_cooling_for_discharge(current);

	/* Calculate if all conditions OK & fault if not. */
	bool ok = (temp_OK && voltage_OK && current_OK && fuse_OK && overcurrent_OK);
	if (!ok) {
		printf("Not all conditions OK: FAULT occured.\n");
		return false;
	} 
	
	// All Checks succeeded, return true
	printf("All checked passed.\n");
	return true;
}

/* Helper function to read the line from the input file & into a pre-allocated buffer */
void read_line_into_buffer(char *buffer, FILE *file) {
	fgets(buffer, sizeof(buffer), file);
    buffer[strcspn(buffer, "\n")] = '\0';
}

/* Main running function, the body of the program */
int main() {

	/* Read the provided TEXT file to get in test inputs */
	FILE* file = fopen("input.txt", "r");
	if (file == NULL) {
		printf("Error: File Not Found. \n");
		return 1;
	}

	// Load in items to the HEAP from FILE.
	char *curr_state = malloc(sizeof(char) * 20);
    fgets(curr_state, sizeof(curr_state), file);
    curr_state[strcspn(curr_state, "\n")] = '\0';  // Remove newline

    char buffer[20];

    // Read temperature
	read_line_into_buffer(buffer, file);
    float *temp = malloc(sizeof(float));
    *temp = atof(buffer);

    // Read voltage
    read_line_into_buffer(buffer, file);
    float *voltage = malloc(sizeof(float));
    *voltage = atof(buffer);

    // Read current
   	read_line_into_buffer(buffer, file);
    float *current = malloc(sizeof(float));
    *current = atof(buffer);

    // Read fuse_OK
    read_line_into_buffer(buffer, file);
    bool *fuse_OK = malloc(sizeof(bool));
    *fuse_OK = (strcmp(buffer, "OK") == 0);

    // Read overcurrent_OK
    read_line_into_buffer(buffer, file);
    bool *overcurrent_OK = malloc(sizeof(bool));
    *overcurrent_OK = (strcmp(buffer, "OK") == 0);

    // Read shutdown_signal
    read_line_into_buffer(buffer, file);
    bool *shutdown_signal = malloc(sizeof(bool));
    *shutdown_signal = (strcmp(buffer, "TRUE") == 0);

	// Read shutdown_signal
    read_line_into_buffer(buffer, file);
    bool *accel_in_use = malloc(sizeof(bool));
    *accel_in_use = (strcmp(buffer, "TRUE") == 0);

	// Print all the file information;
	printf("FILE Information: \n");
	printf("Curr State: %s\n", curr_state);
	printf("Temp: %f\n", *temp);
	printf("Voltage: %f\n", *voltage);
	printf("Current: %f\n", *current);
	printf("Fuse: %s\n", *fuse_OK ? "OK" : "NOT OK");
	printf("Overcurrent: %s\n", *overcurrent_OK ? "OK" : "NOT OK");
	printf("Shutdown Signal: %s\n", *shutdown_signal ? "TRUE" : "FALSE");
	printf("Accelerator: %s\n", *accel_in_use ? "ON" : "OFF");

	/* LOOP the entire system forever until the USER quits the program 
	
		START either at DRIVE or IDLE
	*/
	printf("/**************************/\n");

	while (true) {
		printf("/**************************/\n");
		printf("Current State: %s\n", curr_state);
		printf("/**************************/\n");
		sleep(2); // Sleep so that we can witness changes across time. 

		if (strcmp(curr_state, "DRIVE") == 0) {
			bool ok = check_conditions(*temp, *voltage, *current, *fuse_OK, *overcurrent_OK);

			// if shutdown signal is hit, then switch the state. 
			// execute the correct type of shutdown
			if (*shutdown_signal) {
				printf("Shutdown signal triggered. Changing State to IDLE.\n");
				strcpy(curr_state, "IDLE"); // switch state to IDLE
			}

			// some fault occured, run fault sequence
			if (!ok) {
				run_fault_sequence();
				strcpy(curr_state, "IDLE"); // switch state to IDLE
			}

			// if accelerator is NOT in use, then switch to IDLE
			if (!*accel_in_use) {
				strcpy(curr_state, "IDLE"); // switch state to IDLE
			}
		}

		else if (strcmp(curr_state, "IDLE") == 0) {
			bool ok = check_conditions(*temp, *voltage, *current, *fuse_OK, *overcurrent_OK);

			// if accelerator IS IN USE, then switch to DRIVE
			if (*accel_in_use) {
				strcpy(curr_state, "DRIVE"); // switch state to DRIVE
			}

			// run the fault sequence if condition check failed. Do not switch to drive
			if (!ok) {
				run_fault_sequence(); 
			}
		}
	}
	
	// free all allocated variables (if needed)
	free(temp);
    free(voltage);
    free(current);
    free(fuse_OK);
    free(overcurrent_OK);
    free(shutdown_signal);

	return 0; 
}



