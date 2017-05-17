import sys
import shutil 

print "builder:" 
for i in range(1, len(sys.argv)):
	print "para:", i, sys.argv[i]

	if sys.argv[i] == "-d"
	print "copy sampler to " , sys.argv[++i]
	shutil.copytree("Sampler1",sys.argv[i]+"\Source")
	shutil.copytree("Content",sys.argv[i])