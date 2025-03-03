# **Function/Method**



## **accept()**
```	
Accepts an incoming client connection on a listening    
socket and returns a new socket file descriptor for communication with the client.
```


## **bind()**	
```		
Associates (binds) a socket with a specific IP address and port number,
preparing it for incoming or outgoing connections.
```	

## **close()**
```
Closes the specified file or socket descriptor, releasing its resources.
```

## **fcntl()**
```
Performs various operations on file descriptors; 
commonly used to set a socket to non-blocking mode (e.g., O_NONBLOCK).
```

## **fsync()**
```
Forces any buffered data to be written out to a file descriptor 
(e.g., a file or socket) immediately, ensuring data integrity.
```

## **listen()**
```
Places a socket in a passive state, ready to accept incoming 
connection requests (in conjunction with accept()).
```

## **poll()**
```
Monitors multiple file descriptors (sockets) for events 
(e.g., ready for reading or writing) within a specified timeout period.
```

## **POLLIN**
```
A flag for poll() indicating that the file descriptor is 
ready to read (incoming data available).
```

## **POLLOUT**
```
A flag for poll() indicating that the file descriptor is
ready to write (buffer space available to send data).
```

## **recv()**
```
Receives data from a socket and stores it in a buffer.
Typically used in non-blocking or blocking modes.
```

## **send()**
```
Transmits data over a socket to the connected peer.
Similar to write(), but specific to networking.
```

## **setsockopt()**
```
Configures socket-level options (e.g., SO_REUSEADDR, TCP_NODELAY),
allowing fine control over socket behavior.
```

## **socket()**	
```
Creates a socket for network communication (TCP/UDP).
Returns a file descriptor that can be used for subsequent socket operations.
```

## **std::stoi()**
```
Converts a string to an integer (e.g., used to parse a port number
from command-line arguments). Throws an exception on failure.
```

## **std::transform()**
```
Applies a unary or binary operation to a range of elements.
Commonly used to convert strings to uppercase or lowercase.
```

## **LIBERA CHAT (reference)**
```
irssi -c irc.libera.chat -p 6697 -n Alisa_test
```