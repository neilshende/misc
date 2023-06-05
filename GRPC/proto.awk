BEGIN {print "#!/bin/bash";}
/^service/{print "service="$2}
/^package/{print "package="$2}
/ rpc /{rpcs=rpcs" " $2; requests=requests " " $4; replies=replies " " $8}
END {print "rpcs=("rpcs")"; print "requests=("requests")"; print "replies=("replies")";}
