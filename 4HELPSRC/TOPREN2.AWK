#
#  Renumbers 4DOS help topics (part 2)
#
#  Run with:
#
#     awk -f topren2.awk filename
#
#  Generates output with adjusted cross-references, using the index of
#  topic number changes in the file TOPREN.DAT
#

function adjust_topic(oldnum, minlength) {
   oldval = sprintf("%d", oldnum)
   newnum = new_topics[oldval]
   if (newnum == "")
      newnum = oldnum
   if (length(newnum) < minlength)
      newnum = substr("00000", 1, (minlength - length(newnum))) newnum
   return newnum
}

BEGIN {
   xrefmin = 3

   # Read in the array of topic equates
   while ((getline < "topren.dat") == 1)
      new_topics[$1] = $2
}


{
   fch = substr($1,1,1)

   # Copy comments
   if (fch == ";")
      print

   # Directives
   else if (fch == "!") {

      # Copy everything but topics
      if ($1 != "!TOPIC")
         print

      # Parse topic line
      else {

         # If topic is not equated, just print it
         if (substr($3, 1, 1) != "=")
            print

         # If equated, adjust both halves
         else {
            new_equate = adjust_topic(substr($3, 2), 1)
            print "!TOPIC " $2 " =" new_equate substr($0, (index($0, $3) + length($3)))
         }
      }
   }

   # Not a directive or comment, must be text
   else {
      line = $0

      # loop through and adjust all cross-references (cross-refs are formatted
      # as <004>nnn<005>text ... where nnn is the topic number).  We have to go
      # character by character to deal with escapes

      newline = ""

      for (pos = index(line, "\004"); pos != 0; pos = index(line, "\004")) {

         # if topic start is escaped, ignore it
         if ((pos > 1) && (substr(line, pos - 1, 1) == "\030")) {
            if ((pos == 2) || ((pos > 2) && (substr(line, pos - 2, 1) != "\030"))) {
               newline = newline substr(line, 1, pos)
               line = substr(line, pos + 1)
               continue
            }
         }

         topiclen = index(substr(line, pos), "\005") - 2

         # if there's no mark at the end of the topic number, we're done
         # with the line
         if (topiclen < 0)
            break

         # if the topic number is empty ignore the reference, otherwise
         # adjust it
         if (topiclen == 0)
            newline = newline substr(line, 1, pos + 1)
         else {
            newtopic = adjust_topic(substr(line, pos + 1, topiclen), xrefmin)
            newline = newline substr(line, 1, pos) newtopic "\005"
         }
         line = substr(line, pos + topiclen + 2)
      }
      print newline line
   }
}

