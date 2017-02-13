#1. Input Command Line Error Handling
#-------------------------------------------------------------------------------
#Check Argument Number
if ( $#argv < 3 ) then
  echo "Usage: filter <input_directory> <output_directory> [pattern_1] [pattern_2] ... [pattern_n]"
  exit 1
endif

#Check Input Directory and change permission
set in_dir = "$argv[1]"
if (!(-d $in_dir)) then
  echo "Usage: filter <input_directory> <output_directory> [pattern_1] [pattern_2] ... [pattern_n]"
  exit 1
endif

if (!(-r $in_dir)) then
  echo "Input directory permission denied"
  exit 1
endif
shift

#Check ouput directory if not exist, creat one
set out_dir = "$argv[1]"
shift
if ( !( -d $out_dir) ) then
  mkdir $out_dir
chmod 700 $out_dir
endif

#Check directory "thumbs" under output directory
if (!(-d $out_dir/thumbs)) then
  mkdir $out_dir/thumbs
chmod 700 $out_dir/thumbs
endif

#Check Image extension Validation
@ i = 1 #iterator parameter
while($i <= $#argv)
  set ext =  `echo "$argv[$i]" | awk -F"." '{print $NF}'`
  if( "$ext" != "png" && "$ext" != "gif" && "$ext" != "tiff") then
    echo "Illegal Pattern!"
    exit 1
  endif
  @ i++
end

#2. Convertion
#-------------------------------------------------------------------------------
# Now iterate through the files, testing them for the presence of the pattern

@ j = 1
#for converted filename
set list = ()
#for converted file size
set list2 = ()
#for converted file time
set list3 = ()
while ( $j <= $#argv)
    #pivot for checking duplication both in out_dir and thumbs
    #find the file that meed the pattern (with directory address)
    set file_list = `find $in_dir -name "$argv[$j]"`
    #find the file that meed the pattern (w/o directory address)
    #get the extension of filename
    foreach fn ($file_list)
      if (!(-r $fn)) then
        echo $fn "is not readable"
      else
      #get filename of input fn
        set fn_filename = `basename $fn | cut -d "." -f1`
        #change it to uppercase then compare with those in out_dir
        set up_input = `echo $fn_filename | tr "a-z" "A-Z"`
        set difft1 = true
        set difft2 = true
        foreach existedfile1 ($out_dir/*)
          if (-f $existedfile1) then
          #get filename of out_dir w/o extension
            set temp =  `basename -s . $existedfile1 | cut -d "." -f1`
            #if same, not convert
            if ( "$temp" == "$up_input" ) then
              set difft1 = false
            endif
          endif
        end

        #check empty thumbs
        set isempty = `ls $out_dir/thumbs | wc -l`
        if ($isempty != 0) then
          foreach existedfile2 ($out_dir/thumbs/*)
            if (-f $existedfile2) then
            #get filename before '_' in /thumbs
              set temp2 = `basename -s . $existedfile2 | cut -d "_" -f1`
              #if same, not convert
              if ( "$temp2" == "$up_input" ) then
                set difft2 = false
              endif
            endif
          end
        endif
        #if neither in out_dir nor thumbs, put in a list and convert
        #to uppercase and thumbnail
        if ( $difft1 == true && $difft2 == true ) then
          #converted image filename list (w/o extension)
          set list = ($list $up_input)
          convert  $fn $out_dir/`echo $up_input`.jpg
  	      convert -thumbnail 200x200 $fn $out_dir/thumbs/`echo $up_input`_thumb.jpg
          #converted size list
          set list2 =($list2 `identify ./$out_dir/thumbs/$up_input"_thumb.jpg" | awk -F" " '{print $3}'`)
          #Original Date list
          set list3 = ($list3 `stat -c %y $fn | awk -F" " '{print $1}'`)
        endif
    endif
  end
  @ j++
end

#3. HTML 
#-------------------------------------------------------------------------------
#creat html file, if exist, overwrite it

set date =  (`date +"%a %m/%d/%y"`)
set date = ("<" $date ">")
echo "Please Enter Web Theme: "
set theme = $<
echo "<html>" > ./"pic_name_xx.html"
echo "<head>" >> ./"pic_name_xx.html"
echo "<title>My Picture</title>" >>./"pic_name_xx.html"
echo "</head>" >>./"pic_name_xx.html"
echo "<body>">> ./"pic_name_xx.html"
echo "<table border = "2">">> ./"pic_name_xx.html"
echo "<tbody>" >> ./"pic_name_xx.html"
#get table content
#if list is not empty then creat table content
if ($#list > 0) then
  #print image
  echo "<tr>" >>./"pic_name_xx.html"
    @ i = 1
    while ($i <= $#list)
      echo "<td><a href = "\"./$out_dir//$list[$i].jpg\"">\
      <img src = "\"./$out_dir//thumbs/$list[$i]_thumb.jpg\""/></a></td>" >> ./"pic_name_xx.html"
      @ i++
    end
  echo "</tr>">>./"pic_name_xx.html"
  #print resized size
  echo "<tr>" >>./"pic_name_xx.html"
  @ i = 1
  while ($i <= $#list)
    echo "<td align="\"center\""> $list2[$i] </td>" >> ./"pic_name_xx.html"
    @ i++
  end
  echo "</tr>">>./"pic_name_xx.html"
  #print original date
  echo "<tr>" >>./"pic_name_xx.html"
  @ i = 1
  while ($i <= $#list)
    echo "<td align="\"center\""> $list3[$i] </td>" >> ./"pic_name_xx.html"
    @ i++
  end
  echo "</tr>">>./"pic_name_xx.html"
endif
echo "</tbody>">> ./"pic_name_xx.html"
echo "</table>">> ./"pic_name_xx.html"
echo "<p>Theme: $theme</p>" >> ./"pic_name_xx.html"
echo "<p>Date & day: $date </p>" >> ./"pic_name_xx.html"
echo "</body>" >> ./"pic_name_xx.html"
echo "</html> " >> ./"pic_name_xx.html"
