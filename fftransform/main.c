/*
 * author: Mehmet ASLAN
 * date: February 24, 2017
 *
 * no warranty, no licence agreement
 * all modifications allowed, just state any changes
 * use it at your own risk
 */

#include <stdio.h>
#include <stdlib.h>
#include <fftw3.h>
#include <math.h>
#include <getopt.h>

int main(int argc, char **argv)
{
        int c = 0;
        FILE *input_file = NULL, *output_file = NULL;
        float ts = -1; // sampling interval in sec
        int fs = 0; // sampling frequency
        int n = 0; // number of samples
        int fn = 0; // total number of frequencies in spectogram
        float sig_dur = 0; // total sampled signal duration

        static struct option long_options[] =
        {
                {"input",       required_argument, NULL, 'i'},
                {"output",      required_argument, NULL, 'o'},
                {"fs",          required_argument, NULL, 'f'},
                {"ts",          required_argument, NULL, 't'},
                {"number",      required_argument, NULL, 'n'}
        };

        while ((c = getopt_long(argc, argv, "i:o:f:t:n:", long_options, NULL)) != EOF) {
                switch (c) {
                case 'i':
                        input_file = fopen(optarg, "rb");
                        break;
                case 'o':
                        output_file = fopen(optarg, "w");
                        break;
                case 'f':
                        sscanf(optarg, "%d", &fs);
                        break;
                case 't':
                        sscanf(optarg, "%f", &ts);
                        break;
                case 'n':
                        sscanf(optarg, "%d", &n);
                        break;
                }
        }

        if (input_file == NULL) {
                printf("input file could not opened\n");
                return -1;
        }

        if (output_file == NULL) {
                output_file = fopen("out.dat", "w");

                if (output_file == NULL) {
                        printf("out.dat could not opened\n");
                        return -2;
                }
        }

        if (fs == 0 && ts < 0) {
                printf("undefined sampling time\n");
                return -3;
        }

        if (ts <= 0) {
                if (fs <= 0) {
                        printf("error, invalid sampling freq = %d", fs);
                        return -4;
                }

                ts = (float)1.0 / fs;
        }

        if (n <= 0) {
                printf("error, no sample\n");
                return -5;
        }

        fn = n / 2 + 1;
        sig_dur = ts * n;

        double *in;
        fftw_complex *out;
        fftw_plan p;

        in = (double *) fftw_malloc(sizeof(double) * n);
        out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * fn);

        p = fftw_plan_dft_r2c_1d(n, in, out, FFTW_ESTIMATE);

        for (int i = 0; i < n; ++i) {
                float tmp;
                fread(&tmp, sizeof(float), 1, input_file);
                in[i] = (double) tmp;
        }

        fftw_execute(p);

        fprintf(output_file, "#frequency in hz, magnitude, phase\n");

        for (int i = 0; i < fn; ++i) {
                double img, real;
                real = out[i][0];
                img = out[i][1];

                fprintf(output_file, "%f %f %f\n", (float)i/sig_dur,
                        (float)sqrt(real * real + img * img), (float)atan(img / real));
        }

        fclose(input_file);
        fclose(output_file);

        printf("fast fourier transform successfull\n");
        return 0;
}
