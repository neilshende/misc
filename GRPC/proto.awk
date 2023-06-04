#cat helloworld.proto |awk -f proto.awk | tr -d "()" | sed 's/\[/(/g' | sed 's/\]/)/g'
BEGIN {print "#!/bin/bash";}
/^service/{print "service="$2}
/^package/{print "package="$2}
/ rpc /{rpcs=rpcs" " $2; requests=requests " " $3; replies=replies " " $5}
END {print "rpcs=["rpcs"]"; print "requests=["requests"]"; print "replies=["replies"]";}
