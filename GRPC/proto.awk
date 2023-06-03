#cat helloworld.proto |awk -f proto.awk | tr -d "()" | sed 's/\[/(/g' | sed 's/\]/)/g'
/^service/{print "service="$2}
/^package/{print "service="$2}
/ rpc /{rpc=rpc" " $2; requests=requests " " $3; replies=replies " " $5}
END {print "rpc=["rpc"]"; print "requests=["requests"]"; print "replies=["replies"]";}
