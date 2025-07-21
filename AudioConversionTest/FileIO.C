
#include <stdint.h>
#include <stdio.h>      /* FILE; fclose(), fopen(), fread(), frite(), fseek(); */

#define LEN_WAV_HEADER 44  /* 0x2C */

int main(int argc, char * argv[])
{
	FILE * file_handle = NULL;
	size_t ret_code = 0;
	int itr = 0;
	uint8_t lbx_entry_data[856] = { 0 };
	uint8_t * data_rvr = NULL;
    uint8_t wav_header[LEN_WAV_HEADER] = {
		/* [Master RIFF chunk] */
        /*00*/ 'R', 'I', 'F', 'F',	/* RIFF Chunk ID: "RIFF"   */
        /*04*/ 0, 0, 0, 0,  		/* RIFF Chunk Body Size    */	/* (size in bytes - 8 bytes for header) (FourCC and Chunk Size */
        /*08*/ 'W', 'A', 'V', 'E',	/* RIFF Form Type: "WAVE"  */
		/* [Chunk describing the data format] */
        /*0c*/ 'f', 'm', 't', ' ',	/* Format Chunk ID: "fmt " */
        /*10*/ 16, 0, 0, 0, 		/* Format Chunk Body Size  */	/* length of fmt chunk */
		/* Format Chunk Body */
        /*14*/ 1, 0,        										/* audio format (1: PCM integer, 3: PCM IEEE 754 float) */
        /*16*/ 1, 0,        										/* channels */
        /*18*/ 0x94, 0x15, 0, 0,  										/* number of sample frames that occur each second  Hz */
        /*1c*/ 0x94, 0x15, 0, 0,  										/* bytes required for one second of audio data  Hz * 2B/sample * stereo */
        /*20*/ 1, 0,        										/* block align */
        /*22*/ 8, 0,       											/* bits per sample */
		/* [Chunk containing the sampled data] */
        /*24*/ 'd', 'a', 't', 'a',	/* Data Chunk ID: "data" */
        /*28*/ 0, 0, 0, 0   		/* Data Chunk Body Size */
		/* Data Chunk Body */
    };
	uint32_t voc_len = 0;
	uint8_t * voc_ptr = NULL;
	uint8_t voc_block_type;
	uint8_t voc_block_sample_rate = 0;
	uint8_t voc_block_codec_id = 0;
	uint32_t voc_block_size = 0;
	uint32_t voc_block_frequency = 0;
	uint8_t sample_8bit_unsigned = 0;
	uint8_t wav_data[1024] = { 0 };
	uint8_t * wav_ptr = NULL;
	uint32_t wav_sample_count = 0;
	uint16_t wav_sample_size = 0;
	uint16_t wav_channel_count = 0;
	uint16_t wav_sample_frame_size = 0;
	uint32_t wav_data_size = 0;
	
	printf("BEGIN\n");
	
	file_handle = fopen("SOUNDFX.LBX", "rb");
	if(file_handle == NULL)
	{
		printf("ERROR: fopen()\n");
	}
	fseek(file_handle, 4224, 0);
	ret_code = fread(lbx_entry_data, 1, 856, file_handle);
	printf("ret_code: %zu\n", ret_code);
	if(ret_code == 856)
    {
        // for (itr = 0; itr != 856; itr++)
        //     printf("%02X ", lbx_entry_data[itr]);
        // putchar('\n');
    }
    else // error handling
    {
        if (feof(file_handle))
            printf("Error reading test.bin: unexpected end of file\n");
        else if (ferror(file_handle))
            perror("Error reading test.bin");
    }
	fclose(file_handle);
	file_handle = NULL;
	data_rvr = &lbx_entry_data[16];
	// printf("%c\n", *data_rvr);
	
	wav_ptr = &wav_data[0];
	wav_sample_count = 0;
	wav_sample_size = 1;  // 1 byte per sample - 8-bit, mono
	wav_channel_count = 1;  // 1 channel AKA mono
	wav_sample_frame_size = wav_sample_size * wav_channel_count;  // 1 channel * 1 byte per sample
	voc_len = 856-16-26;
	voc_ptr = &lbx_entry_data[16+26];
	while(voc_len)
	{
		printf("voc_len: %u\n", voc_len);
		
		voc_block_type = voc_ptr[0];
		printf("voc_block_type: %u\n", voc_block_type);
		
		switch(voc_block_type)
		{
            case 0:  /* VOC_TYPE_EOF */
            {
				printf("VOC_TYPE_EOF\n");
                voc_len -= 1;  // -1 for block type
            } break;
            case 1:  /* VOC_TYPE_VOICE_DATA */
            {
				printf("VOC_TYPE_VOICE_DATA\n");
                voc_block_sample_rate = voc_ptr[4];
				printf("voc_block_sample_rate: %u\n", voc_block_sample_rate);
                voc_block_codec_id = voc_ptr[5];
				printf("voc_block_codec_id: %u\n", voc_block_codec_id);
                voc_block_size = voc_ptr[3] << 16;
				voc_block_size |= voc_ptr[2] << 8;
				voc_block_size |= voc_ptr[1];
				voc_block_size -= 2;  // -2 for sample rate and codec id
				printf("voc_block_size: %u\n", voc_block_size);
                voc_len -= 6;  // -6 for block type, block size, sample rate, and codec id
                voc_ptr += 6;  // +6 for block type, block size, sample rate, and codec id
                voc_block_frequency = 1000000 / (256 - voc_block_sample_rate);  // sample rate in hertz
				printf("voc_block_frequency: %u\n", voc_block_frequency);
                // wav_frequency = voc_block_frequency;
                if(voc_block_codec_id != 0)
                {
                    __debugbreak();
                }
				// if(((wav_sample_count + voc_block_size) * 2 * 2) > curr_wav_buf_size)
				// tmp_wav_data = realloc(wav_data, new_wav_buf_size);
				wav_sample_count += voc_block_size;
				voc_len -= voc_block_size;
                while(voc_block_size--)
                {
                    // convert 8-bit, unsigned to 16-bit, signed
                    // sample_8bit_unsigned = *voc_ptr++;
                    // sample_16bit_signed = (int16_t)(sample_8bit_unsigned << 8);
                    // Create stereo data by duplicating the 16-bit value
                    // *wav_ptr++ = sample_16bit_signed;
                    // *wav_ptr++ = sample_16bit_signed;
					sample_8bit_unsigned = *voc_ptr++;
					*wav_ptr++ = sample_8bit_unsigned;
                }
			} break;
			default:
			{
				__debugbreak();
			} break;
		}
	}
	
	printf("wav_sample_count: %u\n", wav_sample_count);
	
	wav_data_size = wav_sample_count * wav_sample_frame_size;
	
	printf("wav_data_size: %u\n", wav_data_size);
	
    // wav_header[0x04] = ((44 - 8) + wav_data_size);  /* File size (integer)  (total file size - 8 bytes), in bytes (32-bit integer) */
	wav_header[0x04] = ((((44 - 8) + wav_data_size) >>  0) & 0x000000FF);
	wav_header[0x05] = ((((44 - 8) + wav_data_size) >>  8) & 0x000000FF);
	wav_header[0x06] = ((((44 - 8) + wav_data_size) >> 16) & 0x000000FF);
	wav_header[0x07] = ((((44 - 8) + wav_data_size) >> 24) & 0x000000FF);
    // wav_header[0x18] = audiorate);                            /* Sample Rate - 32 bit integer. Common values are 44100 (CD), 48000 (DAT). Sample Rate = Number of Samples per second, or Hertz */
    // wav_header[0x1C] = ((audiorate * 16 * 2) / 8));           /* (Sample Rate * BitsPerSample * Channels) / 8 */
    // wav_header[0x28] = wav_data_size;                        /* File size (data)  Size of the data section */
	wav_header[0x28] = ((wav_data_size >>  0) & 0x000000FF);
	wav_header[0x29] = ((wav_data_size >>  8) & 0x000000FF);
	wav_header[0x2A] = ((wav_data_size >> 16) & 0x000000FF);
	wav_header[0x2B] = ((wav_data_size >> 24) & 0x000000FF);
	
	file_handle = fopen("SLX19.WAV", "wb");
	if(file_handle == NULL)
	{
		printf("ERROR: fopen()\n");
	}
	fwrite(wav_header, 1, LEN_WAV_HEADER, file_handle);
	fwrite(wav_data, 1, wav_data_size, file_handle);
	fclose(file_handle);


	// file_handle = fopen("SLX19.VOC", "wb");
	// if(file_handle == NULL)
	// {
	// 	printf("ERROR: fopen()\n");
	// }
	// fwrite(data_rvr, 1, 840, file_handle);
	// fclose(file_handle);
	
	return 0;
}
