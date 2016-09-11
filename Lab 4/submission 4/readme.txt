The experiment is done with server on a terminal in a laptop and the client in another terminal in the same laptop.
Laptop specifications: 7.7 GB RAM, i5-4210U CPU, Linux - 64bit.

The Protocol used:

	For every packet we add an extra byte at the beginning '1', and for the last packet we replace this with a '0' so that the client can stop reading.
	We are reading files and sending it over the socket over packet sizes of 2048 bytes.

To run program :

	1. You must have foo0.txt to foo9999.txt in "files" directory which must be present in the same directory as the executables. These can be created using the python code written in the end. This creates 10000 files inside the directory where the python code is executed.
	2. To create executable files, type "make" in the terminal; and server and client executables are generated.
	3. './server-mt 5000 2 1' to start server of queue size 1 and 2 workers at port 5000
	4. './multi-client localhost 5000 1000 120 0 fixed' to start a client with localhost replaced as the server ip address, and its post number as 5000 with 1000 user threads experimanting for 120 seconds with a sleep time of zero with a fixed file files/foo1.txt

Test cases:
	server shows nothing. below is the test case for queue size of 1 and 1 worker at server

	$ ./multi-client localhost 5000 1000 120 0 fixed
	Done!
	throughput = 618.625 req/s
	average response time = 0.025 sec
	$