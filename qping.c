#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <time.h>
#include "qping.h"

int parse_args(int argc, char** argv, char* url, struct prog_flags* pf) {
    // Parses given commandline arguments to set any appropriate options.
    // If no arguments are given, default values are used.
    //      -> Default URL is https://en.wikipedia.org/wiki/Cat
    // See help section of this function for all arguments that can be passed.
    // argc: number of arguments
    // argv: array of arguments
    // url: pointer to char array to store url in
    // pf: pointer to struct containing all program flags
    // Returns 0 if successful, 1 if an error occurred.
    // Has possibility to terminate program (i.e. using help argument).

    // Error Checking
    if (argc < 0 || argv == NULL || url == NULL) {
        fprintf(stderr, "Bad argument(s) given to argument parser.\n");

        return 1;
    }

    // Set defaults
    memcpy(url, DEFAULT_URL, sizeof(DEFAULT_URL));

    if (argc == 1) {
        // No args, return with default values
        return 0;
    }

    // Loop through the arguments and apply things as necessary
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            // Format: 2 spaces for arg, 4 spaces for extra info
            printf("\n< qping: Quick internet status checkup. >\n\n");

            printf("Connects to a given URL directly.\n");
            printf("Helps you see if your internet is out, or your browsers ");
            printf("are just being silly.\n");

            #ifdef QP_MODE
                printf("Usage: qp [OPTIONAL ARGUMENTS]\n\n");
            #else
                printf("Usage: qping [OPTIONAL ARGUMENTS]\n\n");
            #endif

            printf("Optional arguments:\n");

            printf("  -h: Display this help page.\n");

            printf("\n");

            printf("  -u [URL]: Set a custom URL to try connecting to.\n");
            printf("    ->Default is https://en.wikipedia.org/wiki/Cat.\n");

            printf("\n");

            printf("  -c: Sets to connect only mode; no packages are ");
            printf("sent/recieved\n");

            printf("\n");

            printf("  -t [timeout]: Sets a custom timeout for connecting ");
            printf("(in seconds)\n");
            printf("    ->Default is 5 seconds\n");
            printf("    ->Value given must be >= 0\n");

            exit(0);
        } else if (strcmp(argv[i], "-u") == 0) {
            // Check error case of no extra arg given as required
            if (i == argc - 1) {
                fprintf(stderr, "Incorrect use of argument.\n");
                fprintf(stderr, "\tUsage: -u [URL]\n\n");

                return 1;
            }

            i++;

            // Ensure it isn't an empty url
            if (strcmp(argv[i], "") == 0) {
                fprintf(stderr, "Empty URL given.\n");

                return 1;
            }

            memset(url, '\0', BUFFER_SIZE);
            memcpy(url, argv[i], strlen(argv[i]));
        } else if (strcmp(argv[i], "-c") == 0) {
            pf->connect_only = 1;
        } else if (strcmp(argv[i], "-t") == 0) {
            // Check error case of no extra arg given as required
            if (i == argc - 1) {
                fprintf(stderr, "Incorrect use of argument.\n");
                fprintf(stderr, "\tUsage: -t [timeout]\n\n");

                return 1;
            }

            i++;

            // Check that input is good
            if (!is_numeric(argv[i])) {
                fprintf(stderr, "Incorrect use of argument.\n");
                fprintf(stderr, "\tTimeout given must be numeric, and >= 0\n\n");

                return 1;
            }

            pf->timeout = strtol(argv[i], NULL, 10);
        } else {
            fprintf(stderr, "Unrecognised argument \"%s\".\n", argv[i]);
            
            #ifdef QP_MODE
                fprintf(stderr, "Please use qp -h for valid arguments.\n");
            #else
                fprintf(stderr, "Please use qping -h for valid arguments.\n");
            #endif

            return 1;
        }
    }

    return 0;
}

size_t write_check(void* ptr, size_t size, size_t n, void* userdata) {
    // Write function callback passed to curl for writing the webpage data;
    // puts the memory in the heap.

    struct data_pkg* d = (struct data_pkg*) userdata;
    void* heap_ptr = realloc(d->data, size * n);

    if (heap_ptr == NULL) {
        fprintf(stderr, "Realloc failed in write callback.\n");

        return 1;
    }

    d->data = heap_ptr;
    
    memcpy(heap_ptr, ptr, size * n);

    d->size = size * n;

    return size * n;
}

int main(int argc, char** argv) {
    // Program initialisation
    // Parameter defaults
    struct prog_flags pf;
    pf.connect_only = 0;
    pf.timeout = 5;

    char url[BUFFER_SIZE];

    if (parse_args(argc, argv, url, &pf) != 0) {
        fprintf(stderr, "Argument parsing failed.\n");

        return 1;
    }

    // Connecting initialisation
    if (curl_global_init(CURL_GLOBAL_ALL) != 0) {
        fprintf(stderr, "Failed curl initialisation.\n");

        return 1;
    }
    
    CURL* handle = curl_easy_init();

    if (handle == NULL) {
        fprintf(stderr, "Failed handle initialisation.\n");
        curl_global_cleanup();
        
        return 1;
    }

    if (curl_easy_setopt(handle, CURLOPT_URL, url) != CURLE_OK) {
        fprintf(stderr, "Failed to set URL for handle.\n");
        curl_easy_cleanup(handle);
        curl_global_cleanup();

        return 1;
    } else if (curl_easy_setopt(handle, CURLOPT_CONNECT_ONLY, 1L) != CURLE_OK) {
        fprintf(stderr, "Failed to set connect only option for handle.\n");
        curl_easy_cleanup(handle);
        curl_global_cleanup();

        return 1;
    } else if (curl_easy_setopt(handle, CURLOPT_TIMEOUT, pf.timeout) != CURLE_OK) {
        fprintf(stderr, "Failed to set timeout option for handle.\n");
        curl_easy_cleanup(handle);
        curl_global_cleanup();

        return 1;
    }

    // Do the connect
    time_t start = clock();
    time_t end;

    if (curl_easy_perform(handle) == CURLE_OK) {
        end = clock();
        int time_taken = ((end - start) * 1000) / CLOCKS_PER_SEC;

        printf("Successful connect to %s in %dms.\n", url, time_taken % 1000);
        
        if (pf.connect_only) {
            curl_easy_cleanup(handle);
            curl_global_cleanup();

            return 0;
        }

        // Now try receiving data
        // Reset, setup for receiving
        curl_easy_reset(handle);

        if (curl_easy_setopt(handle, CURLOPT_URL, url) != CURLE_OK) {
            fprintf(stderr, "Failed to set URL for handle.\n");
            curl_easy_cleanup(handle);
            curl_global_cleanup();

            return 1;
        }

        if (curl_easy_setopt(handle, CURLOPT_TIMEOUT, pf.timeout) != CURLE_OK) {
            fprintf(stderr, "Failed to set timeout option for handle.\n");
            curl_easy_cleanup(handle);
            curl_global_cleanup();

            return 1;
        }
        
        if (curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_check) != CURLE_OK) {
            fprintf(stderr, "Failed to set write function.\n");
            curl_easy_cleanup(handle);
            curl_global_cleanup();

            return 1;
        }

        struct data_pkg d;
        d.size = 0;
        d.data = malloc(1);

        if (curl_easy_setopt(handle, CURLOPT_WRITEDATA, &d) != CURLE_OK) {
            fprintf(stderr, "Failed to set write destination.\n");
            curl_easy_cleanup(handle);
            curl_global_cleanup();

            return 1;
        }

        if (curl_easy_setopt(handle, CURLOPT_USERAGENT, "chicken/1.0") != CURLE_OK) {
            fprintf(stderr, "Failed to set user agent.\n");
            curl_easy_cleanup(handle);
            curl_global_cleanup();

            return 1;
        }

        // Do the receive
        start = clock();

        if (curl_easy_perform(handle) == CURLE_OK) {
            end = clock();
            time_taken = ((end - start) * 1000) / CLOCKS_PER_SEC;

            printf("%lu bytes received in %dms\n", d.size, time_taken % 1000);
        } else {
            printf("\nConnected, but didn't download resource in required ");
            printf("%ld second timeout.\n", pf.timeout);
            printf("Consider checking your connection speed.\n");

            curl_easy_cleanup(handle);
            curl_global_cleanup();

            return 0;
        }

        free(d.data);
    } else {
        printf("Unsuccessful connect to %s.\n", url);
        printf("Consider checking your connection status.\n");
        curl_easy_cleanup(handle);
        curl_global_cleanup();

        return 0;
    }

    curl_easy_cleanup(handle);
    curl_global_cleanup();

    return 0;
}

// Supplementary functions
int is_numeric(char* str) {
    // Checks that all characters in a string are numbers (positive only).
    // str: string to check
    // Returns 1 if all numeric, 0 if not.
    
    // Error Checking
    if (str == NULL) {
        return 0;
    }
    
    for (int i = 0; i < strlen(str); i++) {
        if (str[i] < 48 || str[i] > 57) {
            return 0;
        }
    }

    return 1;
}


