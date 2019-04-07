#include "../include/simulator.h"
#include<queue>
#include<string.h>
#include<iostream>
using namespace std;
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



/******** STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

int expectedSeq;
int recentlyAckSeq;
int count;

int seqNum;
int ackNum;
int sentFlag;
int recentlySentSeqNum = 0;
float timeOutValue = 20.0;
queue<pkt> bufferQueue;
struct pkt recentlySentPacket;

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{

	struct pkt newPacket;
	newPacket.seqnum = seqNum;
	newPacket.acknum = ackNum;
	
	//find checkSum//
	int checkSum = 0;
	for(int i = 0;i<20;i++)
	{
		checkSum = checkSum + int(message.data[i]);
	}
	newPacket.checksum = checkSum + newPacket.seqnum + newPacket.acknum;
	strncpy(newPacket.payload,message.data,sizeof(message.data));
	
	if(seqNum == 0)
		seqNum = 1;
	else
	{
		if(seqNum == 1)
			seqNum = 0;
	}

	if(sentFlag == 0){
		tolayer3(0,newPacket);
		starttimer(0,timeOutValue);
		sentFlag = 1;
		recentlySentSeqNum = newPacket.seqnum;
		recentlySentPacket = newPacket;
	}
	else
	{
		bufferQueue.push(newPacket);
	}
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
	
	if(packet.seqnum == recentlySentSeqNum)
	{
		stoptimer(0);
		if(!bufferQueue.empty())
		{
			tolayer3(0,bufferQueue.front());
			starttimer(0,timeOutValue);
			recentlySentSeqNum = bufferQueue.front().seqnum;
			recentlySentPacket = bufferQueue.front();
			bufferQueue.pop();
		}
		else
			sentFlag = 0;	
	}
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
		tolayer3(0,recentlySentPacket);
		starttimer(0,timeOutValue);
		sentFlag = 1;
		recentlySentSeqNum = recentlySentPacket.seqnum;
		recentlySentPacket = recentlySentPacket;	
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	seqNum = 0;
	ackNum = 0;
	sentFlag = 0;
	recentlySentSeqNum = 0;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
	
	int checkSum = 0;

	if(count>1 && packet.seqnum == recentlyAckSeq)
	{
		tolayer3(1,packet);
	}
	if(packet.seqnum == expectedSeq)
	{
		for(int i=0;i<20;i++)
		{
			checkSum = checkSum + int(packet.payload[i]);
		}
		checkSum = checkSum + packet.seqnum + packet.acknum;
		if(checkSum == packet.checksum)
		{
			tolayer5(1,packet.payload);
			tolayer3(1,packet);
			count = count+1;
			recentlyAckSeq = packet.seqnum;
			if (expectedSeq == 0)
				expectedSeq = 1;
			else
				expectedSeq = 0;
		}
		else
		{
			packet.seqnum = recentlyAckSeq;
			tolayer3(1,packet);	
		}
	}
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	expectedSeq = 0;
	recentlyAckSeq = 0;
	count = 0;
}
