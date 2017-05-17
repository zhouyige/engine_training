import sys
import shutil 

print "builder:"

dstProjName = "Sampler1"
dstContentName = "Content"

for i in range(2, len(sys.argv)):
	print "para:", i, sys.argv[i]

        if sys.argv[i] == '-n':
                dstProjName = sys.argv[i+1]
        if sys.argv[i] == '-c':
                dstContentName = sys.argv[i+1]
	if sys.argv[i] == '-d':
                print "copy sampler to " , sys.argv[i+1]
                shutil.copytree("Sampler1",sys.argv[i+1]+"/"+dstProjName)
                shutil.copytree("Content",sys.argv[i+1]+"/"+dstContentName)
