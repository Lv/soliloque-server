#include "control_packet.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <resolv.h>
#include <string.h>

#include "server.h"
#include "channel.h"
#include "acknowledge_packet.h"
#include "array.h"
#include "packet_tools.h"

extern struct server *ts_server;
extern int socket_desc;
/**
 * Reply to a c_req_chans by sending packets containing
 * a data dump of the channels.
 *
 * @param pl the player we send the channel list to
 * @param s the server we will get the channels from
 *
 * TODO : split the channels over packets
 */
void s_resp_chans(struct player *pl, struct server *s)
{
	char *data;
	int data_size = 0;
	struct channel *ch;
	char *ptr;
	int ch_size;
	/* compute the size of the packet */
	data_size += 24;	/* header */
	data_size += 4;		/* number of channels in packet */
	ar_each(struct channel *, ch, s->chans)
		data_size += channel_to_data_size(ch);
	ar_end_each;

	/* initialize the packet */
	data = (char *)calloc(data_size, sizeof(char));
	ptr = data;
	*(uint32_t *)ptr = 0x0006bef0;			ptr+=4;		/* */
	*(uint32_t *)ptr = pl->private_id;		ptr+=4;		/* player private id */
	*(uint32_t *)ptr = pl->public_id;		ptr+=4;		/* player public id */
	*(uint32_t *)ptr = 0x00000000;			ptr+=4;		/* packet counter */
	/* packet version */				ptr+=4;
	/* empty checksum */				ptr+=4;
	*(uint32_t *)ptr = s->chans->used_slots;	ptr+=4;		/* number of channels sent */
	/* dump the channels to the packet */
	ar_each(struct channel *, ch, s->chans)
		ch_size = channel_to_data_size(ch);
		channel_to_data(ch, ptr);
		ptr+=ch_size;
	ar_end_each;

	packet_add_crc_d(data, data_size);

	printf("size of all channels : %i\n", data_size);
	sendto(socket_desc, data, data_size, 0, (struct sockaddr *)pl->cli_addr, pl->cli_len);
}

/**
 * Handles a request for channels by the player.
 *
 * @param data the request packet
 * @param len the length of data
 * @param cli_addr the address of the client
 * @param cli_len the size of cli_addr
 */
void *c_req_chans(char *data, unsigned int len, struct sockaddr_in *cli_addr, unsigned int cli_len)
{
	uint32_t pub_id, priv_id;
	struct player *pl;

	memcpy(&priv_id, data+4, 4);
	memcpy(&pub_id, data+8, 4);
	pl = get_player_by_ids(ts_server, pub_id, priv_id);

	if(pl != NULL) {
		send_acknowledge(pl);
		s_resp_chans(pl, ts_server);
	}
	return NULL;
}
