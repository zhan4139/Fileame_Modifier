Commentary: This is a converting program using parallel processes, where the
            maximum number of processes will be limited by user's input.

            THe program will first search input directory to find images of
          ".png" ".bmp" and ".gif" type, then convert them to output directory
          with jpg extension (thumbnail image) together with html files.
          For the rest of file type from input directory, name will be recorded
          in "junk.txt" in output directory. Notice that the html files will
          be displayed one by one and all the converting processes is recorded
          in "log.txt" file.

Usage Requirements:
username@Directory/path:$ make
username@Directory/path:$ parallel_convert [convert_count] [output_dir] [input_dir]
