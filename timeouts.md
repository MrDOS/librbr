# Timeouts

When communicating with instruments,
there are three types of timeout to consider.
One is handled by the communications library,
one must be configured,
and one must be handled by the library consumer.

## Instrument Sleep

As described in [the command reference],
RBR instruments will sleep after 10 seconds without input.
The library tracks when a command
was last sent to the instrument
and wakes the instrument as necessary
when sending subsequent commands.

This process should be transparent
to users of the library.
For implementation details,
see `RBRInstrument_wake()`
in `RBRInstrumentInternal.c`.

[the command reference]: https://docs.rbr-global.com/L3commandreference/introduction/command-processing-and-timeouts/timeouts-output-blanking-and-power-saving

## Command Timeout

The second type of timeout,
configured by the user
via the \a commandTimeout value passed to RBRInstrument_open()
or RBRInstrument_setCommandTimeout(),
defines the minimum amount of time the library will wait
for a complete response from the instrument.

Because the user-provided callback
to read instrument data
is defined to block,
this timeout can only be checked
between read operations.

## Character Timeout

Because character reads are implemented by the user
via the RBRInstrumentReadCallback() callback function,
any character timeout must also be implemented by the user.
On POSIX systems, this can be done by using `select(3)`
to determine whether a file descriptor is ready for reading
before a subsequent `read(3)` call;
or for `termios` serial port I/O,
via appropriate configuration `VMIN`/`VTIME`.
