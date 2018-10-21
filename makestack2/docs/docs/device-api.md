# Device API

## Using plugin
TODO

## Functions
### println
- **Definition:** `void println(const *fmt, ...);`
- **Description:** Emit a log message to the server.
- **Example:**
  ```c++
  println("The answer is %d.", 42);
  ```

### publish
- **Definition:** `void publish(const char *event, char *value);`
- **Description:** Emit an event. You can listen for the event by `server.device.onEvent` in server.
- **Example:**
  ```c++
  publish("button_a", "pressed");
  ```

## Arduino-compatible Functions
### delay
### digitalRead
### digitalWrite
## Wire
