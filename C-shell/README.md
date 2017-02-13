# Fileame_Modifier
This program will search through a given directory looking for images of a specified type that as user entered, convert all the images into JPEG format in a given output directory, and create a presentation web page called pic_name_xx.html that contains links to the converted images.

This program will first ask user input from command line as create_images input_dir output_dir pattern_1 [pattern_2] ... [pattern_n]. 
First off, program will check whether input is valid or not by going through every argument of the command line, when invalid argument appear, it will automatic terminate. If there is no error of user input, program will check output directory. if not exist, it will create one. 
Secondly, program will find all files satisfying the patterns from input directory. 
Thirdly, it will convert all of the satisfied pattern to jpg file and put them into the input directory, simultaneously it will also generate a thumb pattern into the thumbs directory. 
Finally, the program will generate a html which will contain all of the file you just converted, it will contain some info of the converted pattern.

After run this program, user will get all of the satisfied pattern to be converted as jpg file into output directory and a thumb jpg file into thumb directory according to the user input. 
A html file will be generated to present all of the converted pattern. When user has invalid input, program will automatic terminate. 
If the input directory is unreadable, program will prompt the error message and terminate. When there is unreadable file exited in user input file, program will prompt the error message, but keep converting the rest of the satisfied patterns.
