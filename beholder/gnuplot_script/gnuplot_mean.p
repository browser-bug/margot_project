# Gnuplot script file for plotting the observed data points on which the beholder works
# and the computed ICI for the feature MEAN
#reset everything to start from scratch
reset
reset session
set   autoscale                        # scale axes automatically
unset log                              # remove any log-scaling
unset label                            # remove any previous labels
set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically

# set legend key left bottom
set key outside l b horizontal

# define workspace folder with trailing slash.
# It's the same as the beholder one but with the suffix "beholder/" added.
workspace = '../../build/beholder/'
# define app name
app_name = 'kmeans'
# define metric name
metric = 'exec_time'
# define counter suffix
suffix = 0

# define input files path
observations = workspace.app_name.'observations_'.metric.'_'.suffix.'.txt'


set terminal pdf size 10,10 enhanced font "Helvetica,15"
set output '/home/alberto/Desktop/Thesis/mARGOt/Test_Apps/POC/synthetic/v1/build/output.pdf'

#read the configuration file in order to plot the vertical lines
set term push
set term dumb
plot "/home/alberto/Desktop/Thesis/mARGOt/Test_Apps/POC/synthetic/v1/build/config.txt" index 0 every ::0::0 using (a1=$1, a2=$2, a3=$3, a4=$4, a5=$5, a6=$6, a7=$7, a8=$8, a9=$9, a10=$10, a11=$11, a12=$12):(0/0), ''
set term pop
show variables

#set plot title and axes names
set title sprintf("Synthetic application generated values with windows of size=%d, %d windows used for training,\n observations starting from %d, with a gaussian noise of mean 0 and std dev=%f, mean gamma=%d, variance gamma=%d", a3, a4, a10, a7, a8, a9)
set xlabel "Observations"
set ylabel "Value"

#set xrange [0:300]

#plot the vertical lines
#set for [i = 1:a2] arrow from a1*i, graph 0 to a1*i, graph 1 nohead lc rgb 'green'

#plot the vertical lines delimiting the windows for the training section
set for [i = 1:a4] arrow from a3*i, graph 0 to a3*i, graph 1 nohead lc rgb 'green' lw 1

#plot the vertical lines delimiting the windows for the operation section
set for [i = 1:a5] arrow from a3*(i+a4), graph 0 to a3*(i+a4), graph 1 nohead lc rgb '#DCDCDC' lw 1

#plot the two vertical lines to delimit the area where the change was detected (for mean)
if (a11 != 0 && a12 != 0){
  set arrow from a11, graph 0 to a11, graph 1 nohead lc rgb 'red' lw 3
  set arrow from a12, graph 0 to a12, graph 1 nohead lc rgb 'red' lw 3
}

#plot the data
#plot "/home/alberto/Desktop/Thesis/mARGOt/Test_Apps/POC/synthetic/v1/build/output.txt" using 1

#plot '/home/alberto/Desktop/Thesis/mARGOt/Test_Apps/POC/synthetic/v1/build/configICI.txt' using 1:2:3 with yerrorbars linestyle 1, \
     ''                        with lines linestyle 1

#plot CI as in paper
#plot "/home/alberto/Desktop/Thesis/mARGOt/Test_Apps/POC/synthetic/v1/build/output.txt" using 1, \
      '/home/alberto/Desktop/Thesis/mARGOt/Test_Apps/POC/synthetic/v1/build/configICI.txt' using 1:2:3 with yerrorbars linestyle 1 lc rgb 'yellow' lw 3 title "mean CI", \
#     ''                        with lines linestyle 1

#plot
#if (a6 == 0.0){
##plot w/o variance
plot "/home/alberto/Desktop/Thesis/mARGOt/Test_Apps/POC/synthetic/v1/build/output.txt" using 1 title "observations", \
     '/home/alberto/Desktop/Thesis/mARGOt/Test_Apps/POC/synthetic/v1/build/configICIlines.txt' u 1:2 w steps ls 2 title "lower bound of mean ICI", \
     '/home/alberto/Desktop/Thesis/mARGOt/Test_Apps/POC/synthetic/v1/build/configICIlines.txt' u 1:3 w steps ls 3 title "upper bound of mean ICI", \
     '/home/alberto/Desktop/Thesis/mARGOt/Test_Apps/POC/synthetic/v1/build/configICIlines.txt' u 1:4 w steps ls 4 lc rgb 'black' title "mean", \
     1/0 lc rgb 'green' lw 1 t "Training windows", \
     1/0 lc rgb '#DCDCDC' lw 1 t "Production windows", \
     1/0 lc rgb 'red' lw 3 t "Change window for mean"
#     '/home/alberto/Desktop/Thesis/mARGOt/Test_Apps/POC/synthetic/v1/build/configICI.txt' using 1:2:3 with yerrorbars linestyle 1 lc rgb 'yellow' lw 3 title "CI"
#} else {
#plot w/ variance
#plot "/home/alberto/Desktop/Thesis/mARGOt/Test_Apps/POC/synthetic/v1/build/output.txt" using 1 title "observations", \
     '/home/alberto/Desktop/Thesis/mARGOt/Test_Apps/POC/synthetic/v1/build/configICIlines.txt' u 1:2 w steps ls 2 title "lower bound of mean ICI", \
     '/home/alberto/Desktop/Thesis/mARGOt/Test_Apps/POC/synthetic/v1/build/configICIlines.txt' u 1:3 w steps ls 3 title "upper bound of mean ICI", \
     '/home/alberto/Desktop/Thesis/mARGOt/Test_Apps/POC/synthetic/v1/build/configICIlines.txt' u 1:4 w steps ls 4 lc rgb 'black' title "mean", \
     '/home/alberto/Desktop/Thesis/mARGOt/Test_Apps/POC/synthetic/v1/build/configICIlines.txt' u 1:5 w steps ls 5 title "lower bound of variance ICI", \
     '/home/alberto/Desktop/Thesis/mARGOt/Test_Apps/POC/synthetic/v1/build/configICIlines.txt' u 1:6 w steps ls 6 title "upper bound of variance ICI", \
     '/home/alberto/Desktop/Thesis/mARGOt/Test_Apps/POC/synthetic/v1/build/configICIlines.txt' u 1:7 w steps ls 7 title "variance", \
     1/0 lc rgb 'green' lw 1 t "Training windows", \
     1/0 lc rgb '#DCDCDC' lw 1 t "Production windows", \
     1/0 lc rgb 'red' lw 3 t "Change window for mean"
#     '/home/alberto/Desktop/Thesis/mARGOt/Test_Apps/POC/synthetic/v1/build/configICI.txt' using 1:2:3 with yerrorbars linestyle 1 lc rgb 'yellow' lw 3 title "CI"
#}
