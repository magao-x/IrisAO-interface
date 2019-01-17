
// standard include header files
#include <stdio.h>
#include <irisao.mirrors.h>
#include <fstream>
#include <vector>
#include <string>


typedef struct
    {
    char dserial[512];
    char mserial[512];
    int hardware_flag;
    char PTTfile[512];
    char dumpfile[512];
    } Arguments;


int main(int argc, char* argv[])
{
    Arguments arguments = {"\0", "\0", 1, "\0", "\0"};
    FILE*     stream = NULL;

    // handle parsing the command line arguments
	try
		{
        // display the application version and copyright information
        #ifdef __x86_64
        printf("Iris AO, PTT Utility 64bit [Version 1.0.2.4]\n");
        #else
        printf("Iris AO, PTT Utility 32bit [Version 1.0.2.4]\n");
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

        // verify the required hardware flag command line argument was supplied
        if ((argc < 4) || (sscanf(argv[3], "%d", &arguments.hardware_flag) == 0))
            {
            // write a message indicating no hardware flag argument was supplied
            printf("ERROR: No Hardware Flag argument was supplied.\n\n");

            // throw an invalid argument exception to terminate execution
            throw InvalidArgument;
            }

        // verify the required PTT filename command line argument was supplied
        if ((argc < 5) || (sscanf(argv[4], "%s", arguments.PTTfile) == 0))
            {
            // write a message indicating no PTT filename argument was supplied
            printf("ERROR: PTT filename argument was supplied.\n\n");

            // throw an invalid argument exception to terminate execution
            throw InvalidArgument;
            }
        
        // check if the optional dumpfile command line argument was supplied
        if ((argc < 6) || (sscanf(argv[5], "%s", arguments.dumpfile) != 0))
            {
            // open a new dump file for writing modal position values to
            stream = fopen(arguments.dumpfile, "w");
            }

        if (arguments.hardware_flag == 1)
        {
            arguments.hardware_flag = true;
            printf("\nHardware Disable Flag is ON. Not sending commands!\n\n");
        }
        else
        {
            printf("\nHardware Disable Flage is OFF. Sending commands!\n\n");
            arguments.hardware_flag = false;
        }

		try
			{
            MirrorPosition position;
			SegmentNumber  segment = 0;

            // connect to the specified mirror device
			MirrorHandle mirror = MirrorConnect(arguments.mserial, arguments.dserial, arguments.hardware_flag);
 
 
/*
            // notify the user that the mirror is being flattened
			printf("Flattening mirror segments.\n\n");

            // send the mirror position values to the driver box
            MirrorCommand(mirror, MirrorInitSettings);
*/
 
            // scan through arguments.PTTfile line by line and read to driver
            std::ifstream infile(arguments.PTTfile);
            float z,  xgrad, ygrad;
            segment = 1;
            printf("\nSetting Segment PTT\'s\n\n");
            while (infile >> z >> xgrad >> ygrad)
            {
                /*
                printf("******************************************\n               Segment %d               \n******************************************\n", segment);
                printf("Piston: %2.2f \tTip: %2.2f \tTilt: %2.2f \n\n", z, xgrad, ygrad);
                */
                // Set mirror commands
                SetMirrorPosition(mirror, segment, z, xgrad, ygrad);
                segment++;
            }
            
            // send the mirror position values to the driver box
            MirrorCommand(mirror, MirrorSendSettings);
                
            segment = 0;
                // iterate over the mirror segments twitching the user defined axis in the positive direction
            while (MirrorIterate(mirror, segment))
                {
                // set the mirror position into a positive magnatude tip position
                GetMirrorPosition(mirror, segment, &position);

                // check if the current segment was saturated
                if (!position.reachable)
                    {
                    // notify the user the current segment was saturated
                    printf("Segment %u: Saturated!\n", segment);
                    }
                
                // check if a dumpfile stream was opened
                if (stream)
                    {
                    // write the current segment position information to the dump file
                    fprintf(stream, "%u, %f, %f, %f\n", segment, position.z, position.xgrad, position.ygrad);
                    }
                
                }

            // check if a dumpfile stream was opened
            if (stream)
                {
                // close the specified dumpfile stream
                fclose(stream);
                }
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
        printf("  mserial:            \"Required mirror serial number\"\n");
        printf("  dserial:            \"Required driver serial number\"\n");
        printf("  hardware_flag:      \"Required hardware disable flag [0 or 1]\"\n");
        printf("  PTTfile:            \"Required filename with PTT data\"\n");
        printf("  dumpfile:           \"Optional dumpfile filename.\"\n");
		}

    return 0;
	}