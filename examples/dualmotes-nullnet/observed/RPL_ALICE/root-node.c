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
#include "lib/random.h"
#include "sys/ctimer.h"
#include "sys/etimer.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-debug.h"
#include "net/routing/routing.h"
#include "net/ipv6/simple-udp.h"

#include "project-conf.h"
#include <inttypes.h>

#include <stdio.h>
#include <string.h>

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define UDP_PORT 1234
#define SERVICE_ID 190

#define SEND_INTERVAL		(10 * CLOCK_SECOND)
#define SEND_TIME		(random_rand() % (SEND_INTERVAL))

// message counters
static uint16_t received = 0 ;

static struct simple_udp_connection unicast_connection;

/*---------------------------------------------------------------------------*/
PROCESS(unicast_receiver_process, "Unicast receiver example process");
AUTOSTART_PROCESSES(&unicast_receiver_process);
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
  
  if(datalen == sizeof(struct energestmsg)) {
        //rtimer_clock_t rtime = RTIMER_NOW();        //received time (for the latency)
        struct energestmsg msg;
        memcpy(&msg, data, datalen);
        received ++;
        //uint16_t timestamp = packetbuf_attr(PACKETBUF_ATTR_TIMESTAMP);        

        printf("%x%x,%" PRIu16 ",%" PRIu32 ",%" PRIu32 ",%" PRIu32 ",%" PRIu32 ",%" PRIu32"\n\r",
                sender_addr->u8[sizeof(sender_addr->u8) - 2], sender_addr->u8[sizeof(sender_addr->u8) - 1], msg.seqno, 
                msg.cpu, msg.lpm, msg.transmit, msg.listen, msg.totaltime);
  }
  else {
  printf("Data received from ");
  uip_debug_ipaddr_print(sender_addr);
  printf(" on port %d from port %d with length %d: '%s'\n",
        receiver_port, sender_port, datalen, data);
  }
}

/*---------------------------------------------------------------------------*/
/*void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
    if(len == sizeof(struct energestmsg)) {
        //rtimer_clock_t rtime = RTIMER_NOW();        //received time (for the latency)
        struct energestmsg msg;
        memcpy(&msg, data, len);
        received ++;
        //uint16_t timestamp = packetbuf_attr(PACKETBUF_ATTR_TIMESTAMP);        

        printf("%x%x,%" PRIu16 ",%" PRIu32 ",%" PRIu32 ",%" PRIu32 ",%" PRIu32 ",%" PRIu32"\n\r",
                src->u8[sizeof(src->u8) - 2], src->u8[sizeof(src->u8) - 1], msg.seqno, 
                msg.cpu, msg.lpm, msg.transmit, msg.listen, msg.totaltime);
    }
}*/
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(unicast_receiver_process, ev, data)
{
  PROCESS_BEGIN();

  NETSTACK_ROUTING.root_start();

  simple_udp_register(&unicast_connection, UDP_PORT,NULL, UDP_PORT, receiver);
  //nullnet_set_input_callback(input_callback);

  while(1) {
    //PROCESS_WAIT_EVENT();
    PROCESS_YIELD();
  }

  printf("process ended -> whileloop terminated");
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
