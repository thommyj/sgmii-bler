/*
 ============================================================================
 Name        : sgmii.c
 Author      : Thommy Jakobsson
 Copyright   : Thommy Jakobsson 2013, GNU General Public License
 Description : simulate bler on sgmii
 ============================================================================
 */

/*This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>


/*
 * *parameters to change
 * -----------------------
 */
//packets to send
#define PACKETS 100000
//100Mbit link utilization, 0.1*1Gbps link utilization in %
#define LINKUTIL 20
//size of each packet
#define PKTSIZE (124*8)
//bits of each error burst
#define BURSTLNT 2
//bit error rate in %
#define BER      0.01
/*-----------------------------------*/



#define IFG_SGMII (lround(PKTSIZE/(LINKUTIL/10/100.0)-PKTSIZE))


#define BITERROR 1
#define OK 0

int errorgen();


int main(void) {
  //counters
  unsigned long long pktcnt_100error=0, pktcnt_1000error=0, bitcnt_100error=0, bitcnt_1000error=0;
  unsigned long long bitcnt_ok=0,bitcnt_total=0;

  int berror100=0, berror1000=0;
  unsigned long long packets=0;
  int bit=0;
  int cbiterror=0;
  int temp;

  setbuf(stdout, NULL);

  printf("Starting..\n  Packets to send: %d\n",PACKETS);
  printf("  Frame size: %d bytes (%d bits)\n",PKTSIZE/8,PKTSIZE);
  printf("  Requested BER: %.04f%%\n",BER);
  printf("  Error burstsize: %d\n",BURSTLNT);
  printf("  Linkutil for 100Mbit: %d%%\n", LINKUTIL);
  printf("  IPG on SGMII: %d bits\n\n",(int)IFG_SGMII);

  srandom ( time(NULL) );

  while(packets++<PACKETS){
    while(bit<(PKTSIZE+IFG_SGMII)){
        cbiterror = errorgen();
        if(cbiterror == OK){
            bit++;
            bitcnt_ok++;
            continue;
        }

        //are we in idle both for 100 and for 1000?
        //then an error doesn't matter
        if(bit>=(PKTSIZE*10)){
            bit++;
            continue;
        }

        //still in packet for 1G?
        if(bit<PKTSIZE){
            berror1000++;
//          printf("error 1000, bit:%d\n",bit);
        }

        //in area that belongs to 100Mbit?
        temp=bit%(8*10);
        if(temp<=7){
            berror100++;
//          printf("error 100, bit:%d(%d)\n",bit,((int)(bit/80))*8+bit%(8*10));
        }

        bit++;

    }

    bitcnt_100error += berror100;
    bitcnt_1000error += berror1000;

    if(berror100)
      pktcnt_100error++;
    if(berror1000)
      pktcnt_1000error++;

    bitcnt_total += bit;

    berror100=0;
    berror1000=0;
    bit=0;

    if(packets%1000==0)
      printf(".");
    if(packets%100000==0)
      printf("\n");
  }
  printf("\n\nFinished\n");
  printf("  Total bits sent %llu\n",bitcnt_total);
  printf("  Total bit errors\n");
  printf("    in 100Mbit:%llu\n",bitcnt_100error);
  printf("    in 1000Mbits:%llu\n",bitcnt_1000error);
  printf("    including idle:%llu\n", bitcnt_total-bitcnt_ok);
  printf("  Total frame errors\n");
  printf("    in 100Mbit:%llu\n",pktcnt_100error);
  printf("    in 1000Mbits:%llu\n\n",pktcnt_1000error);

  printf("BLER(%%)\n");
  printf("  100Mbit:%.05f\n",((float)pktcnt_100error)/PACKETS*100);
  printf("  1000Mbit:%.05f\n\n",((float)pktcnt_1000error)/PACKETS*100);

  printf("BER(%%)\n");
  printf("  100Mbit:%.05f\n",((float)bitcnt_100error)/(PACKETS*PKTSIZE)*100);
  printf("  1000Mbit:%.05f\n",((float)bitcnt_1000error)/(PACKETS*PKTSIZE)*100);
  printf("  Including idle:%.05f\n",(1.0-(float)bitcnt_ok/bitcnt_total)*100);



  return EXIT_SUCCESS;
}

int errorgen(){
  static int burst=0;
  float burstber = BER/BURSTLNT/100;
  float prob = ((float)random() / (RAND_MAX));

  if(burst>0){
      burst--;
      return BITERROR;
  }else if(prob<=burstber){
      burst=BURSTLNT-1;
      return BITERROR;
  }

  return OK;

}
