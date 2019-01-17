
//#pragma warning( disable : 4996 )

// standard include header files
#include "stdio.h"
#include "irisao.mirrors.h"

typedef struct
    {
    char dserial[512];
    char mserial[512];
    char dumpfile[512];
    } Arguments;

int main (int argc, char* argv[])
    {
    Arguments arguments = {"\0", "\0", "\0"}; 
    FILE*     stream = NULL;

    // handle parsing the command line arguments
	try
		{
        // display the application version and copyright information
        #ifdef __x86_64
        printf("Iris AO, Flatten Utility 64bit [Version 1.0.2.4]\n");
        #else
        printf("Iris AO, Flatten Utility 32bit [Version 1.0.2.4]\n");
        #endif
        printf("Copyright (C) 2015 Iris AO, Inc. All rights reserved.\n\n");

        // verify the required mirror serial number command line argument was supplied
        if ((argc < 2) || (sscanf(argv[1], "%s", arguments.mserial) == 0))
            {
            // notify the user an invalid mirror serial number command line argument was supplied
            printf("ERROR: Invalid mirror serial number command line argument.\n");

            // throw an invalid argument exception to terminate execution
            throw InvalidArgument;
            }

        // verify the required driver serial number command line argument was supplied
        if ((argc < 3) || (sscanf(argv[2], "%s", arguments.dserial) == 0))
            {
            // notify the user an invalid driver serial number command line argument was supplied
            printf("ERROR: Invalid driver serial number command line argument.\n");

            // throw an invalid argument exception to terminate execution
            throw InvalidArgument;
            }

        
        // check if the optional dumpfile command line argument was supplied
        if ((argc < 4) || (sscanf(argv[3], "%s", arguments.dumpfile) != 0))
            {
            // open a new dump file for writing modal position values to
            stream = fopen(arguments.dumpfile, "w");
            }


		try
			{
            MirrorPosition position;
			//SegmentNumber  segment = 0;

            // connect to the specified mirror device
			MirrorHandle mirror = MirrorConnect(arguments.mserial, arguments.dserial, false);
 
            // notify the user that the mirror is being flattened
			printf("Flattening mirror segments.\n\n");

            // send the mirror position values to the driver box
            MirrorCommand(mirror, MirrorInitSettings);
			}
	
		catch (...)
			{
            // check if a dumpfile stream was opened
            if (stream)
                {
                // close the specified dumpfile stream
                fclose(stream);
                }

            return -1;
			}	

		}
	
	
	catch (...)
		{
		// display the application command line argument usage information
        printf("\nUsage IrisAO.Utility.Modals.exe <mserial> <dserial> <number> <value> [dumpfile]\n\n");
        printf("  mserial:     \"Required mirror serial number\"\n");
        printf("  dserial:     \"Required driver serial number\"\n");
        printf("  dumpfile:    \"Optional dumpfile filename.\"\n");
		}

    return 0;
	}