j1939sniff
==========

This is a simple example of using the  due_can  library to sniff
j1939 packets on a CAN bus.  If you want to filter the messages
just add some if statements to the  got_frame()  function.  This
code only runs on the Arduino Due.

Requires a CAN transceiver chip attached to the CAN0 TX and RX 
pins of the Due.

Capturing lots of data to the Serial port for analysis will require
the use of the native USB port, which can go much faster than the
normal UART on the programming USB port.
