#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "assert.h"
#include <ogg/ogg.h>

#include "opus_encoder.h"

//https://chromium.googlesource.com/chromium/deps/opus/+/3c3902f0ac13428394f14f78f0fab05ef3468d69/doc/trivial_example.c

//TODO: KOLLA IN http://stackoverflow.com/questions/12516650/opus-audio-codec-encoding-for-iphone
//https://review.webrtc.org/10489004/
//http://holdenc.altervista.org/parole/


/* write functions, be */

static void write_be64(uint8_t *p, uint64_t v)
{
	p[0] = (v >> 56) & 0xFF;
	p[1] = (v >> 48) & 0xFF;
	p[2] = (v >> 40) & 0xFF;
	p[3] = (v >> 32) & 0xFF;
	p[4] = (v >> 24) & 0xFF;
	p[5] = (v >> 16) & 0xFF;
	p[6] = (v >>  8) & 0xFF;
	p[7] = (v      ) & 0xFF;
}

static void write_be32(uint8_t *p, uint32_t v)
{
	p[0] = (v >> 24) & 0xFF;
	p[1] = (v >> 16) & 0xFF;
	p[2] = (v >>  8) & 0xFF;
	p[3] = (v      ) & 0xFF;
}

static void write_be16(uint8_t *p, uint16_t v)
{
	p[0] = (v >>  8) & 0xFF;
	p[1] = (v      ) & 0xFF;
}

/* write functions, le */

static void write_le64(uint8_t *p, uint64_t v)
{
	p[0] = (v      ) & 0xFF;
	p[1] = (v >>  8) & 0xFF;
	p[2] = (v >> 16) & 0xFF;
	p[3] = (v >> 24) & 0xFF;
	p[4] = (v >> 32) & 0xFF;
	p[5] = (v >> 40) & 0xFF;
	p[6] = (v >> 48) & 0xFF;
	p[7] = (v >> 56) & 0xFF;
}

static void write_le32(uint8_t *p, uint32_t v)
{
	p[0] = (v      ) & 0xFF;
	p[1] = (v >>  8) & 0xFF;
	p[2] = (v >> 16) & 0xFF;
	p[3] = (v >> 24) & 0xFF;
}

static void write_le16(uint8_t *p, uint16_t v)
{
	p[0] = (v      ) & 0xFF;
	p[1] = (v >>  8) & 0xFF;
}


/* read functions, be */

static uint64_t read_be64(const uint8_t *p)
{
	int i;
	uint64_t r = 0;
    
	for (i = 0; i < 8; i++)
		r |= p[i] << (7-i)*8;
    
	return r;
}

static uint32_t read_be32(const uint8_t *p)
{
	return ((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
}

static uint16_t read_be16(const uint8_t *p)
{
	return ((p[0] << 8) | p[1]);
}

/* read functions, le */

static uint64_t read_le64(const uint8_t *p)
{
	int i;
	uint64_t r = 0;
    
	for (i = 0; i < 8; i++)
		r |= p[i] << i*8;
    
	return r;
}

static uint32_t read_le32(const uint8_t *p)
{
	return ((p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0]);
}

static uint16_t read_le16(const uint8_t *p)
{
	return ((p[1] << 8) | p[0]);
}


#define DEBUGOGG 0

#if DEBUGOGG
static void print_opacket_info(ogg_packet *opacket)
{
	fprintf(stderr, "P(%5ld) %s%s no=0x%016lx gpos=0x%016lx\n", opacket->bytes,
			opacket->b_o_s ? "B" : " ", opacket->e_o_s ? "E" : " ",
			opacket->packetno, opacket->granulepos);
}

static void print_opage_info(ogg_page *opage)
{
	fprintf(stderr, "  page: serialno=0x%08x %s%s no=%08ld C%d P%02d gpos=0x%016lx len=%ld\n",
			ogg_page_serialno(opage), ogg_page_bos(opage) ? "B" : " ", ogg_page_eos(opage) ? "E" : " ",
			ogg_page_pageno(opage), ogg_page_continued(opage), ogg_page_packets(opage),
			ogg_page_granulepos(opage), opage->header_len + opage->body_len);
}
#endif

static ogg_packet *make_opus_header0_oggpacket(unsigned char channels, unsigned int samplerate)
{
	int size = 19;
	ogg_packet *op;
	unsigned char *data;
    
	op   = malloc(sizeof(*op));
	data = malloc(size);
	if (!op || !data)
		return NULL;
    
	memcpy(data, "OpusHead", 8);     /* identifier */
	data[8] = 1;                     /* version */
	data[9] = channels;              /* channels */
	write_le16(data+10, 0);          /* pre-skip */
	write_le32(data+12, samplerate); /* original sample rate */
	write_le16(data+16, 0);          /* gain */
	data[18] = 0;                    /* channel mapping family */
    
	op->packet     = data;
	op->bytes      = size;
	op->b_o_s      = 1;
	op->e_o_s      = 0;
	op->granulepos = 0;
	op->packetno   = 0;
    
	return op;
}

static ogg_packet *make_opus_header1_oggpacket(char *vendor)
{
	char *identifier = "OpusTags";
    //	char *vendor     = "test";
	int size = strlen(identifier) + 4 + strlen(vendor) + 4;
	ogg_packet *op;
	unsigned char *data;
    
	op   = malloc(sizeof(*op));
	data = malloc(size);
	if (!op || !data)
		return NULL;
    
	memcpy(data, identifier, 8);
	write_le32(data+8, strlen(vendor));
	memcpy(data+12, vendor, strlen(vendor));
	write_le32(data+12+strlen(vendor), 0);
    
	op->packet     = data;
	op->bytes      = size;
	op->b_o_s      = 0;
	op->e_o_s      = 0;
	op->granulepos = 0;
	op->packetno   = 1;
    
	return op;
}

static int write_ogg_packet(ogg_stream_state *ostream, ogg_packet *opacket, FILE *fp, int flush)
{
	ogg_page opage;
    
#if DEBUGOGG
	print_opacket_info(opacket);
#endif
	/* Submit a raw packet to the streaming layer */
	if (ogg_stream_packetin(ostream, opacket) == 0) {
		/* Output a completed page if the stream contains enough packets to form a full page. */
		while (flush ? ogg_stream_flush(ostream, &opage) : ogg_stream_pageout(ostream, &opage)) {
#if DEBUGOGG
			print_opage_info(&opage);
#endif
			fwrite(opage.header, 1, opage.header_len, fp);
			fwrite(opage.body  , 1, opage.body_len,   fp);
		}
	} else {
		fprintf(stderr, "Error on ogg_stream_packetin().\n");
		return -1;
	}
	return 0;
}


ogg_stream_state ostream_rec_local;

drError drOpusEncoder_init(void* opusEncoder, const char* filePath, float fs, float numChannels)
{
    
    int oggInitResult = ogg_stream_init(&ostream_rec_local, rand());
    assert(oggInitResult >= 0);
    
    
    
    assert(numChannels <= 2);
    
    drOpusEncoder* encoder = (drOpusEncoder*)opusEncoder;
    encoder->numAccumulatedInputFrames = 0;
    int err;
    assert(encoder->encoder == 0);
    //TODO: sample rate must match system rate!
    encoder->encoder = opus_encoder_create(48000, numChannels, OPUS_APPLICATION_AUDIO, &err);
    if (err < 0)
    {
        fprintf(stderr, "failed to create an encoder: %s\n", opus_strerror(err));
        //assert(err >= 0 && "failed to create opus encoder");
        return DR_FAILED_TO_INITIALIZE_ENCODER;
    }
    
    err = opus_encoder_ctl(encoder->encoder, OPUS_SET_BITRATE(BITRATE));
    assert(err >= 0 && "failed to set oupus encoder bitrate");
    
    assert(encoder->file == 0);
    encoder->file = fopen(filePath, "w");
    if (encoder->file == 0)
    {
        return DR_FAILED_TO_OPEN_ENCODER_TARGET_FILE;
    }
    
    return DR_NO_ERROR;
}

drError drOpusEncoder_write(void* opusEncoder, int numChannels, int numFrames, float* buffer)
{
    drOpusEncoder* encoder = (drOpusEncoder*)opusEncoder;
    
    //accumulate buffer to encode
    for (int i = 0; i < numFrames; i++)
    {
        for (int c = 0; c < numChannels; c++)
        {
            int destIdx = encoder->numAccumulatedInputFrames * numChannels + c;
            int srcIdx = numChannels * i + c;
            encoder->scratchInputBuffer[destIdx] = buffer[srcIdx];
            encoder->numAccumulatedInputFrames++;
            if (encoder->numAccumulatedInputFrames == FRAME_SIZE)
            {
                encoder->numAccumulatedInputFrames = 0;
                int encodedPacketSize = opus_encode_float(encoder->encoder,
                                                          encoder->scratchInputBuffer,
                                                          FRAME_SIZE,
                                                          encoder->scratchOutputBuffer,
                                                          MAX_PACKET_SIZE);
                if (encodedPacketSize < 0)
                {
                    fprintf(stderr, "encode failed: %s\n", opus_strerror(encodedPacketSize));
                    return DR_FAILED_TO_ENCODE_AUDIO_DATA;
                }
                else
                {
                    printf("encoded opus packet of size %d\n", encodedPacketSize);
                    int n = fwrite(encoder->scratchOutputBuffer, 1, encodedPacketSize, encoder->file);
                    if (n != encodedPacketSize)
                    {
                        return DR_FAILED_TO_WRITE_ENCODED_AUDIO_DATA;
                    }
                }
            }
        }
    }
    
    return DR_NO_ERROR;
}

drError drOpusEncoder_finish(void* opusEncoder)
{
    drOpusEncoder* encoder = (drOpusEncoder*)opusEncoder;
    opus_encoder_destroy(encoder->encoder);
    encoder->encoder = 0;
    
    if (fclose(encoder->file) != 0)
    {
        return DR_FAILED_TO_CLOSE_ENCODER_TARGET_FILE;
    }
    
    encoder->file = 0;
    
    return DR_NO_ERROR;
}

drError drOpusEncoder_cancel(void* opusEncoder)
{
    drOpusEncoder* encoder = (drOpusEncoder*)opusEncoder;
    opus_encoder_destroy(encoder->encoder);
    encoder->encoder = 0;
    
    if (fclose(encoder->file) != 0)
    {
        return DR_FAILED_TO_CLOSE_ENCODER_TARGET_FILE;
    }
    
    encoder->file = 0;
    
    return DR_NO_ERROR;
}
