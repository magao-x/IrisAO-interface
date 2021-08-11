/*
This is just BMC-interface adapted to the IrisAO as a stop-gap until we get the proper MagAO-X app working.

To compile:

make shmirisAO

*/

/* IrisAO */
#include "irisao.mirrors.h"

/* cacao */
#include "ImageStruct.h"   // cacao data structure definition
#include "ImageStreamIO.h" // function ImageStreamIO_read_sharedmem_image_toIMAGE()

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <iostream>
#include <signal.h>
#include <curses.h>
#include <unistd.h>
#include <math.h>
#include <argp.h>


typedef int bool_t;

// interrupt signal handling for safe DM shutdown
volatile sig_atomic_t stop;

void handle_signal(int signal)
{
    if (signal == SIGINT)
    {
        printf("\nExiting the IrisAO DM control loop.\n");
        stop = 1;
    }
}

int sendCommand(MirrorHandle dm, IMAGE * SMimage) {
    //Based on Alex Rodack's IrisAO script
   SegmentNumber segment = 0; // start at 0 or 1?
   MirrorPosition position;
   int idx;
   while (MirrorIterate(dm, segment)){

      // need shmim array formatted in a way that's consistent with this loop
      idx = (segment-1) * 3; // segments start at 1
      SetMirrorPosition(dm, segment, SMimage[0].array.F[idx], SMimage[0].array.F[idx+1], SMimage[0].array.F[idx+2]); // z, xgrad, ygrad

      /*printf("segment %d\n", segment);
      printf("z %f\n", SMimage[0].array.F[idx]);
      printf("xgrad %f\n", SMimage[0].array.F[idx+1]);
      printf("ygrad %f\n", SMimage[0].array.F[idx+2]);*/
   
      // check if the current segment was saturated
      // not sure you can do this here. might need to send the commands first (depends on what this is actually querying)
      // not sure how to handle ptt in the m_instSatMap. I guess I need to saturate a whole row/column at once??
      /*GetMirrorPosition(dm, segment, &position);
      if (!position.reachable)
      {
         m_instSatMap.data()[idx] = 1;
         m_instSatMap.data()[idx+1] = 1;
         m_instSatMap.data()[idx+2] = 1;
      }
      else
      {
         m_instSatMap.data()[idx] = 0;
         m_instSatMap.data()[idx+1] = 0;
         m_instSatMap.data()[idx+2] = 0;
      }*/
   }

   /* Finally, send the command to the DM */
   MirrorCommand(dm, MirrorSendSettings);
   
   return 0 ;
}

int zeroDM(MirrorHandle dm)
{
   /* Send the all 0 command to the DM */
   SegmentNumber segment = 0;
   while (MirrorIterate(dm, segment)){
      SetMirrorPosition(dm, segment, 0, 0, 0); // z, xgrad, ygrad
      segment++;
   }
   MirrorCommand(dm, MirrorSendSettings);
   printf("DM zeroed\n");
   return 0;
}

// intialize DM and shared memory and enter DM command loop`
int controlLoop(const char * mSerial, const char * dSerial, const char * shm_name, const char * hardwareDisable) {

    // Initialize variables
    uint32_t shm_x = 37; // Hard-coded for now
    uint32_t shm_y = 3;
    MirrorHandle dm;
    IMAGE* SMimage;

    // connect to shared memory image (SMimage)
    SMimage = (IMAGE*) malloc(sizeof(IMAGE));
    ImageStreamIO_read_sharedmem_image_toIMAGE(shm_name, &SMimage[0]);

    // Validate SMimage dimensionality and size against DM
    if (SMimage[0].md[0].naxis != 2) {
        printf("SM image naxis = %d, but expected 2.\n", SMimage[0].md[0].naxis, shm_x);
        return -1;
    }
    if (SMimage[0].md[0].size[0] != shm_x) {
        printf("SM image size (axis 1) = %d, but expected %d.\n", SMimage[0].md[0].size[0], shm_x);
        return -1;
    }
    if (SMimage[0].md[0].size[1] != shm_y) {
        printf("SM image size (axis 2) = %d, but expected %d.\n", SMimage[0].md[0].size[1], shm_y);
        return -1;
    }

    // open mirror connection
    printf("Opening mirror %s connection with driver %s\n", mSerial, dSerial);
    if (atoi(hardwareDisable)){
        printf("Hardware disable flag on. Not sending commands\n");
    }
    dm = MirrorConnect(mSerial, dSerial, atoi(hardwareDisable) );
    printf("Connected\n");

    // SIGINT handling
    struct sigaction action;
    action.sa_flags = SA_SIGINFO;
    action.sa_handler = handle_signal;
    sigaction(SIGINT, &action, NULL);
    stop = 0;
    double dt_loop = 0;
    double dt_com = 0;
    int times = 0;
    // control loop

    printf("IrisAO %s: waiting on commands!\n", mSerial);
    while (!stop) {
        //printf("IrisAO %s: waiting on commands.\n", mSerial);
        // Wait on semaphore update
        ImageStreamIO_semwait(&SMimage[0], 3); //hard-coded
        
        // Send Command to DM
        if (!stop) { // Skip DM on interrupt signal
            // send command
            sendCommand(dm, SMimage);
        }
    }

    // Safe DM shutdown on loop interrupt
    // Zero all actuators
    zeroDM(dm);
    printf("IrisAO %s: all voltages set to 0.\n", mSerial);

    // Close the connection
    MirrorRelease(dm);
    printf("IrisAO %s: connection closed.\n", mSerial);

    return 0;
}

int main(int argc, char* argv[]) {

    // args = mSerial dSerial shmim_name hardwareDisable
    controlLoop(argv[1], argv[2], argv[3], argv[4]);

    return 0;
}
