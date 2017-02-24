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

#define T 9 // total sampling time
#define N (44100 * T)
#define OUT_N (N/2 + 1)

int main()
{
        // created with audacity
        // total sampling time 10 sec
        // sampling freq 44100
        // headerless, float 32bit, raw file
        FILE *f = fopen("sine.raw", "rb");

        if (f == NULL) {
                printf("f can not opened\n");
                return -1;
        }

        FILE *period = fopen("period.dat", "w");

        if (f == NULL) {
                printf("what is the problem\n");
                return -1;
        }

        fprintf(period, "# in_second amplitude\n");

        for (int i = 0; i < 441; ++i) {
                float tmp;
                fread(&tmp, sizeof(float), 1, f);
                fprintf(period, "%f %f\n", (1.0/44100)*i, tmp);
        }

        double *in;
        fftw_complex *out;
        fftw_plan p;

        in = (double *) malloc(sizeof(double) * N);
        out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * OUT_N);

        p = fftw_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);

        if (fseek(f, 0, SEEK_SET)) {
                printf("fseek problem\n");
                return -1;
        }

        for (int i = 0; i < N; ++i) {
                float tmp;
                fread(&tmp, sizeof(float), 1, f);
                in[i] = (double) tmp;
        }

        fftw_execute(p);

        FILE *mag = fopen("mag.dat", "w");
        FILE *phase = fopen("phase.dat", "w");

        if (mag == NULL) {
                printf("mag can not opened\n");
                return -1;
        }

        if (phase == NULL) {
                printf("phase can not opened\n");
                return -1;
        }

        fprintf(mag, "#in_hz amplitude\n");
        fprintf(phase, "#in_hz amplitude\n");

        for (int i = 0; i < OUT_N; ++i) {
                double img, real;
                real = out[i][0];
                img = out[i][1];

                fprintf(mag, "%f %f\n", i/((double) T), sqrt(real * real + img * img));
                fprintf(phase, "%f %f\n", i/((double) T), atan(img / real));
        }

        fclose(mag);
        fclose(phase);
        fftw_destroy_plan(p);
        fftw_free(out);
        fclose(period);
        fclose(f);
        return 0;
}
