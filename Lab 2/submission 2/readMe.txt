140050007, 140050002
Neeladrishekhar Kanjilal, Deep Modh

For this experiment we have arranged the server on a laptop and the client on a lab machine. They are connected by the ethernet cables available inside the lab.
	Server machine(Laptop) specifications: 7.7 GB RAM, i5-4210U CPU, Linux - 64bit.
	Client machine : lab machine in SL 2
	Ethernet cable : cable in lab SL 2

The Protocol used:

	For every packet we add an extra byte at the beginning '1', and for the last packet we replace this with a '0' so that the client can stop reading.
	We are reading files and sending it over the socket over packet sizes of 2048 bytes.

To run program :

	1. You must have foo0.txt to foo9999.txt in "files" directory which must be present in the same directory as the executables. These can be created using the python code written in the end. This creates 10000 files inside the directory where the python code is executed.
	2. To create executable files, type "make" in the terminal; and server and client executables are generated.
	3. Run the executables as mentioned in lab2.pdf
	4. To remove executables; type "make clean"

Python code for creating foo text files:-

for j in range(0,10000):
	file = open("foo"+str(j)+".txt", "w")
	for i in range(1,50500):
		file.write("hello world the new file "+str(i)+" "+str(j)+" anoth\n")
	file.close()