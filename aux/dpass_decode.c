#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>

#include <jibal_gsto.h>

#define MAX_LINE_LEN 80

#define Z_MAX 92 /* This is the output Z_MAX. It doesn't have to be exactly the same as the Z_MAX of the database file, but expect warnings if it is too small and empty data (zeros) if it is too large. */

#define DBFILE "DPASS_DB.dat"
#define DPASS_URL "https://www.sdu.dk/en/DPASS"

#define DB_HEADER_BYTES 25
#define DB_ELEMENT_HEADERS_BYTES 3

typedef struct {
    FILE *f;
    long size;
    char *header_data;
    uint16_t n_points;
    int valid;
    int n_combs;
    double *E;  /* Energy per mass, or "E/n" in MeV/u, contains n_points values */
    double *s;
} dbfile;

dbfile load_dbfile(const char *filename) {
    dbfile file;
    file.valid=0;
    file.n_combs=0;
    file.f=fopen(filename, "r");
    int i;
    if(!file.f) {
        fprintf(stderr, "No such file: %s. Download DPASS from %s\n", filename, DPASS_URL);
        return file;
    }
    fseek(file.f, 0, SEEK_END);
    file.size = ftell(file.f);
    fprintf(stderr, "File is %li bytes long\n", file.size);
    rewind(file.f);
    file.header_data = malloc(sizeof(char)*DB_HEADER_BYTES);
    if(!file.header_data) {
        return file;
    }
    size_t s = fread(file.header_data, sizeof(char), DB_HEADER_BYTES, file.f);
    if(s != DB_HEADER_BYTES) {
        fprintf(stderr, "Error while reading!\n");
        return file;
    }
    fprintf(stderr, "Headers.\n i  x   u    i c\n");
    for(i=0; i < DB_HEADER_BYTES; i++) {
        fprintf(stderr, "%2i %2x %3u %+4i %c\n", i, (char)file.header_data[i] & 0xFF, (char)file.header_data[i] & 0xFF, file.header_data[i], file.header_data[i]>=32?file.header_data[i]:' ');
    }
    fputc('\n', stderr);
    file.n_points=*((uint16_t *)&file.header_data[23]);
    fprintf(stderr, "There are %"PRIu16" points in the file.\n", file.n_points);
/*    long expected=DB_HEADER_BYTES+(file.n_points*8)*(
    if(file.size != expected) {
        fprintf(stderr, "Wrong size, got %li, expected %li.\n", file.size, expected);
        return file;
    }*/
    file.E = malloc(sizeof(double)*file.n_points);
    s = fread(file.E, sizeof(double), file.n_points, file.f);
    if(s != file.n_points) {
        fprintf(stderr, "Error while reading!\n");
        return file;
    }

    file.s = calloc(Z_MAX*Z_MAX*file.n_points, sizeof(double));
    if(!file.s) {
        fprintf(stderr, "Could not allocate memory!\n");
        return file;
    }
    char headers[3];
    while(!feof(file.f)) {
#ifdef DEBUG
        long start = ftell(file.f);
#endif
        size_t elem = fread(&headers, sizeof(char), DB_ELEMENT_HEADERS_BYTES, file.f);
        if(elem != DB_ELEMENT_HEADERS_BYTES) {
            fprintf(stderr, "Error while reading!\n");
            break;
        }
        uint8_t unknown = *((uint8_t *)&headers[0]);
        if(unknown != 1) {
            fprintf(stderr, "I didn't expect this. The data format is not exactly what I expect.\n");
            exit(EXIT_FAILURE);
        }
        uint8_t Z1 = *((uint8_t *)&headers[1]);
        uint8_t Z2 = *((uint8_t *)&headers[2]);
#ifdef DEBUG
        fprintf(stderr, "Z1=%"PRIu8", Z2=%"PRIu8", byte=0x%x (%li), should be %i\n", Z1, Z2, (unsigned int)start, start, DB_HEADER_BYTES+file.n_points*8+((Z1-1)*Z_MAX+(Z2-1))*(file.n_points*8+DB_ELEMENT_HEADERS_BYTES));
#endif
        if(Z1 > Z_MAX || Z2 > Z_MAX) {
            fprintf(stderr, "Warning: Z1 or Z2 is larger than my hard coded Z_MAX=%i. Skipping!\n", Z_MAX);
            fseek(file.f, sizeof(double)*file.n_points, SEEK_CUR);
            continue;
        }
        int i=((Z1-1)*Z_MAX+(Z2-1))*file.n_points;
        elem = fread(&file.s[i], sizeof(double), file.n_points, file.f);
        if(elem != file.n_points) {
            fprintf(stderr, "Error while reading!\n");
            break;
        }
        file.n_combs++;
        if(Z1 == Z_MAX && Z2 == Z_MAX)
            break; /* Nothing more to read */
    }
    fprintf(stderr, "Read %i Z1, Z2 combinations. %li bytes read.\n", file.n_combs, ftell(file.f));
    fclose(file.f);
    file.valid=1;
    return file;
}




int main(int argc, char **argv) {
    fprintf(stderr, "I try to convert DPASS data to GSTO compatible format and print it to file.\n\n");
    if(argc != 2) {
        fprintf(stderr, "YOU MUST SUPPLY THE FILE NAME.\n\n");
    } else {
        fprintf(stderr, "The file will be called \"%s\".\n\n", argv[1]);
    }
    fprintf(stderr, "I work with the following assumptions:\n");
    fprintf(stderr, "1. The data is is a file called %s.\n", DBFILE);
    fprintf(stderr, "2. There should be %i header bytes.\n", DB_HEADER_BYTES);
    fprintf(stderr, "3. Bytes 23 and 24 encode the number of datapoints per Z1, Z2 combination.\n");
    fprintf(stderr, "4. The energies are log spaced (I'll check this) and the E/n are listed after the headers.\n");
    fprintf(stderr, "5. Stopping data are doubles too. I expect Z1 and Z2 to be in range of 1..%i\n", Z_MAX);
    fprintf(stderr, "   If there are more data, I will skip it and if there is less, I might output zeroes.\n");
    fprintf(stderr, "   This should result in %i Z1, Z2 combinations in output.\n\n", Z_MAX*Z_MAX);
     if(argc != 2) {
        fprintf(stderr, "Usage: dpass_decode <outfile>\n");
        return EXIT_FAILURE;
    }
    FILE *file_out=fopen(argv[1], "w");
    if(!file_out) {
        fprintf(stderr, "Couldn't open file %s.\n", argv[1]);
        return EXIT_FAILURE;
    }
    dbfile f = load_dbfile(DBFILE);
    if(!f.valid) {
        fprintf(stderr, "There were issues.\n");
        return EXIT_FAILURE;
    }
    int Z1, Z2, i;
    double E_min=f.E[0];
    double E_max=f.E[f.n_points-1];
    jibal_gsto_fprint_header_property(file_out, GSTO_HEADER_TYPE, GSTO_STO_ELE);
    jibal_gsto_fprint_header_string(file_out, GSTO_HEADER_SOURCE, "DPASS"); /* TODO: version? */
    jibal_gsto_fprint_header_int(file_out, GSTO_HEADER_Z1MIN, 1);
    jibal_gsto_fprint_header_int(file_out, GSTO_HEADER_Z1MAX, Z_MAX);
    jibal_gsto_fprint_header_int(file_out, GSTO_HEADER_Z2MIN, 1);
    jibal_gsto_fprint_header_int(file_out, GSTO_HEADER_Z2MAX, Z_MAX);
    jibal_gsto_fprint_header_property(file_out, GSTO_HEADER_STOUNIT, GSTO_STO_UNIT_EV15CM2);
    jibal_gsto_fprint_header_property(file_out, GSTO_HEADER_XUNIT, GSTO_X_UNIT_MEV_U);
    jibal_gsto_fprint_header_property(file_out, GSTO_HEADER_FORMAT, GSTO_DF_ASCII);
    jibal_gsto_fprint_header(file_out, GSTO_HEADER_XMIN, &E_min);
    jibal_gsto_fprint_header(file_out, GSTO_HEADER_XMAX, &E_max);
    jibal_gsto_fprint_header_property(file_out, GSTO_HEADER_XSCALE, GSTO_XSCALE_LOG10);
    jibal_gsto_fprint_header(file_out, GSTO_HEADER_XPOINTS, &f.n_points);
    fprintf(file_out, "\n");
    for(i=0; i < f.n_points; i++) {
        double E_read=f.E[i];
        double E_predicted=E_min*pow(E_max/E_min,1.0*i/(f.n_points-1));
        if((E_read-E_predicted)/E_predicted > 1e-6) {
            fprintf(stderr, "Warning: E/n from file (%e) deviates from expected value (%e)\n", E_read, E_predicted);
        }
        fprintf(stderr, "%3i: %e ", i, E_read);
        if(i%5 == 4) {fputc('\n', stderr);}
    }
    for(Z1=1; Z1 <= Z_MAX; Z1++) {
        for(Z2=1; Z2 <= Z_MAX; Z2++) {
            double *s = &f.s[((Z1-1)*Z_MAX+(Z2-1))*f.n_points];
            for(i=0; i < f.n_points; i++) {
                fprintf(file_out, "%e\n", s[i]);
            }
        }
    }
    return EXIT_SUCCESS; 
}
