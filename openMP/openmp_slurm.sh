#!/bin/bash
##############################################################################
# FILE: openmp_slurm
# DESCRIPTION:
#   Simple Slurm job command script for openmp code on Heracles
# AUTHOR: Manh and Iris
# LAST REVISED: June 2019
##############################################################################
### Job commands start here
## echo '=====================JOB STARTING=========================='
#SBATCH --partition=day-long-cpu        ### Partition
## PartitionName=day-long-cpu Priority=20000 Default=NO MaxTime=1-0:00:00 State=UP Nodes=node[2-17]
#SBATCH --job-name=openmpcode              ### Job Name
#SBATCH --output=slurm_output.%j        ### File in which to store job output
#SBATCH --error=slurm_error.%j          ### File in which to store job error messages
#SBATCH --time=0-00:01:00       ### Wall clock time limit in Days-HH:MM:SS
#SBATCH --nodes=1               ### Node count required for the job, default = 1
#SBATCH --ntasks-per-node=1     ### Nuber of tasks to be launched per Node, default = 1
#SBATCH --cpus-per-task=48  ### Number of threads per task (OMP threads) and set SLURM_CPUS_PER_TASK
#SBATCH --exclusive             ### no shared resources within a node
#SBATCH --mail-type=ALL                   ### email alert at start, end and abortion of execution
## SBATCH --mail-user=myemail ### send mail to this address (uncommnet this line in case using email)
export OMP_PROC_BIND='close'
echo ' thread affinity/proc_bind = ' ; echo $OMP_PROC_BIND
## slurm will decide which node the code will run according to the partition ste in the job. No need ssh node on heracles
./lud_openMPpar "$@"
### Display some diagnostic information
echo '=====================JOB DIAGNOTICS========================'
date
echo -n 'This machine is ';hostname
echo -n 'My jobid is '; echo $SLURM_JOBID
echo 'My path is:' 
echo $PATH
sinfo -s
echo 'My job info:'
squeue -j $SLURM_JOBID
echo 'Machine info'
echo ' '
echo '========================ALL DONE==========================='
