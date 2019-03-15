#Python script to compute the hypothesis test (Welch's test):

# import necessary Python packages
import os
import numpy as np
from scipy import stats

# print message after packages imported successfully
print("Import of packages successful")

########################################################
########## EDIT THESE PATHS TO REFLECT YOURS ###########
########################################################
workspace = "/home/alberto/Desktop/Thesis/mARGOt/Core/myCore/core/build/workspace_beholder/tutorial/v3_online/foo/exec_time/1"
before_change_name = "before_change_residuals_alberto_Surface_Pro_2_17634"
after_change_name = "after_change_residuals_alberto_Surface_Pro_2_17634"

before_change_path = workspace + '/' + before_change_name + ".txt"
after_change_path =  workspace + '/' + after_change_name + ".txt"

# import the data as two numpy arrays
before_change_array = np.loadtxt(fname = before_change_path)
after_change_array = np.loadtxt(fname = after_change_path)

#print(before_change_array)
#print(after_change_array)

n1 = len(before_change_array)
n2 = len(after_change_array)

mean_1 = np.mean(before_change_array)
mean_2 = np.mean(after_change_array)

std_dev_1 = np.std(before_change_array)
std_dev_2 = np.std(after_change_array)

var_1 = np.var(before_change_array)
var_2 = np.var(after_change_array)

print ("Number of residuals before the change: " + str(n1))
print ("Number of residuals after the change: " + str(n2))

print ("Sample mean of residuals before the change: " + str(mean_1))
print ("Sample mean of residuals after the change: " + str(mean_2))

print ("Std deviation of residuals before the change: " + str(std_dev_1))
print ("Std deviation of residuals after the change: " + str(std_dev_2))

print ("Sample variance of residuals before the change: " + str(var_1))
print ("Sample variance of residuals after the change: " + str(var_2))

#t_score = stats.ttest_ind_from_stats(mean_1, std_dev_1, n1, mean_2, std_dev_2, n2, equal_var=False)
#print(t_score)

# compute Welch's test
# NB: the p-value returned here is the one to be compared directly with significance level alpha
# for the 2 sided (two-tailed) test.
# In other situations the p_value is computed for the 1 sided test. In that case it is required to compare it with alpha/2.
test = stats.ttest_ind(before_change_array,after_change_array, equal_var = False)

print (test)

# If the p-value is less than the significance level alpha, then we can conclude that our data
# is statistically significant and the null hypothesis will be rejected.
# Many statisticians typically use alpha = 0.05.
