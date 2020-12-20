# Client-Server

Welcome!
I made this project all by myself as a challenge to make a socket connection in a low language like C instead of just do it the simple way in python.  
This project include two small projects and a one ongoing.

All small projects are sharing some few common aspects, 
the code create a TCP socket connection using the header called "windows.h" and use it to handle a comunication with more than one client. Though each small project does it in a diffrent way!

## Server 1.0
The first project is under "Server" folder. Its bassicly an echo server with a little twist which also use multithreding to handle each client, here is a simple yet all-covering exmple.

### Exmple:
So lets say two clients: client A and client B are connecting to the server each at its own time, sending some messages and disconnect.

*\*Client A establishs a connection\*  
Client A to Server: "Hello"  
Server to Client A: "Hello"  
Client A t Client A: "World"  
Server to Client A: "World"  
\*Client B establishs a connection\*  
Server to Client B: "World"      
Client B to Server: "Wow"    
Server to Client B: "Wow"  
Server to Client A: "Wow"  
Client A to Server: "That is so cool!"  
Server to Client A: "That is so cool!"  
Server to Client B: "That is so cool!"*  

Lets explain what is happening in the above exmple.
First the Server make a new thread to handle the Client A connection, then it wait and listen for an input using that thread. When an input comes the thread activate a [LOCK](http://google.com) mechanism until it finish to save the input in memorie. After that the loop on the thread detect a change in the memorie and output the saved info. Then the last procces can be repeated several times:

server listen -> gets an input -> overwrite the memorie -> detect a new memorie -> output the overwrited memorie -> back to listen

When Client B connects a new thread created for him. First the thread check if there is new data in memorie (if its diffrent than NULL) and output if true. Than that same routines occurs. Due now the [LOCK](http://google.com) can be active and if so the input will not be saved and accepted by the server, also since all thread check for new data in the memorie the input for the server will be echo for all Clients!.

## Server 2.0


