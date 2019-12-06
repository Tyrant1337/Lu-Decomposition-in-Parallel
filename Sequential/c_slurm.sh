#!/bin/bash
##############################################################################
# FILE: c_slurm
# DESCRIPTION:
#   Simple Slurm job command script for C/C++ code on Heracles
# AUTHOR: Manh and Iris
# LAST REVISED: June 2019
##############################################################################
### Job commands start here
### Display some diagnostic information
## echo '=====================JOB STARTING=========================='
#SBATCH --partition=interactive-cpu     ### Partition
## PartitionName=interactive-cpu Priority=50000 Shared=YES:2 DefaultTime=8:00:00 MaxTime=48:00:00 State=UP Nodes=node[2-17] MaxNodes=1 MaxCPUsPerNode=10 MaxMemPerNode=64000
#SBATCH --job-name=seqcode              ### Job Name
#SBATCH --output=slurm_output.%j        ### File in which to store job output
#SBATCH --error=slurm_error.%j          ### File in which to store job error messages
#SBATCH --time=0-00:01:00       ### Wall clock time limit in Days-HH:MM:SS
#SBATCH --nodes=1               ### Node count required for the job, default = 1
#SBATCH --ntasks-per-node=1     ### Nuber of tasks to be launched per Node, default = 1
#SBATCH --exclusive             ### no shared resources within a node
#SBATCH --mail-type=ALL                   ### email alert at start, end and abortion of execution
## SBATCH --mail-user=myemail ### send mail to this addres (uncommnet this line in case using email)
##  the code will be executed in the node chosen by slurm in the interctive-cpu partition
## it is not necessary to ssh to any node in order to run the code in a specific node
./lud_sequential "$@"
## the script receives the matrix size from keyboard
## execute this script like that
## sbatch c_slurm.sh size  ###   size is an integer number for the matrix size
echo '=====================JOB DIAGNOTICS========================'
date
echo -n 'This machine is ';hostname
echo -n 'My jobid is '; echo $SLURM_JOBID
echo 'My path is:'
echo $PATH
echo 'My job info:'
squeue -j $SLURM_JOBID
echo 'Machine info'
sinfo -s

