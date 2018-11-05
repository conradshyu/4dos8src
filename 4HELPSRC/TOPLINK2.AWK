#
#  Resolves 4DOS help links (part 2)
#
#  Run with:
#
#     awk -f toplink2.awk filename
#
#  Generates output with adjusted cross-references, using the index of
#  topic numbers in the file TOPLINK.DAT
#

function find_topic(name) {
   tnum = -1
   for (i in topics) {
      if (topics[i] == name) {
         if (tnum != -1) {
            print "***Duplicate topic!"
            return "yyy"
         }
         tnum = topicnums[i]
      }
   }
   if (tnum == -1)
      return "yyy"
   if (length(tnum) < minlength)
      tnum = substr("00000", 1, (minlength - length(tnum))) newnum
   return tnum
}

BEGIN {
   xrefmin = 3
   tcnt = 0

   # Read in the array of topic numbers and names
   while ((getline < "topnum.dat") == 1) {
      topicnums[tcnt] = $1
      topics[tcnt] = substr($0, length($1) + 2)
      tcnt = tcnt + 1
   }
}


{
   fch = substr($1,1,1)

   # Copy comments
   if (fch == ";")
      print

   # Print directives
   else if (fch == "!")
      print

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
         namelen = index(substr(line, pos + topiclen + 2), "\005") - 1

         # if there's no mark at the end of the topic number or name, we're
         # done with the line
         if ((topiclen < 0) || (namelen < 0))
            break

         # if the topic number is empty ignore the reference, otherwise
         # adjust it if required
         copied = 0
         if ((topiclen != 0) && (namelen != 0)) {
            oldtopic = substr(line, pos + 1, topiclen)
            if (oldtopic == "yyy") {
               newtopic = find_topic(substr(line, pos + topiclen + 2, namelen))
               newline = newline substr(line, 1, pos) newtopic "\005" substr(line, pos + topiclen + 2, namelen) "\005"
               copied = 1
            }
         }
         # copy topic if code above did not do it
         if (copied == 0)
            newline = newline substr(line, 1, pos + topiclen + namelen + 2)

         # get rest of line
         line = substr(line, pos + topiclen + namelen + 3)
      }
      print newline line
   }
}

