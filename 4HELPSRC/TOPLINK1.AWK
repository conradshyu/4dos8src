#
#  Resolves 4DOS help links (part 1)
#
#  Run with:
#
#     awk -f toplink1.awk filename
#
#  Generates an index of topic numbers in the file TOPNUM.DAT
#

BEGIN {
   system("del topnum.dat > nul")
}

# Accumulate topic numbers and names
$1 == "!TOPIC" {
   if (($2 != 0) && ((length($3) == 1) || (substr($3, 1, 1) != "="))) {
      topicstr = substr($0, (index($0, $2) + length($2)))
      if (substr(topicstr, 1, 1) == "")
         topicstr = substr(topicstr, 2)
      printf "%s%s\n", $2, substr($0, (index($0, $2) + length($2))) >> "topnum.dat"
   }
}


