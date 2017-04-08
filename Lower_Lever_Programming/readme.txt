This program is a simple implementation of in-memory file system. The program will first check from the given input directory, then it will copy all of the files to your file system, provide a summary of the filesystem in a text file, and copy only the images to an output directory. Then convert the images to thumbnails inside the same directory and prepare an html file showing all of the thumbnail images.

This program includes the following items:
test.c     
mini_filesystem.h
mini_filesystem.c
libctest.a ( This is the static library of the source code)
makefile

Usage: test <input_dir> <output_dir> <log_filename>

 

a. Personal Information for the group:
 
 
b. CSELab machine tested on: kh4-250 -31
 
c.Syntax and usage pointers:
        Usage: test <input_dir> <output_dir> <log_filename>
 
 
d. Special test cases handled or note handled: 

We are assuming that filename will not be larger than 20 characters. 
We are assuming that input files exist and are readable. We are assuming that none of the input file names will have any spaces in the name. 
We are assuming that all file names will be in lowercase format. 
We are assuming total file count cannot exceed 128 or the total size would not exceed the size of the mini file system.
We are assuming that only files in the input directory.

e. Experiment results:

        Given:
        Rotation Rate – 15,000 RPM
        Average Seek time – 4 ms
        Average number of sectors per track – 1000
        Sector size = Block size = 512 bytes.
       
        Data we got:
        Total count:12355
        Read operations: 12355/2=6178(approximate)
        Average Read per file = 6178/6=1030
 
        Max Rotation Time = 1/RPM * 60secs/1min = 4 ms
 
        Average Rotation Time = 1/2 * Max Rotation Time = 2 ms
 
Best case:
Best case = (Average Seek time+ Average Rotation Time)+( Average Read per file/ Average number of sectors per track)*(Max Rotation Time)
= (4+2)+(1030/1000)*4=10.03ms
 
Average case:
Average case=(Average Seek time+ Average Rotation Time)*( Average Read per file)*(Average Rotation Time)
=(4+2)*1030=6180ms
