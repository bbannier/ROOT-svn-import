
   This directory contains the necessary stuff to build a small binary for a non-blocking check
   for xproofd or xrootd services at a given host and port.

   0. Building

      Just typing 'make' should suffice to build the binary. The name of the binary is 'xrdping'
      and it will be loacted in the current directory.

      To build in debug mode use 'make DEB=-g'

   1. Running

      The arguments understood by 'xrdping' can be displayed pasing the -h or --help switches.

      To test 'xproofd' at 'host' and 'port' use:

         $ xrdping host:port

      or

         $ xrdping host -port

      To test 'xrood' at 'host' and 'port' use:

         $ xrdping host:port xrd

      or

         $ xrdping host:port xrootd

      (the -p switch can be used for the port also in this case).

      The program returns a non negative value according to

               0   the required service accepts connections at given host and port
               1   no service running at the given host and port
               2   a service accepts connections at given host and port but most likely
                   is not of the required type (failure occured during handshake)
               3   service at given host and port is PROOFD (or ROOTD)

      To silence all messages (including errors) pass the '-nv' switch.

      To increase the verbosity use the '-v' switch.

   2. Scripting

      The script 'ping.sh' gives an example of how to use 'xrdping' in scripts. It assumes that
      the binary is in the current directory.

   ---------------------------------------
   (G. Ganis - last update: Sept 16, 2011)


