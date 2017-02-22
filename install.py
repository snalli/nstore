#!/usr/bin/env python

import argparse
import sys
import os
from subprocess import Popen, PIPE

workload = 'nstore'

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
    parser = argparse.ArgumentParser(description='Builds echo from the whisper suite.')
    parser.add_argument('--clean', dest='clean', action='store_true',
                default='false',
                help='clean')
    parser.add_argument('--build', dest='build', action='store_true',
                default='false',
                help='build')

    args = parser.parse_args()

    # Find if bootstrap and configure need to be run first
    if not os.path.exists("./configured"):
        runCmd("./bootstrap", "Error", "Couldn't bootstrap")
        runCmd("./configure", "Error", "Couldn't configure")
        runCmd("touch ./configured", "Error", "Couldn't create configured file")

    if args.clean == True:
        print "Cleaning " + workload
        cleanCmd = "make clean"
        runCmd(cleanCmd, "No rule", "Couldn't clean %s dir!" % (workload, ))

    if args.build == True:
        print "Building " + workload
        buildCmd = "make -j8"
        runCmd(buildCmd, "No rule", "Couldn't build %s" % (workload,))

if __name__ == "__main__":
    main(sys.argv[1:])
