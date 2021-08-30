#python - vol-086d09f4d5220f08d
import sys, os, re, json
data=json.load(os.popen("aws ec2 describe-volumes"))
for i in range(len(data["Volumes"])):
  print data["Volumes"][i]["VolumeId"]
 
volid=sys.argv[1]
data=json.load(os.popen("aws ec2 describe-volumes --volume-ids "+volid))
print data["Volumes"][0]["Iops"]

