#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <jibal_config.h>
#include <inttypes.h>
#include <jibal.h>
#include <jibal_stop.h>
#include <jibal_defaults.h>

#ifdef WIN32
#include <jibal_registry.h>
#include <win_compat.h>
#else
#include <unistd.h>
#endif

#include "jibal_bootstrap.h"

char read_user_response(const char *question) {
    char *line=NULL;
    size_t line_size=0;
    //ssize_t linelen;
    char r=0;
    const char *template = "%s [yes/no]: ";
    const char *snarky_comebacks[]={"Please answer either yes or no.",
                                    "Yes or no. Or actually you can say exit too.",
                                    "Could you make up your mind. \"yes\", \"no\" or \"exit\". Easy.",
                                    "Let me spell you your options: Y E S or N O.",
                                    "There are two options. Yes or no. And exit. There are three options: yes, no and exit. Did you expect a Spanish Inquisition?",
                                    "NOBODY EXPECTS THE SPANISH INQUISITION (ha ha, same joke again)",
                                    "Oh come on! ENGLISH! YES OR NO. Well, don't shout or I won't listen to you.",
                                    "I've just about had it with you. Last warning.",
                                    "No, really.",
                                    "Fine! That's it.",
                                    NULL};
    int comeback=0;
    if(question) {
        fprintf(stderr, template, question);
    }
    while(getline(&line, &line_size, stdin) > 0) {
        line[strcspn(line, "\r\n")] = 0; /* Strip newlines */
        if(strcmp(line, "yes") == 0) {
            r = 'y';
        }
        if(strcmp(line, "maybe") == 0) {
            fprintf(stderr, "I take that as a yes.\n");
            r = 'y';
        }
        if(strcmp(line, "no") == 0) {
            r = 'n';
        }
        if(strcmp(line, "exit") == 0) {
            r = 'x';
        }
        if(strcmp(line, "quit") == 0) {
            r = 'x';
        }
        if(strcmp(line, "abort") == 0) {
            r = 'x';
        }
        if(r != 0)
            break;
        if(strlen(line) == 1) {
            fprintf(stderr, "Single letter response? What are you, an animal?\n");
        } else if(isupper(line[0])) {
            fprintf(stderr, "Please no upper case. I don't like capital letters.\n");
        } else {
            int i=((comeback%2==0)?0:comeback/2);
            fprintf(stderr, "%s\n", snarky_comebacks[i]);
            comeback++;
            if(!snarky_comebacks[i+1]) { /* Nothing clever left to say, exit */
                exit(EXIT_FAILURE);
            }
        }
        if(question) {
            fprintf(stderr, template, question);
        } else {
            fprintf(stderr, "[yes/no]: ");
        }

    }
    fprintf(stderr, "\n");
    free(line);
    switch (r) {
        case 'x':
            exit(EXIT_SUCCESS);
        case 0:
            exit(EXIT_FAILURE);
        default:
            return r;
    }
}

int bootstrap_write_user_config(jibal_config *config) {
    int retval=0;
    if(jibal_config_user_dir_mkdir_if_necessary() != 0) {
        char *user_dir=jibal_config_user_dir();
        fprintf(stderr, "Directory %s doesn't exist and can not be created. Try creating it manually. Now.\n", user_dir);
        free(user_dir);
        while(read_user_response("Are you ready to continue?") != 'y') {}; /* Infinite loop */
    }
    char *user_configfile=jibal_config_user_config_filename();
    FILE *config_out = fopen(user_configfile, "w");
    if(!config_out) {
        fprintf(stderr, "I can't write to that damn file.\n");
        retval = -1;
    } else {
        fprintf(stderr, "You are a brave soul. Writing.\n");
        jibal_config_file_write(config, config_out);
        fprintf(stderr, "Writing complete.\n");
        fclose(config_out);
    }
    return retval;
}

void bootstrap_make_blanks(const char *user_dir, const char *filename) { /* Silently creates empty files if they don't
 * exist, filename is relative to user_dir */
    char *path;
    asprintf(&path, "%s/%s", user_dir, filename);
    if(!path)
        return;
    path = jibal_path_cleanup(path);
    if(access(path, F_OK) == -1) { /* Doesn't exist */
        FILE *f=fopen(path, "w");
        if(f) {
            fclose(f);
        }
    }
    free(path);
}

int main(int argc, char *argv[]) {
    char r = 0;
    fprintf(stderr, "Welcome to Jibal user configuration bootstrap procedure, I will be your guide.\n\n");
    fprintf(stderr, "The rules are simple: I ask the questions and you answer.\n");
    fprintf(stderr, "The questions will be mostly yes/no questions. Let's start with a simple one.\n");
    r = read_user_response("Do you want to continue?");
    if(r != 'y') {
        fprintf(stderr, "Ok, maybe next time.\n");
        return EXIT_SUCCESS;
    }
    jibal_units *units = jibal_units_default();
    if(!units) {
        fprintf(stderr, "Bootstrapping failed because of issues with units. This is unheard of.\n");
        exit(EXIT_FAILURE);
    }
    jibal_config *config = jibal_config_init(units, NULL, FALSE); /* Initialize config without any configuration files */
    if(config->error) {
        fprintf(stderr, "Can't initialize configuration.\n");
        exit(EXIT_FAILURE);
    }
    char *user_dir = jibal_config_user_dir();
    if(!user_dir) {
        fprintf(stderr, "I can't figure out which directory to put data on your platform.\n");
        exit(EXIT_FAILURE);
    }
    char *user_configfile = jibal_config_user_config_filename();
    if(!user_configfile) {
        fprintf(stderr, "I can't figure out where to put a configuration file on your platform.\n");
        exit(EXIT_FAILURE);
    }

    if(config->files_file)
        free(config->files_file);
    if(config->assignments_file)
        free(config->assignments_file);
    config->files_file=strdup(JIBAL_FILES_FILE); /* Just default names, not so important. We want relative paths to user_dir! */
    config->assignments_file=strdup(JIBAL_ASSIGNMENTS_FILE);

    fprintf(stderr, "A blank user configuration would look like this (with my best guesses):\n\n");
    fprintf(stderr, "======================================================================\n");
    jibal_config_file_write(config, stderr);
    fprintf(stderr, "======================================================================\n");
    fprintf(stderr, "\nI could write it to %s, where Jibal could find it.\n", user_configfile);

    if(access(user_configfile, F_OK) == -1) { /* Doesn't exist, good */
        r = read_user_response("Do you want me to write this file?");
    } else { /* Exists */
        if(access(user_configfile, W_OK) == 0) {
            r = read_user_response("\nWARNING! VAROITUS! WARNUNG! VARNING!\nThe file exists!\nDo you want me to overwrite this file?");
        } else {
            fprintf(stderr, "\nThe file exists but is not writable. Can't do shit. :( :( Sorry.\n");
            r = 0;
        }
    }
    if(r == 'y') {
        if(bootstrap_write_user_config(config) == 0) { /* Success */
            bootstrap_make_blanks(user_dir, config->files_file);
            bootstrap_make_blanks(user_dir, config->assignments_file);
        }
    } else if (r != 0) {
        fprintf(stderr, "Ok, I didn't do anything, I swear! :) :)\n");
    }
    free(user_dir);
    free(user_configfile);
    jibal_units_free(units);
    jibal_config_free(config);
    fprintf(stderr, "Bootstrap complete. That's it.\n");
    r = read_user_response("Do you want to do it again?");
    if(r == 'y') {
        fprintf(stderr, "YESSS! Oh boy it will be much more fun the next round!\n\n\n");
        return main(argc, argv);
    } else {
        fprintf(stderr, "I can see why, it's not really that much fun. Come back again later! I'll be here for you if you need me.\n");
        exit(EXIT_SUCCESS);
    }
}
