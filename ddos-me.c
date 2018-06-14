/*
 * Copyright (C) 2018 - Josh Arenson
 * Author: Josh Arenson <josharenson@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2 or version 3 of the License.
 * See http://www.gnu.org/copyleft/lgpl.html the full text of the license.
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

#include "thpool.h"

volatile int success = 0;

void curl_to_dev_null(char *url)
{
    CURL *curl;
    CURLcode res;
    FILE *fp;

    char output_filename[FILENAME_MAX] = "/dev/null";
    curl = curl_easy_init();

    if(curl) {
        fp = fopen(output_filename, "wb");

        if (fp == NULL) {
            printf("Couldn't open /dev/null for writing");
            return;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        fclose(fp);

        switch(res) {
            case CURLE_OK:
                success++;
                break;
            case CURLE_HTTP_RETURNED_ERROR:
                printf("CUrl returned an http status > 400!\n");
                break;
            case CURLE_OPERATION_TIMEDOUT:
                printf("The CUrl operation timed out!\n");
                break;
            default:
                printf("Some kind of CUrl error occurred! ERNO:%d\n", res);
        }

    } else {
        printf("Failed to initialize curl\n");
    }
}

int main(int argc, char* argv[])
{
    if (argc != 4) {
        printf("USAGE: %s url num_downloads num_threads", argv[0]);
        return 1;
    }

    char *url = argv[1];
    int num_downloads = atoi(argv[2]);
    int num_threads = atoi(argv[3]);

    printf("Initializing %s \n\tto download %s \n\twith %d downloads and \n\t%d threads\n",
           argv[0], url, num_downloads, num_threads);

    int i;
    threadpool thpool = thpool_init(num_threads);
    for (i = 0; i < num_downloads; i++) {
        thpool_add_work(thpool, (void*)curl_to_dev_null, url);
    }

    thpool_wait(thpool);
    thpool_destroy(thpool);

    printf("Successfully downloaded %d files\n", success);

    return 0;
}

