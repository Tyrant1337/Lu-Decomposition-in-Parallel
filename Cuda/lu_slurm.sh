#!/bin/bash
##############################################################################
# FILE: gpu_slurm
# DESCRIPTION:
#   Simple Slurm job command script for gpu on Heracles
# AUTHORS: Manh and Iris
# LAST REVISED: June/2019
##############################################################################
### Job commands start here
## echo '=====================JOB STARTING=========================='
### CHANGE THE LINES BELOW TO SELECT DIFFERENT email, output file,, memory size and executable code
#SBATCH --job-name=gpucode              ### Job Name
#SBATCH --partition=short-gpu           ### do not change this parameter
## PartitionName=short-gpu Priority=40000 Default=NO MaxTime=30:00 State=UP Nodes=node[18]
#SBATCH --mem=30G                       ### request max memory
#SBATCH --output=slurm_output.%j        ### File in which to store job output
#SBATCH --error=slurm_error.%j          ### File in which to store job error messages
#SBATCH --gres=gpu:P100_SXM:4           ### max number of GPUs - do not change
# SBATCH --mail-type=ALL     ## email alert at start, end and abortion of execution (uncomment this line if use email)
# SBATCH --mail-user=michael.k.chen@ucdenver.edu ## send mail to this address (uncomment this line if use email)
/usr/local/cuda-8.0/bin/nvprof ./lu "$@"
## the script receives the matrix size from keyboard
## execute this script like that
## sbatch gpu_slurm.sh size  ###   size is an integer number for the matrix size
### Display some diagnostic information
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
echo '========================ALL DONE==========================='
