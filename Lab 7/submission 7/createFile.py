file = open("foo.txt", "w")
for i in range(1,4000000):
	file.write("hello world hello world"+str(i))
file.close()