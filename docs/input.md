
# Public functions

## `int connect_input_fd(char *location)`

Takes a string with the location of the input file for the keyboard, and returns a file descriptor for it, ready to be read.

## `char *get_keyboard_device_location()`

Automatically tries to find a keyboard in `/dev/input/by-id` and returns a string containing the keyboard file path, or NULL if one is not found

## `int get_keypress(int input_fd, struct keypress_info *result)`

Fills in the `struct keypress_info` with the next key press, returning 0, or 1 if an error occurred. It will hang until a key is pressed

## `key_to_code(char *key)`

Converts a char or string to its matching key code returned in the `struct keypress_info`.
Some possible values of key:
- `"ctrl"`
- `"shift"`
- `'t'`
- `'j'`

# Structures

## `key_state`

The position of the key
	enum key_state {
		RELEASED,
		PRESSED,
		HELD,
	}

## `keypress_info`

Stores info about a key press.
Returned by `get_keypress()`
	struct keypress_info{
		uint16_t keycode
		enum key_state state
	}
