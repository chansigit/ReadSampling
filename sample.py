#!/usr/bin/python
#  -*- coding: utf-8 -*-
import sys
import getopt
import string
import time
import os
import random
import linecache

def print_help_info():
    # Display help info
    print '''  --input-file1,   -i:  specify pair-end file 1'''
    print '''  --input-file2,   -I:  specify pair-end file 2'''
    print '''  --output-dir,    -d:  specify output directory'''
    print '''  --output-prefix, -p:  specify output directory'''
    print '''  --help,          -h:  check help'''

def getline(thefilepath,line_num):
    # 如果文件比较大 使用下面
    if line_num < 1 :return ''
    for currline,line in enumerate(open(thefilepath,'rU')):
        if currline == line_num -1 : return line
    return ''

if __name__ == "__main__":
    # ------------------------------------------------
    #                Getting Parameters
    # ------------------------------------------------
    inputFile1   = ""
    inputFile2   = ""
    outputDir    = ""
    outputPrefix = ""
    ratio = 0.20
    sampleSize = 10
    shortargs    = "hi:I:d:p:r:n:"
    longargs     = ['input-file1=', 'input-file2=', 'output-dir=',
                    'output-prefix=', 'ratio=', 'sample-size=']
    opts, args   = getopt.getopt(sys.argv[1:], shortargs, longargs)
    for op, val in opts:
        if op in ('--input-file1', '-i'):
            inputFile1 = val
        elif op in ('--input-file2', '-I'):
            inputFile2 = val
        elif op in ('--output-dir', '-d'):
            outputDir = val
        elif op in ('--output-prefix', '-p'):
            outputPrefix = val
        elif op in ('--ratio','-r'):
            ratio = string.atof(val)
        elif op in ('--sample-size', '-n'):
            sampleSize = int(val)
        elif op in ('--help', '-h'):
            print_help_info()
            sys.exit(0)

    if (not os.path.isfile(inputFile1)) or (not os.path.isfile(inputFile2)):
        print "Input File Not Exist!"
        sys.exit(2)
    if not os.path.isdir(outputDir):
        print "Output Directory Not Exist!"
        sys.exit(2)

    if inputFile1!='' and inputFile2!='' and outputDir!=''  and outputPrefix!='':
        print "---------------Parameters------------------"
        print "input file1= %s" % (inputFile1)
        print "input file2= %s" % (inputFile2)
        print "output dir= %s" % (outputDir)
        print "outputPrefix= %s" % (outputPrefix)
        print "sampling ratio= %f" % (ratio)
        print "sample size= %d" % (sampleSize)
        print "--------------------------------------------"
    else:
        print_help_info()
        sys.exit()
    sys.stdout.flush()

    tottime_start = time.time()
    # ------------------------------------------------
    #                    Line Count
    # ------------------------------------------------
    lc1_start = time.time()
    if False:
        # ======= method 1
        count1 = -1
        for count1, line in enumerate(open(inputFile1, 'rU')):
            pass
        count1 += 1
        #======= method 2
        count1 = len(open(inputFile1, 'rU').readlines())

    # ======= method 3
    count1 = int(os.popen('wc -l '+inputFile1).readlines()[0].split()[0])
    lc1_end = time.time()

    lc2_start = time.time()
    if False:
        # ======== method 1
        count2 = -1
        for count2, line in enumerate(open(inputFile2, 'rU')):
            pass
        count2 += 1
        # ======== method 2
        count2 = len(open(inputFile2, 'rU').readlines())

    # ======== method 3
    count2 = int(os.popen('wc -l '+inputFile2).readlines()[0].split()[0])
    lc2_end = time.time()
    print "File1 lines:%d taking %f sec" % (count1, lc1_end-lc1_start)
    print "File2 lines:%d taking %f sec" % (count2, lc2_end-lc2_start)
    print "--------------------------------------------"
    sys.stdout.flush()
    if count1!=count2:
        print "File lines unmatch!"
        sys.exit(2)


    # ------------------------------------------------
    #                   Extract Lines
    # ------------------------------------------------
    N = int(count1 / 2)
    M = int(count1 / 2 * ratio)
    print 'Extract %d reads from %d reads.' %(M,N)
    extract_begin = time.time()
    for iteration in xrange(0, sampleSize):
        itertime_start = time.time()
        # -------Generate Sampling Index-------
        idxlist = []
        for i in xrange(0, N):
            if random.randint(0, N) % (N - i) < M:
                idxlist.append(i+1)
                M -= 1
        # -------open output file-------
        filename = outputPrefix + str(iteration + 1) + '.fasta'
        filepath = os.path.join(outputDir, filename)
        output_f = open(filepath, 'w')
        # -------extract line-------
        # extract = linecache.getline
        extract = getline
        for idx in idxlist:
            output_f.write(extract(inputFile1, 2*idx-1))
            output_f.write(extract(inputFile1, 2*idx))
            output_f.write(extract(inputFile2, 2*idx-1))
            output_f.write(extract(inputFile2, 2*idx))
        # -------close output file-------
        output_f.close()
        # -------Iteration Number-------
        itertime_end = time.time()
        print 'sampling %d takes %f sec' % (iteration + 1, itertime_end-itertime_start)
        sys.stdout.flush()

    extract_end = time.time()
    print 'Sample takes %f sec' % (extract_end-extract_begin)
    sys.stdout.flush()
    print "--------------------------------------------"
    tottime_end = time.time()
    print "Total Time: %f sec" % (tottime_end-tottime_start)

