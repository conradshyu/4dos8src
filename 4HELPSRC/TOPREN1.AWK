#
#  Renumbers 4DOS help topics (part 1)
#
#  Run with:
#
#     awk -f topren1.awk filename
#
#  Generates output with adjusted topic numbers, and an index of topic number
#  changes in the file TOPREN.DAT
#

BEGIN {
   new_topic = 1
   system("del topren.dat > nul")
}

{
   # Handle new topic number line
   if ($1 == ";;new") {
      print
      if (($2 == 0) || ($2 < new_topic)) {
         print "Error at line ", NR, ":  Invalid new topic number" > "CON"
         exit(1)
      }
      new_topic = $2
   }

   # Handle roundoff line
   else if ($1 == ";;round") {

      print

      # Defaults are skip 5, round to nearest 10
      TSkip = 5
      TRound = 10

      # Override defaults with user values if specified
      if (NF > 2) {
         TSkip = $2
         if (NF > 3)
            TRound = $3
      }

      # Check for errors in user values
      if ((TSkip < 1) || (TRound < 1)) {
         print "Error at line ", NR, ":  Invalid rounding values" > "CON"
         exit(1)
      }

      # First add skip amount, and subtract one to make rounding easier
      new_topic += (TSkip - 1)

      # Round up, and add 1 back
      new_topic = (TRound * int((new_topic + TRound - 1) / TRound)) + 1

   }

   # Adjust topic numbers
   else if ($1 == "!TOPIC") {
      topic_str = sprintf("!TOPIC %d", new_topic)
      print topic_str substr($0, (index($0, $2) + length($2)))
      if ($2 != 0)
         printf "%04d %04d\n", $2, new_topic >> "topren.dat"
      new_topic++
   }

   # Just print everything else
   else
      print

}


