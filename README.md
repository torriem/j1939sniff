j1939sniff
==========

This is a simple example of using the  due_can  library on the Arduino Due, or
  FlexCAN_T4  on the Teensy 3.6 or Teensy 4.0 to sniff
j1939 packets on a CAN bus.  If you want to filter the messages
just add some if statements to the  got_frame()  function.  This
code only runs on the Arduino Due.

Requires a CAN transceiver chip attached to the CAN0 TX and RX 
pins of the Due, CAN0 on the Teensy 3.6, or CAN1 on the Teensy 4.0.

Capturing lots of data to the Serial port for analysis on the Due will require
the use of the native USB port, which can go much faster than the
normal UART on the programming USB port. The Teensy USB Serial devices are
already full speed.
