# IrisAO-interface

Compiled with libirisao.devices.1.0.2.5.so in /usr/lib directory using:

    g++ -Wall -o <executable_name> <filename.cpp> -lirisao.devices.1.0.2.5


Both the flatten_mirror.cpp and mirror_release.cpp codes take the arguments of the mirror and driver serial numbers.

Example call for executable for mirror_release.cpp named Release:

    sudo ./Release PWA37-05-04-0404 09150004

PTT.cpp takes the arguments of the mirror serial number, driver serial number, hardware disable flag (as 1 or 0), and the name of the text file containing the PTT commands (test.txt is uploaded).

example call for executable compiled with name PTT:

    sudo ./PTT PWA37-05-04-0404 09150004 0 test.txt
