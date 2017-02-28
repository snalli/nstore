#!/usr/bin/env python

import argparse
import sys
import os
from subprocess import Popen, PIPE

sim_sizes = {'test':'./nstore -x100 -k1000 -e1 -w  -p0.5 -i', 
            'small': './nstore -x1000 -k10000 -e2 -w -p0.5 -i -n',
            'medium': './nstore -x20000 -k50000 -e2 -w -p0.5 -i -n ',
            'large' : './nstore -x8000000 -k100000 -e4 -w -p0.8 -i -n'}

def runCmd(cmd, err, out):
    """
    Takes two strings, command and error, runs it in the shell
    and then if error string is found in stdout, exits.
    For no output = no error, use err=""
    """
    print cmd
    (stdout, stderr) = Popen(cmd, shell=True, stdout=PIPE).communicate()
    if err is None:
        if stdout != "":
            print "Error: %s" %(out,)
            print "Truncated stdout below:"
            print '... ', stdout[-500:]
            sys.exit(2)
    else:
        if err in stdout:
            print "Error: %s" %(out,)
            print "Truncated stdout below:"
            print '... ', stdout[-500:]
            sys.exit(2)

def main(argv): 
    """
    Parses the arguments and cleans and/or builds the specified
    workloads of the whisper suite
    """
    parser = argparse.ArgumentParser(description='Runs nstore workloads from the whisper suite.')
    parser.add_argument('benchmarks', metavar='workload', type=str, nargs=1,
                help='workloads to be run: tpcc/ycsb')
    parser.add_argument('--sim_size', dest='sim_size', action='store', 
            default='test', help='Simulation size: test, small, medium, large')

    args = parser.parse_args()
    print 'Running a ' + str(args.sim_size) + ' simulation for ' + str(args.benchmarks)
    
    if args.benchmarks == 'ycsb':
        variant = ' -y'
    else:
        variant = ' -t'

    os.chdir('src/')
    if args.sim_size in sim_sizes:
        runCmd(sim_sizes[args.sim_size] + variant, "Error", "Simulation failed")    
    os.chdir('../')

if __name__ == "__main__":
    main(sys.argv[1:])
