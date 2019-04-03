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

//
//int messagessent = 1;
//int interruptCount = 1;
//

int expectedSeq;
int seqNum = 0;
int ackNum = 0;
int sentFlag = 0;
int recentlySentSeqNum = 0;
float timeOutValue = 10.0;
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
		// cout<<endl;
		// cout<<"Number of messages sent : "<<messagessent++<<endl;
		// cout<<"A_output Start : "<<endl;
		// cout<<"Sent message from A_output : "<<newPacket.payload<<endl;
		// cout<<"Sent seqnum from A_output : "<<newPacket.seqnum<<endl;
		// cout<<"A_output End : "<<endl;
		tolayer3(0,newPacket);
		starttimer(0,timeOutValue);
		sentFlag = 1;
		recentlySentSeqNum = newPacket.seqnum;
		recentlySentPacket = newPacket;
	}
	else
	{
		// cout<<endl;
		// cout<<"Number of messages sent : "<<messagessent++<<endl;
		// cout<<"Buffer Start : "<<endl;
		// cout<<"Buffer Sent message from A_output : "<<newPacket.payload<<endl;
		// cout<<"Buffer Sent seqnum from A_output : "<<newPacket.seqnum<<endl;
		// cout<<"Buffer End : "<<endl;
		bufferQueue.push(newPacket);
		// cout<<"Queue Size : "<<bufferQueue.size()<<endl;
	}
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
	stoptimer(0);
	if(packet.seqnum == recentlySentSeqNum)
	{
		// cout<<endl;
		// cout<<"--------Acknowledgment Start------"<<endl;
		// cout<<"Received Packet Sequence Number : "<<packet.seqnum<<endl;
		// cout<<"Received Packet Payload : "<<packet.payload<<endl;
		// cout<<"Current Queue Size : "<<bufferQueue.size()<<endl;
		// cout<<"--------Acknowledgment End------"<<endl;
		if(!bufferQueue.empty())
		{
			sentFlag = 0;
			tolayer3(0,bufferQueue.front());
			// cout<<"--------Message sent after Acknowledgment : "<<bufferQueue.front().payload<<endl;
			// cout<<"--------Message seqnum after Acknowledgment : "<<bufferQueue.front().seqnum<<endl;
			starttimer(0,timeOutValue);
			sentFlag = 1;
			recentlySentSeqNum = bufferQueue.front().seqnum;
			recentlySentPacket = bufferQueue.front();
		}
		else
			sentFlag = 0;	
	}
	else
	{
		//cout<<endl;
		// cout<<"--------Negative Acknowledgment Start------"<<endl;
		// cout<<"Received Packet Sequence Number : "<<packet.seqnum<<endl;
		// cout<<"Received Packet Payload : "<<packet.payload<<endl;
		// cout<<"--------Negative Acknowledgment End------"<<endl;
		tolayer3(0,recentlySentPacket);
		starttimer(0,timeOutValue);
		sentFlag = 1;
		recentlySentSeqNum = recentlySentPacket.seqnum;
		recentlySentPacket = recentlySentPacket;	
	}
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
		// cout<<endl;
		// cout<<"interruptCount : "<<interruptCount++<<endl;
		// cout<<"--------Interrupt Start------"<<endl;
		// cout<<"Interrupt Packet Sequence Number : "<<recentlySentPacket.seqnum<<endl;
		// cout<<"Interrupt Packet Payload : "<<recentlySentPacket.payload<<endl;
		// cout<<"--------Interrupt End------"<<endl;

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
	
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
	int checkSum = 0;
	// cout<<endl;
	// cout<<"****B_Input****"<<endl;
	// cout<<"expectedSeq : "<<expectedSeq <<endl;
	if(packet.seqnum == expectedSeq)
	{
		// cout<<"Received seqnum : "<<expectedSeq <<endl;
		for(int i=0;i<20;i++)
		{
			checkSum = checkSum + int(packet.payload[i]);
		}
		checkSum = checkSum + packet.seqnum + packet.acknum;
		if(checkSum == packet.checksum)
		{
			// cout<<"______________________Proper Message____________________"<<endl;
			tolayer5(1,packet.payload);

			if (expectedSeq == 0)
				expectedSeq = 1;
			else
				expectedSeq = 0;

			tolayer3(1,packet);
		}
		else
		{
			// cout<<"ImProper Message"<<endl;

			int seqack = 0;

			if(expectedSeq == 1)
				seqack = 0;
			else
				seqack = 1;

			packet.seqnum = seqack;
			packet.acknum = seqack;
			tolayer3(1,packet);
		}
	}
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	int expectedSeq = 0;
}
