#include "../include/simulator.h"
#include <string.h>
#include <iostream>
#include <vector>
using namespace std;
//int calculateChecksum(char *,int,int);
/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer 
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

int windowSize;
vector<pkt> window;
float timeOutValue = 55.0;
int expectedSequence;
int nextSequenceNumber;
int startingSequenceNumber;
int recentlyAckSeq;
int base;
int count = 0;
struct pkt recentPacket;


/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
	struct pkt newPacket;
	newPacket.seqnum = startingSequenceNumber;
	newPacket.acknum = startingSequenceNumber;
	strncpy(newPacket.payload,message.data,sizeof(message.data));
	int checkSum = 0;
	for(int i = 0;i<20;i++)
	{
		checkSum = checkSum + int(newPacket.payload[i]);
	}
	checkSum = checkSum + newPacket.seqnum + newPacket.acknum;
	newPacket.checksum = checkSum;
	window.push_back(newPacket);
	startingSequenceNumber++;

	if(nextSequenceNumber < (base + windowSize))
	{
		tolayer3(0,window[nextSequenceNumber]);
		recentPacket = window[nextSequenceNumber];
		if(base == nextSequenceNumber)
		{	
			starttimer(0,timeOutValue);
		}
		nextSequenceNumber++;
	}
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
	if(recentPacket.seqnum == packet.seqnum && recentPacket.acknum == packet.acknum)
	{
		base = packet.seqnum + 1;
		if(base == nextSequenceNumber){
			stoptimer(0);
		}
		else{
			stoptimer(0);
			starttimer(0,timeOutValue);
		}
	}
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
	starttimer(0,timeOutValue);
	for(int i=base;i<nextSequenceNumber;i++)
	{
		tolayer3(0,window[i]);
		recentPacket = window[i];
	}
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	windowSize = getwinsize();
	nextSequenceNumber = 0;
	startingSequenceNumber = 0;
	base = 0;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
		if(packet.seqnum == expectedSequence)
		{
			if(count>1 && packet.seqnum == recentlyAckSeq)
			{
				tolayer3(1,packet);
			}
			else
			{
				int checkSum = 0;
				for(int i = 0;i<20;i++)
				{
					checkSum = checkSum + int(packet.payload[i]);
				}
				checkSum = checkSum + packet.seqnum + packet.acknum;

				if(checkSum == packet.checksum)
				{
					count++;	
					tolayer5(1,packet.payload);
					struct pkt newPacket;
					newPacket.seqnum = expectedSequence;
					newPacket.acknum = expectedSequence;
					expectedSequence++;
					recentlyAckSeq = packet.seqnum;
					tolayer3(1,newPacket);
				}
			}
		}
}

/* the following rouytine will be called once (onlwindowy) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	expectedSequence = 0;
	recentlyAckSeq = 0;	
}	