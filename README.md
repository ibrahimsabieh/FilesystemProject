# FilesystemProject

This repo contains code for a file system for a linux server. 

**Warning, this code is unsafe and should not be used for any purpose other than a demonstration of how a file system may work.**

## Getting started

The purpose of this program is to server as an example of a file system. This file system partitions the contents of files across 8 disks; diska, diskb, diskc, etc. (For purposes of demonstration, these disks are folders that are in the same directory as the program). 

To compile and run the program, first clone the repo and cd into the directory.
```
$ git clone https://github.com/ibrahimsabieh/FilesystemProject
$ cd FilesystemProject
```
This repo already contains the compiled program, however you can compile it yourself if you'd like.

Before you compile the program however, you may want to change the port it is assigned to which by default is 1067.

Run the following to compile the program
`$ gcc -o output_filename project.c -lpthread`

To start the program just run
`$ ./output_filename `
 
 You will see the terminal wait for an input.

## Running Commands

There are three possible commands you can run

 - create
	 - three inputs
		 - file name - required
		 - partitions - optional (default 8)
		 - file contents
 - read
	 - two inputs
		 - file name - required
		 - partition - optional (default all)
			 - outputs all or a specific partitions contents
 - delete
	 - one input
		 - file name - required

In order to run commands on the program you need to open a new terminal and use netcat. There are three example files use can use as input for netcat. These files contain example commands. To create a file using the example create.txt file you can run (assuming port is default).

`$ nc localhost 1067 < create.txt`

Afterwards, you can run the other two command files to either delete or read the contents of the file.

`$ nc localhost 1067 < read.txt`

`$ nc localhost 1067 < delete.txt`

Running the read command will output the contents of the file onto the client terminal, and running create or delete will output true if successful.