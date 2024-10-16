
# Public functions

## `int main_tray()`

### How to use

`main_tray(int flags)` should be used to start the tray. Usually you would fork, or open a new thread to run this function in. It handles incoming socket connections from `/tmp/tray.socket`. (can be changed in the `#define SOCKET_PATH`). This function handles both socket connections and also waiting on executed forks returning. It will handle a connection then attempt to wait for all open processes.

### Flags

This function can be called with multiple flags to change properties about a starting tray. Flags can be or'd together to use multiple, e.g. `FLAG_A | FLAG_B`. Current flags include:
- `0` start tray in default configuration
- `TRAY_NO_PERSIST` once the tray has no more programs to hold, quit.

### Return value

Don't expect it to return, unless it is sent a kill opcode by a connected client. Otherwise, it should return 0 on success and -1 on failure.

## `int start_program(const char *command)`

### How to use

This function can be used by tray clients, and handles connecting to the tray, sending it a request to run a command, and returning the response code.

### Return value

0 on success, -1 on failure

# Private functions

## `static int connect_tray_socket()`

### How to use

This function handles connecting to the tray socket.

### Return value

Returns the tray socket on success or -1 on failure
