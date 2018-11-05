#
#  Adjusts numbers of 4DOS help topics
#
#  Run with:
#
#     awk -f topadj.awk first=n1 last=n2 adjust=n3 filename
#
#  where:
#
#     n1 = start of renumber range
#     n2 = end of renumber range
#     n3 = adjustment amount (+/-)

function adjust_topic(oldnum) {
   newnum = oldnum + (((oldnum >= first) && (oldnum <= last)) ? adjust : 0)
   if (length(newnum) < minlength)
      newnum = substr("00000", 1, (minlength - length(newnum))) newnum
   return newnum
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

         # If topic is not equated, just adjust topic number
         if (substr($3, 1, 1) != "=") {
            new_topic = adjust_topic($2)
            topic_str = sprintf("!TOPIC %d", new_topic)
            search_str = $2
         }

         # If equated, adjust both halves
         else {
            new_topic = adjust_topic($2)
            new_equate = adjust_topic(substr($3, 2))
            topic_str = sprintf("!TOPIC %d =%d", new_topic, new_equate)
            search_str = $3
         }
         print topic_str substr($0, (index($0, search_str) + length(search_str)))
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
            newtopic = adjust_topic(substr(line, pos + 1, topiclen))
            newline = newline substr(line, 1, pos) newtopic "\005"
         }
         line = substr(line, pos + topiclen + 2)
      }
      print newline line
   }
}

