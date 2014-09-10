#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "mem.h"

#include "opus_encoder.h"

//https://chromium.googlesource.com/chromium/deps/opus/+/3c3902f0ac13428394f14f78f0fab05ef3468d69/doc/trivial_example.c

//TODO: KOLLA IN http://stackoverflow.com/questions/12516650/opus-audio-codec-encoding-for-iphone
//https://review.webrtc.org/10489004/
//http://holdenc.altervista.org/parole/


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
    
	op   = DR_MALLOC(sizeof(*op), "ogg header packet");
	data = DR_MALLOC(size, "ogg header packet data");
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
    
	op   = DR_MALLOC(sizeof(*op), "ogg header packet");
	data = DR_MALLOC(size, "ogg header packet data");
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

static int write_ogg_packet(ogg_stream_state *ostream, ogg_packet *opacket, FILE *fp, int flush, int* numBytesWritten)
{
    *numBytesWritten = 0;
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
            //TODO: check result here
			size_t n1 = fwrite(opage.header, 1, opage.header_len, fp);
            if (n1 > 0) *numBytesWritten += n1;
			size_t n2 = fwrite(opage.body  , 1, opage.body_len, fp);
            if (n2 > 0) *numBytesWritten += n2;
		}
	} else {
		fprintf(stderr, "Error on ogg_stream_packetin().\n");
		return -1;
	}
	return 0;
}


drError drOpusEncoder_init(void* opusEncoder, const char* filePath, float fs, float numChannels)
{
    assert(numChannels <= 2);
    
    drOpusEncoder* encoder = (drOpusEncoder*)opusEncoder;
    encoder->numAccumulatedInputFrames = 0;
    
    //create the opus encoder
    {
        int err;
        assert(encoder->encoder == 0);
        //TODO: sample rate must match system rate!
        encoder->encoder = opus_encoder_create(fs, numChannels, OPUS_APPLICATION_AUDIO, &err);
        if (err < 0)
        {
            fprintf(stderr, "failed to create an encoder: %s\n", opus_strerror(err));
            //assert(err >= 0 && "failed to create opus encoder");
            return DR_FAILED_TO_INITIALIZE_ENCODER;
        }
        
        err = opus_encoder_ctl(encoder->encoder, OPUS_SET_BITRATE(BITRATE));
        assert(err >= 0 && "failed to set oupus encoder bitrate");
    }
    
    //open file to write to
    {
        assert(encoder->file == 0);
        encoder->file = fopen(filePath, "a+");
        
        if (encoder->file == 0)
        {
            return DR_FAILED_TO_OPEN_ENCODER_TARGET_FILE;
        }
    }
    
    //init ogg container file
    const int serialNo = rand();
    int oggInitResult = ogg_stream_init(&encoder->oggStreamState, serialNo);
    assert(oggInitResult >= 0);
    
    encoder->oggHeaderPacket0 = make_opus_header0_oggpacket(numChannels, fs);
    assert(encoder->oggHeaderPacket0);
    encoder->oggHeaderPacket1 = make_opus_header1_oggpacket("digger recorder");
    assert(encoder->oggHeaderPacket1);
    
    return DR_NO_ERROR;
}

drError drOpusEncoder_write(void* opusEncoder, int numChannels, int numFrames, float* buffer, int* numBytesWritten)
{
    drOpusEncoder* encoder = (drOpusEncoder*)opusEncoder;
    *numBytesWritten = 0;
    
    if (encoder->encoder == NULL)
    {
        //the encoder was not created properly. do nothing
        return DR_FAILED_TO_WRITE_ENCODED_AUDIO_DATA;
    }
    
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
                    //fprintf(stderr, "encode failed: %s\n", opus_strerror(encodedPacketSize));
                    return DR_FAILED_TO_ENCODE_AUDIO_DATA;
                }
                else
                {
                    int totalBytesWritten = 0;
                    if (!encoder->hasWrittenHeaderPackets)
                    {
                        int b = 0;
                        int result = write_ogg_packet(&encoder->oggStreamState, encoder->oggHeaderPacket0, encoder->file, 1, &b);
                        assert(result >= 0);
                        totalBytesWritten += b;
                        result     = write_ogg_packet(&encoder->oggStreamState,  encoder->oggHeaderPacket1, encoder->file, 1, &b);
                        assert(result >= 0);
                        totalBytesWritten += b;
                        
                        encoder->hasWrittenHeaderPackets = 1;
                    }
                    
                    // forge ogg packet
                    ogg_packet opacket;
                    memset(&opacket, 0, sizeof(ogg_packet));
                    opacket.packet     = encoder->scratchOutputBuffer;
                    opacket.bytes      = encodedPacketSize;
                    opacket.b_o_s      = 0;//encoder->packetsWritten == 0 ? 1 : 0;
                    opacket.e_o_s      = 0;
                    opacket.granulepos = (encoder->packetsWritten + 1) * FRAME_SIZE;
                    opacket.packetno   = encoder->packetsWritten + 1;
                    encoder->packetsWritten++;
                    
                    int b = 0;
                    int writeResult = write_ogg_packet(&encoder->oggStreamState, &opacket, encoder->file, 0, &b);
                    
                    totalBytesWritten += b;
                    
                    *numBytesWritten = totalBytesWritten;
                    
                    //int n = fwrite(encoder->scratchOutputBuffer, 1, encodedPacketSize, encoder->file);
                    if (writeResult < 0)
                    {
                        return DR_FAILED_TO_WRITE_ENCODED_AUDIO_DATA;
                    }
                }
            }
        }
    }
    
    return DR_NO_ERROR;
}

static drError cleanup(drOpusEncoder* encoder)
{
    opus_encoder_destroy(encoder->encoder);
    ogg_stream_clear(&encoder->oggStreamState);
    
    DR_FREE(encoder->oggHeaderPacket0->packet);
    DR_FREE(encoder->oggHeaderPacket0);
    
    DR_FREE(encoder->oggHeaderPacket1->packet);
    DR_FREE(encoder->oggHeaderPacket1);
    
    FILE* f = encoder->file;
    
    memset(encoder, 0, sizeof(drOpusEncoder));
    
    if (f)
    {
        if (fclose(f) != 0)
        {
            return DR_FAILED_TO_CLOSE_ENCODER_TARGET_FILE;
        }
    }
    
    
    return DR_NO_ERROR;
    
}

drError drOpusEncoder_finish(void* opusEncoder)
{
    return cleanup((drOpusEncoder*)opusEncoder);
}

drError drOpusEncoder_cancel(void* opusEncoder)
{
    return cleanup((drOpusEncoder*)opusEncoder);
}
