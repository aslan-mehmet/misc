compile:
	gcc -g main.c -l fftw3 -l m

plot_wave:
	gnuplot -persist -e "plot 'sine_wave.plot' with lines"
plot_mag:
	gnuplot -persist -e "plot 'out_mag' with lines"
plot_phase:
	gnuplot -persist -e "plot 'out_phase' with lines"

