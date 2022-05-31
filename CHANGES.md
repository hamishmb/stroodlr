Stroodlr (0.9):

  * Initial Development Release.
  * Create a function to convert from string to vector<char> type.
  *
  * Client:
  *
  * Use std.to_string() instead of boost.lexical_cast().
  * Remove unnecessary #includes.
  * Handle if user just hits enter with no input.
  * Get sending messages to work.
  * Get receiving messages semi-working.
  * Stop message reading from blocking the thread.
  * Make the thread exited when requested.
  * Remove old client code.
  * Do sleeps periodically to fix very high CPU usage.
  * Implement in-program help.
  * Start to separate the code into multiple files.
  * Put some files in a subdir.
  * Use pointer to vector<char> when reading data from socket (see below for explanation).
  * Put the message buses in the separate thread that handles the socket.
  * Use SendToServer() again.
  * Get exits sort of working.
  * Declutter client.cpp.
  * Add STATUS, LISTSERV commands.
  * Add support for sending messages with spaces properly.
  * Implement HISTORY command.
  * Update help info.
  * Handle cmdline args properly, add several more args.
  * Have a default server address - localhost - that doesn't always need to be specified.
  * Make HELP command output nicer.
  * Don't ask for input until connected to the server.
  * Send "CLIENTGOODBYE" instead of "Bye!" on exit.
  * Stop user from sending control messages (eg ^).
  * Stop adding spaces at the end of messages.
  * Make commands case-insensitive.
  * Add LISTMSG as an alias for LSMSG.
  * Wait for ENTER if connection fails on startup.
  * Remember what the user was typing if we lose the connection.
  * Don't freeze if user types "SEND" without a message to send.
  *
  * Server:
  *
  * Partially rewrite with boost (rather than C-sockets).
  * Use pointers to vector<char> when reading data from the socket (stops code from adding empty messages to the queue for obscure reasons).
  * Remove unneeded #includes.
  * Make a proper server outgoing message bus.
  * Send an acknowledgement, "ACK", when we receive a message from the client, and handle it in the client.
  * Print some basic info when errors occur, in both server and client.
  * Attempt to get it to reset the socket if the client exits.
  * Make servertools.cpp.
  * Remove need to always specify the portnumber (default is 50000).
  * Handle cmdline args properly.
  * Split into 2 threads.
  *
  * Both:
  *
  * Make the beginnings of a logger (designed to be similar to Python's Logging module).
  * ^ Add some setup member functions.
  * Finish the basic logging functions.
  * Try to fix an issue with mutexes.
  * Use CMake to automate builds.
  * Rewrite to avoid accessing io_service and socket classes across multiple threads (causes unpredictable crashes).
  * Get ACKs working again.
  * Test the logger.
  * Make cmdline usage functions.
  * Some initial testing with a raspberry pi.
  * Refactoring.
  * Remove obsolete function Log_Critical().
  * More refactoring.
  * Fix some segfaults.
  * Add logging messages.
  * Add level support to the logger.
  * Use mutexes in the logger to prevent corruption when there's multithreaded logging happening.
  * Add missing #include <stdexcept>.
  * Test with raspberry pi again.
  * Move cmdline args handlers into separate files.
  * Use const and & where possible to protect and avoid copying data.
  * Start work on a new Sockets class.
  * Split it into 2 classes.
  * Get ClientSocket working.
  * Fix segfault on exit.
  * Get ServerSocket working.
  * Remove old socket functions.
  * Fix a problem with SevrerSockets.
  * Merge the socket classes.
  * Fix weird chars in the logfile.
  * ^ Fix using string comparisons with socket data.
  * Handle CTRL-C.
  * Use special ASCII code \x06 for ACK instead of "ACK".
  * Handle connection errors properly.
  * Handle CTRL-C better.
  * Add the send/receive functions into the Sockets class.
  * Begin to make the Sockets class more independent and generic.
  * ^ Fix a related stack corruption issue.
  * Make Sockets self-managed.
  * Make Sockets.Reset().
  * Refactoring.
  * Make Sockets class handle reconnection itself.
  * Handle losing the connection better.
  * Add more logging messages.
  * Sockets: Allow disabling console output.
  * Clean up CMakeLists.txt.
  * Make a static-library out of the common files to avoid compiling them twice each time.
  * Clean up source code and remove unneeded #includes.
  * Remove a deprecated function.
  * Move SendToServer() to Sockets.SendToPeer().
  * More cleanup.
  * Do "#pragma once" in headers to prevent re-importing.
  * CMake build system: Enable turning debugging symbols and optimisations on and off with cmdline options.
  * Test that reconnecting works.
  * Protect some of the Sockets functions by making them private.
  * Add more logging messages in Sockets functions.
  * Re-do the signal handlers.
  * Add build+dist dirs to the github repo.
  * Remove unwanted conio.h stuff.
  * Fix CMakeLists.txt glitchy compilation options.
