/*
 * Copyright (c) 2012, Thingsquare, www.thingsquare.com.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#include "contiki.h"
#include "sys/ctimer.h"
#include "sys/etimer.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-debug.h"

#include "simple-udp.h"

//imported
#include "project-conf.h"
#include "sys/rtimer.h"
#include "sys/energest.h"
#include "dev/gpio.h"
#include "dev/ioc.h"
/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#include <stdio.h>
#include <string.h>

#define UDP_PORT 1234

#define SEND_INTERVAL		(1 * CLOCK_SECOND)

//imported
//const linkaddr_t transm_addr = {{ 0x00, 0x12, 0x4b, 0x00, 0x19, 0x32, 0xe4, 0x84 }}; //0x00, 0x12, 0x4b, 0x00, 0x18, 0xe6, 0x9b, 0xcf 
//const linkaddr_t recv_addr = {{ 0x00, 0x12, 0x4b, 0x00, 0x19, 0x32, 0xe5, 0xc5 }}; //0x00, 0x12, 0x4b, 0x00, 0x19, 0x32, 0xe4, 0xcc
//#if ENERGEST_CONF_ON
//const linkaddr_t energest_addr = {{ 0x00, 0x12, 0x4b, 0x00, 0x19, 0x32, 0xe5, 0xc5 }}; //0x00, 0x12, 0x4b, 0x00, 0x14, 0xd5, 0x2d, 0xe1
//#endif

uint16_t seqno=0;
struct energestmsg prev_energest_vals;

static struct simple_udp_connection unicast_connection;

/*---------------------------------------------------------------------------*/
PROCESS(observed_sender_node_process, "Observed Sender node process");
AUTOSTART_PROCESSES(&observed_sender_node_process);
/*---------------------------------------------------------------------------*/
void
GPIOS_init(void)
{

	ioc_set_over(0, 6, IOC_OVERRIDE_OE);
  ioc_set_over(0, 7, IOC_OVERRIDE_OE);
  ioc_set_over(2, 0, IOC_OVERRIDE_OE);
  ioc_set_over(2, 1, IOC_OVERRIDE_OE);
  ioc_set_over(2, 2, IOC_OVERRIDE_OE);
  ioc_set_over(2, 3, IOC_OVERRIDE_OE);
  ioc_set_over(2, 4, IOC_OVERRIDE_OE);
  ioc_set_over(2, 5, IOC_OVERRIDE_OE);
  ioc_set_over(2, 6, IOC_OVERRIDE_OE);
  ioc_set_over(3, 0, IOC_OVERRIDE_OE);
  ioc_set_over(3, 1, IOC_OVERRIDE_OE);
  ioc_set_over(3, 2, IOC_OVERRIDE_OE);

	GPIO_SOFTWARE_CONTROL(GPIO_A_BASE,GPIO_PIN_MASK(6));
  GPIO_SOFTWARE_CONTROL(GPIO_A_BASE,GPIO_PIN_MASK(7));
	GPIO_SET_OUTPUT(GPIO_A_BASE,GPIO_PIN_MASK(6));		//GPIO PA6
	GPIO_SET_OUTPUT(GPIO_A_BASE,GPIO_PIN_MASK(7));		//GPIO PA7

  GPIO_SOFTWARE_CONTROL(GPIO_C_BASE,GPIO_PIN_MASK(0));		//GPIO PC0
	GPIO_SOFTWARE_CONTROL(GPIO_C_BASE,GPIO_PIN_MASK(1));		//GPIO PC1
  GPIO_SOFTWARE_CONTROL(GPIO_C_BASE,GPIO_PIN_MASK(2));		//GPIO PC2
  GPIO_SOFTWARE_CONTROL(GPIO_C_BASE,GPIO_PIN_MASK(3));		//GPIO PC3
	GPIO_SOFTWARE_CONTROL(GPIO_C_BASE,GPIO_PIN_MASK(4));		//GPIO PC4
	GPIO_SOFTWARE_CONTROL(GPIO_C_BASE,GPIO_PIN_MASK(5));		//GPIO PC5
  GPIO_SOFTWARE_CONTROL(GPIO_C_BASE,GPIO_PIN_MASK(6));		//GPIO PC6
  
 	GPIO_SET_OUTPUT(GPIO_C_BASE,GPIO_PIN_MASK(0));		//GPIO PC0
	GPIO_SET_OUTPUT(GPIO_C_BASE,GPIO_PIN_MASK(1));		//GPIO PC1
  GPIO_SET_OUTPUT(GPIO_C_BASE,GPIO_PIN_MASK(2));		//GPIO PC2
  GPIO_SET_OUTPUT(GPIO_C_BASE,GPIO_PIN_MASK(3));		//GPIO PC3
	GPIO_SET_OUTPUT(GPIO_C_BASE,GPIO_PIN_MASK(4));		//GPIO PC4
	GPIO_SET_OUTPUT(GPIO_C_BASE,GPIO_PIN_MASK(5));		//GPIO PC5
  GPIO_SET_OUTPUT(GPIO_C_BASE,GPIO_PIN_MASK(6));		//GPIO PC6

  GPIO_SET_INPUT(GPIO_D_BASE,GPIO_PIN_MASK(0));		//GPIO PD0
  GPIO_SET_INPUT(GPIO_D_BASE,GPIO_PIN_MASK(1));		//GPIO PD1
	GPIO_SET_INPUT(GPIO_D_BASE,GPIO_PIN_MASK(2));		//GPIO PD2

	GPIO_SET_OUTPUT(GPIO_D_BASE,GPIO_PIN_MASK(0));		//GPIO PD0
  GPIO_SET_OUTPUT(GPIO_D_BASE,GPIO_PIN_MASK(1));		//GPIO PD1
	GPIO_SET_OUTPUT(GPIO_D_BASE,GPIO_PIN_MASK(2));		//GPIO PD2

	GPIO_CLR_PIN(GPIO_A_BASE,GPIO_PIN_MASK(6));		//GPIO PA6
}
/*---------------------------------------------------------------------------*/
void
clear_GPIOS(void)
{
	GPIO_CLR_PIN(GPIO_A_BASE,GPIO_PIN_MASK(6));		//GPIO PA6
	
 	GPIO_CLR_PIN(GPIO_C_BASE,GPIO_PIN_MASK(0));		//GPIO PC0
	GPIO_CLR_PIN(GPIO_C_BASE,GPIO_PIN_MASK(1));		//GPIO PC1
  GPIO_CLR_PIN(GPIO_C_BASE,GPIO_PIN_MASK(2));		//GPIO PC2
  GPIO_CLR_PIN(GPIO_C_BASE,GPIO_PIN_MASK(3));		//GPIO PC3
	GPIO_CLR_PIN(GPIO_C_BASE,GPIO_PIN_MASK(4));		//GPIO PC4
	GPIO_CLR_PIN(GPIO_C_BASE,GPIO_PIN_MASK(5));		//GPIO PC5
  GPIO_CLR_PIN(GPIO_C_BASE,GPIO_PIN_MASK(6));		//GPIO PC6

	GPIO_CLR_PIN(GPIO_D_BASE,GPIO_PIN_MASK(0));		//GPIO PD0
  GPIO_CLR_PIN(GPIO_D_BASE,GPIO_PIN_MASK(1));		//GPIO PD1
	GPIO_CLR_PIN(GPIO_D_BASE,GPIO_PIN_MASK(2));		//GPIO PD2
}
/*---------------------------------------------------------------------------*/
/*
#if ENERGEST_CONF_ON
static void
send_energest()
{
  struct energestmsg energest_values;

  energest_values.totaltime = RTIMER_NOW() - prev_energest_vals.totaltime;

  energest_flush();
  energest_values.cpu = energest_type_time(ENERGEST_TYPE_CPU) - prev_energest_vals.cpu;
  energest_values.lpm = energest_type_time(ENERGEST_TYPE_LPM) - prev_energest_vals.lpm;
  energest_values.transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT) - prev_energest_vals.transmit;
  energest_values.listen = energest_type_time(ENERGEST_TYPE_LISTEN) - prev_energest_vals.listen;
  energest_values.seqno = seqno;

  LOG_INFO("Energest data sent to ");
	LOG_INFO_LLADDR(&energest_addr);
  LOG_INFO_("\n");

  rpl_buf = (uint8_t *) &energest_values;
	rpl_len = sizeof(energest_values);

	NETSTACK_NETWORK.output(&energest_addr);

  energest_flush();
  prev_energest_vals.cpu = energest_type_time(ENERGEST_TYPE_CPU);
  prev_energest_vals.lpm = energest_type_time(ENERGEST_TYPE_LPM);
  prev_energest_vals.transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
  prev_energest_vals.listen = energest_type_time(ENERGEST_TYPE_LISTEN);
  prev_energest_vals.totaltime = RTIMER_NOW();
}
#endif */ /* ENERGEST_CONF_ON */

/*---------------------------------------------------------------------------*/
static void
receiver(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  printf("Sender received data on port %d from port %d with length %d\n",
         receiver_port, sender_port, datalen);
}
/*---------------------------------------------------------------------------*/
static void
set_global_address(void)
{
  uip_ipaddr_t ipaddr;
  int i;
  uint8_t state;
  const uip_ipaddr_t *default_prefix = uip_ds6_default_prefix();

  uip_ip6addr_copy(&ipaddr, default_prefix);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

  printf("IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
      printf("\n");
    }
  }
}
/*---------------------------------------------------------------------------*/
int state = 0;
static void
send_packet()
{
  uip_ipaddr_t addr;
  const uip_ipaddr_t *default_prefix;
	struct testmsg msg;

	seqno++;

	/*Set general info*/
	msg.seqno=seqno;		
	msg.timestamp_app= clock_time();
	
	static uint8_t seqno_bits[IO_WIDTH];			
	uint8_t i;
	for (i = 0; i < IO_WIDTH; i++) {
		seqno_bits[i] = msg.seqno & (1 << i) ? 1 : 0;
	}		//least significant bit in seqno_bits[0]

  struct energestmsg energest_values;
  struct energestmsg buf;

  energest_values.totaltime = RTIMER_NOW() - prev_energest_vals.totaltime;

  energest_flush();
  energest_values.cpu = energest_type_time(ENERGEST_TYPE_CPU) - prev_energest_vals.cpu;
  energest_values.lpm = energest_type_time(ENERGEST_TYPE_LPM) - prev_energest_vals.lpm;
  energest_values.transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT) - prev_energest_vals.transmit;
  energest_values.listen = energest_type_time(ENERGEST_TYPE_LISTEN) - prev_energest_vals.listen;
  energest_values.seqno = seqno;
	
	clear_GPIOS();
	
	if ( seqno_bits[0]==1 )		GPIO_SET_PIN(GPIO_A_BASE,GPIO_PIN_MASK(6));       //  write a 1 in A6
	if ( seqno_bits[1]==1 )		GPIO_SET_PIN(GPIO_C_BASE,GPIO_PIN_MASK(0));       //  write a 1 in C0
	if ( seqno_bits[2]==1 )		GPIO_SET_PIN(GPIO_C_BASE,GPIO_PIN_MASK(1));       //  write a 1 in C1
	if ( seqno_bits[3]==1 )		GPIO_SET_PIN(GPIO_C_BASE,GPIO_PIN_MASK(2));       //  write a 1 in C2
	if ( seqno_bits[4]==1 )		GPIO_SET_PIN(GPIO_C_BASE,GPIO_PIN_MASK(3));       //  write a 1 in C3
	if ( seqno_bits[5]==1 )		GPIO_SET_PIN(GPIO_C_BASE,GPIO_PIN_MASK(4));       //  write a 1 in C4
	if ( seqno_bits[6]==1 )		GPIO_SET_PIN(GPIO_C_BASE,GPIO_PIN_MASK(5));       //  write a 1 in C5
	if ( seqno_bits[7]==1 )		GPIO_SET_PIN(GPIO_C_BASE,GPIO_PIN_MASK(6));       //  write a 1 in C6
	if ( seqno_bits[8]==1 )		GPIO_SET_PIN(GPIO_D_BASE,GPIO_PIN_MASK(0));       //  write a 1 in D0
	if ( seqno_bits[9]==1 )		GPIO_SET_PIN(GPIO_D_BASE,GPIO_PIN_MASK(1));       //  write a 1 in D1
	if ( seqno_bits[10]==1 )	GPIO_SET_PIN(GPIO_D_BASE,GPIO_PIN_MASK(2));       //  write a 1 in D2

	if (state == 0){
		GPIO_SET_PIN(GPIO_A_BASE,GPIO_PIN_MASK(7));
		state = 1;
	}
	else{
		GPIO_CLR_PIN(GPIO_PORT_TO_BASE(0),GPIO_PIN_MASK(7));
		state = 0;
	}

  //vanaf hier vragen zie ook send_packet
  default_prefix = uip_ds6_default_prefix();
  uip_ip6addr_copy(&addr, default_prefix);

  addr.u16[4] = UIP_HTONS(0x0212);
  addr.u16[5] = UIP_HTONS(0x4b00);
  addr.u16[6] = UIP_HTONS(0x18ec);
  addr.u16[7] = UIP_HTONS(0x28d6);

  
  static unsigned int message_number;

  printf("Sending unicast to ");
  uip_debug_ipaddr_print(&addr);
  printf("\n");
  memcpy(&buf, &energest_values, sizeof(energest_values));
  //sprintf(buf, "%x%x,%" PRIu16 ",%" PRIu32 ",%" PRIu32 ",%" PRIu32 ",%" PRIu32 ",%" PRIu32"\n\r", 
  //addr.u16[6],addr.u16[7],seqno, energest_values.cpu, energest_values.lpm, energest_values.transmit, energest_values.listen, energest_values.totaltime);
  message_number++;
  simple_udp_sendto(&unicast_connection, &buf, sizeof(buf), &addr);


  /*
  #if BC
    NETSTACK_NETWORK.output(NULL);
  #else
    NETSTACK_NETWORK.output(&recv_addr);
  #endif
  */

  energest_flush();
  prev_energest_vals.cpu = energest_type_time(ENERGEST_TYPE_CPU);
  prev_energest_vals.lpm = energest_type_time(ENERGEST_TYPE_LPM);
  prev_energest_vals.transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
  prev_energest_vals.listen = energest_type_time(ENERGEST_TYPE_LISTEN);
  prev_energest_vals.totaltime = RTIMER_NOW();
  
  /*
  #if ENERGEST_CONF_ON
    if (seqno%ENERGEST_FREQ==0)
    send_energest();
  #endif
  */
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(observed_sender_node_process, ev, data)
{
  //static struct etimer periodic_timer;
  static struct etimer send_timer;

  //imported
  prev_energest_vals.cpu = 0;
  prev_energest_vals.lpm = 0;
  prev_energest_vals.transmit = 0;
  prev_energest_vals.listen = 0;
  prev_energest_vals.seqno = 0;
	prev_energest_vals.totaltime = 0;

  PROCESS_BEGIN();

  GPIOS_init();

  set_global_address();

  simple_udp_register(&unicast_connection, UDP_PORT,
                      NULL, UDP_PORT, receiver);

  //etimer_set(&periodic_timer, SEND_INTERVAL);
  etimer_set(&send_timer, SEND_INTERVAL);
  while(1) {
    PROCESS_YIELD();
    if(ev == PROCESS_EVENT_TIMER){
      
      if(data == &send_timer){
        send_packet();  
        etimer_reset(&send_timer);
      }
      
    }
    
    //PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&send_timer));
      
    //etimer_reset(&send_timer);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
