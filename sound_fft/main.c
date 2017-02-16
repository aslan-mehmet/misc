/*
 * author: Mehmet ASLAN
 * date: February 16, 2017
 *
 * no warranty, no licence agreement
 * all modifications allowed, just state any changes
 * use it at your own risk
 */

 /*
  * Basic implementation of fftw
  * raw sound file created from audacity (raw, headerless, 16 bit signed pcm)
  * read as a regular file, take ftt
  * output magnitude and phase plotted by gnuplot
  */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fftw3.h>
#include <math.h>

//#define TEST_RAW_FILE
#define EXAMPLE
#define LEN 44100 // how many samples fft uses
#define OUT_LEN (LEN/2+1) // make sure len even number

int main()
{
        // lets test raw file
#ifdef TEST_RAW_FILE
        // open raw sound file, open another file for plotting
        FILE *sound, *plot;
        int16_t sample;

        // sampling frequency = 44100, duration = 10 sec, freq= 1kHz
        sound = fopen("sine_wave.raw", "rb");
        plot = fopen("sine_wave_plot", "w");

        if (sound == NULL || plot == NULL)
                return -1;

        // put one period of data
        for (uint8_t i = 0; i < 45; ++i) {
                fread(&sample, sizeof(sample), 1, sound);
                fprintf(plot, "%d\n", sample);
        }

        fclose(sound);
        fclose(plot);

        return 0;
#endif // TEST_RAW_FILE

#ifdef EXAMPLE
        // real value data fft
        FILE *sound, *comp, *mag, *phase;
        // put your sound file name that you want to analyze
        sound = fopen("sine_wave.raw", "rb");
        comp = fopen("out_comp", "w");
        mag = fopen("out_mag", "w");
        phase = fopen("out_phase", "w");

        if (sound == NULL || comp == NULL || mag == NULL || phase == NULL)
                return -1;

        // allocate memory
        double *real_input;
        fftw_complex *fft_output;

        real_input = (double *) malloc(sizeof(double) * LEN);
        fft_output = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * OUT_LEN);

        // create plan
        fftw_plan plan = fftw_plan_dft_r2c_1d(LEN, real_input, fft_output, FFTW_ESTIMATE);

        // assign input
        for (uint32_t i = 0; i < LEN; ++i) {
                int16_t sample;
                fread(&sample, sizeof(sample), 1, sound);
                real_input[i] = sample;
        }

        // execute
        fftw_execute(plan);

        // write fft output as: complex, mag, phase
        for (uint32_t i = 0; i < OUT_LEN; ++i) {
                double tmp, real, img;

                real = fft_output[i][0];
                img = fft_output[i][1];

                fprintf(comp, "%f i*%f\n", real, img);

                tmp = sqrt(real * real + img * img);
                tmp /= LEN; // library itself not dividing
                fprintf(mag, "%f\n", tmp);

                tmp = atan(img / real);
                fprintf(phase, "%f\n", tmp);
        }

        // free
        fftw_destroy_plan(plan);
        fftw_free(fft_output);
        free(real_input);
        // close files
        fclose(sound);
        fclose(comp);
        fclose(mag);
        fclose(phase);

        return 0;
#endif // EXAMPLE
}
