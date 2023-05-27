# mutex_simple_demo
A simple exercise demonstrates mutex &amp; multi threads.

There are 4 threads to process input from standard input as follows:

Thread 1, called the Input Thread, reads in lines of characters from the standard input.
Thread 2, called the Line Separator Thread, replaces every line separator in the input by a space.
Thread, 3 called the Plus Sign thread, replaces every pair of plus signs, i.e., "++", by a "^".
Thread 4, called the Output Thread, write this processed data to standard output as lines of exactly 80 characters.
These 4 threads communicate with each other using the Producer-Consumer approach. 

Example
We start the program, type 10 characters (not containing any ++) and then press enter. These characters should not be printed to standard output right away because we have 11 characters available to write (10 characters we typed and a space that replaced the line separator) and don't yet have the 80 characters needed to write one complete line.
Next we type 170 characters (not containing any ++) and then press enter. Now there are 182 characters available to write, 11 characters from the Step 1, 170 characters that we typed in step 2 and the space that replaced the line separator in step 2.
The program must write 2 lines with 80 characters each. There are still 22 characters available for output.
Next we type the stop-processing line. Since we only write complete lines with 80 characters, although there are 22 characters still available to write, the program terminates without writing these characters to the standard output.

A pipeline of 4 threads: Input thread reads data from stdin and puts it in Buffer 1. Line Separator thread gets data from Buffer 1, processes it and puts it in Buffer 2. Plus Sign Thread reads data from Buffer 2, processes it and puts it in Buffer 3. Output Thread reads data from Buffer 3 and output lines to stdout.

Pipeline of threads that gets data from stdin, processes it and displays it to stdout

Each pair of communicating threads is constructed as a producer/consumer system.
If a thread T1 gets its input data from another thread T0, and T1 outputs data for use by another thread T2, then
T1 acts as a consumer with respect to T0 and T0 plays the role of T1’s producer
T1 acts as a producer with respect to T2 and T2 plays the role of T1’s consumer
Thus each thread in the interior of the pipeline (i.e., the Line Separator and Plus Sign threads) will contain both producer code and consumer code.
Each producer/consumer pair of threads will have its own shared buffer. Thus, there will be 3 of these buffers in your program, each one shared only by its producer and consumer.
The program must never sleep.
