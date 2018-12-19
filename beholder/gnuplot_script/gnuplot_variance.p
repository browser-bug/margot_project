# Gnuplot script file for plotting the computed ICI for the feature VARIANCE
# Reset everything to start from scratch:
reset
reset session
set   autoscale                        # scale axes automatically
unset log                              # remove any log-scaling
unset label                            # remove any previous labels
set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically

# Set legend key left bottom
set key outside l b horizontal
# Disable the subscript (if any)
# set key noenhanced

# *****************************************************
# USER-DEFINED PARAMETERS
# used to point to input/output files:

# Define workspace folder with trailing slash.
# It's the same as the beholder one:
workspace = '../../build/'
# Define app name (in the format: app_name/version/block_name/)
app_name = 'kmeans/v1/main_kmeans/'
# Define metric name
metric = 'exec_time'
# Define counter suffix
suffix = 1
# *****************************************************

# Compute the input files path:
observations = workspace.'workspace_beholder/'.app_name.metric.'/'.suffix.'/observations_'.metric.'_'.suffix.'.txt'
ici = workspace.'workspace_beholder/'.app_name.metric.'/'.suffix.'/ici_'.metric.'_'.suffix.'.txt'
configuration = workspace.'workspace_beholder/'.app_name.'configTestWindows.txt'

# Compute the output file path:
output_folder = workspace.'workspace_beholder/'.app_name.metric.'/'.suffix.'/ici_variance_'.metric.'_'.suffix.'.pdf'

# Set output terminal:
set terminal pdf size 10,10 enhanced font "Helvetica,15"
set output output_folder

# Read the configuration file in order to read the variables used to plot the vertical lines:
#a1 = window_size (number of observations in each window)
#a2 = training_windows (number of windows used for training)
set term push
set term dumb
plot configuration index 0 every ::0::0 using (a1=$1, a2=$2):(0/0), ''
set term pop
show variables

# Get the maximum index of the ICI windows
# I exploit the fact that the index of the ICI window is always increasing,
# so I take the maximum value and I know I got the index of the last ICI window.
set term push
set term dumb
plot ici using 1
plot[GPVAL_DATA_X_MAX:] ici using 1
set term pop
show variable GPVAL_DATA_Y_MAX

# Now I remove from the index of the last ICI window the range of values corresponding
# to the training phase to obtain the corrected index:
last_ici_index_corrected = GPVAL_DATA_Y_MAX - (a1*a2)

# Now I compute the total number of operational phase ICI windows by dividing the index
# of the last window (corrected) by the size of each window.
# I use the ceiling value to round.
operational_windows = ceil(last_ici_index_corrected/a1)

show variables

# Set plot title (noenhanced to avoid subscript when using underscore in the paths) and axes names:
set title noenhanced sprintf("Plot of the ICI curve for the feature VARIANCE for application: %s, metric under analysis: %s, ICI CDT iteration: %d.\nWindows of size=%d with %d windows used for training and %d production phase windows gathered.", app_name, metric, suffix, a1, a2, operational_windows)
set xlabel "Observation"
set ylabel "Residual value"

#set xrange [0:300]

#plot the vertical lines delimiting the windows for the training section
set for [i = 1:a2] arrow from a1*i, graph 0 to a1*i, graph 1 nohead lc rgb 'green' lw 1

#plot the vertical lines delimiting the windows for the operation section
set for [i = 1:operational_windows] arrow from a1*(i+a2), graph 0 to a1*(i+a2), graph 1 nohead lc rgb '#DCDCDC' lw 1

#plot
plot ici u 1:5 w steps ls 2 title "lower bound of variance ICI", \
     ici u 1:6 w steps ls 3 title "upper bound of variance ICI", \
     ici u 1:7 w steps ls 4 lc rgb 'black' title "variance", \
     1/0 lc rgb 'green' lw 1 t "Training windows (vertical lines)", \
     1/0 lc rgb '#DCDCDC' lw 1 t "Production windows (vertical lines)"
   # 1/0 lc rgb 'red' lw 3 t "Change window for variance"
