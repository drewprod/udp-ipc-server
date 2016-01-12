# udp-ipc-server
Quick and dirty UDP IPC server.
Merging work with simple udp server and the "ipc" implementation on beej.us (http://beej.us/).

- udp server : http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#datagram
- ipc : http://beej.us/guide/bgipc/output/html/singlepage/bgipc.html

## Use with netcat client
Send message at server.

```
    $ nc -u 127.0.0.1 5555
```
      
## Use with spock
spock retrieve and print IPC message (http://beej.us/guide/bgipc/output/html/singlepage/bgipc.html#mqsamp)

## Show IPC status
```
    $ ipcs -q
```
