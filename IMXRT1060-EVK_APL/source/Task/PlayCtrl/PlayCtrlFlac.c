/**
 * @file		PlaCtrlflac.h
 * @brief		Implementation Class
 * @date		2019/10/14
 * @version     0.1
 * @details 
 * --
 * License Type <MIT License>
 * --
 * Copyright 2019 Takashi Kashiwagi
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 *
 * @par Update:
 * - 2019/10/14: Takashi Kashiwagi:
 */
#include "Task/PlayCtrl/PlayCtrlFlac.h"
#include "mimiclib.h"

static FLAC__StreamDecoder *s_pstFlacDecoder = NULL;


static uint32_t s_u32SampleCnt;
static uint8_t s_u8PCMBuffer[96000*2*sizeof(uint32_t)];

static FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data)
{
	size_t i;

	(void)decoder;

	
	if((frame->header.bits_per_sample != 16) && (frame->header.bits_per_sample != 24)){
		mimic_printf("ERROR: this example only supports 16bit stereo streams\r\n");
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}
	if(frame->header.channels != 2) {
		mimic_printf("ERROR: This frame contains %d channels (should be 2)\r\n", frame->header.channels);
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}
	if(buffer [0] == NULL) {
		mimic_printf("ERROR: buffer [0] is NULL\r\n");
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}
	if(buffer [1] == NULL) {
		mimic_printf("ERROR: buffer [1] is NULL\r\n");
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}

	/* write decoded PCM samples */
	s_u32SampleCnt = 0;

	if(frame->header.bits_per_sample == 16){
		int16_t *i16PCMBuffer = (int16_t *)s_u8PCMBuffer;
		for(i = 0; i < frame->header.blocksize; i++) {
			i16PCMBuffer[2*i] = (FLAC__int16)buffer[0][i];
			i16PCMBuffer[2*i + 1] = (FLAC__int16)buffer[1][i];
			s_u32SampleCnt++;
		}
	}else{
		int32_t *i32PCMBuffer = (int32_t *)s_u8PCMBuffer;
		for(i = 0; i < frame->header.blocksize; i++) {
			i32PCMBuffer[2*i] = buffer[0][i];
			i32PCMBuffer[2*i + 1] = buffer[1][i];
			s_u32SampleCnt++;
		}
	}

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static uint32_t s_u32BitPerSample = 0;
static uint32_t s_u32SampleRate = 0;
static uint32_t s_u32Ch = 0;

static void metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
	(void)decoder, (void)client_data;

	/* print some stats */
	if(metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
		/* save for later */
		uint64_t total_samples = metadata->data.stream_info.total_samples;
		s_u32SampleRate = metadata->data.stream_info.sample_rate;
		s_u32Ch = metadata->data.stream_info.channels;
		s_u32BitPerSample = metadata->data.stream_info.bits_per_sample;

		mimic_printf("sample rate    : %u Hz\r\n", s_u32SampleRate);
		mimic_printf("channels       : %u\r\n", s_u32Ch);
		mimic_printf("bits per sample: %u\r\n", s_u32BitPerSample);
		mimic_printf("total samples  : %llu\r\n", total_samples);
	}
}

static void error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
	(void)decoder, (void)client_data;

	mimic_printf("Got error callback: %s\n", FLAC__StreamDecoderErrorStatusString[status]);
}


uint8_t * OpenFlacFile(const TCHAR szFilePath[], stCodecCondition_t *pst, uint32_t *pu32PCMBufferSize){
	uint32_t sizeofpcm;
	uint8_t *pu8PCMBuffer = NULL;
	FLAC__StreamDecoderInitStatus init_status;

	s_u32BitPerSample = 0;
	s_u32SampleRate = 0;
	s_u32Ch = 0;

	s_pstFlacDecoder = FLAC__stream_decoder_new();
	if(s_pstFlacDecoder == NULL) {
		return NULL;
	}

	FLAC__stream_decoder_set_md5_checking(s_pstFlacDecoder, true);

	init_status = FLAC__stream_decoder_init_file(s_pstFlacDecoder, szFilePath, write_callback, metadata_callback, error_callback, NULL);
	if(init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
		FLAC__stream_decoder_delete(s_pstFlacDecoder);
		s_pstFlacDecoder = NULL;
		return NULL;
	}

	if(FLAC__stream_decoder_process_until_end_of_metadata(s_pstFlacDecoder) == false){
		FLAC__stream_decoder_delete(s_pstFlacDecoder);
		s_pstFlacDecoder = NULL;
		return NULL;
	}

	sizeofpcm = pst->enBitsWidth / 8;
	{
		if( (DEF_BUFFER_QUEUE_SIZE % pst->nChannels) == 0)
		{
			*pu32PCMBufferSize = DEF_BUFFER_SAMPLE_SIZE * DEF_BUFFER_QUEUE_SIZE * sizeofpcm;
		}
		else
		{
			*pu32PCMBufferSize = DEF_BUFFER_SAMPLE_SIZE * pst->nChannels * sizeofpcm;
		}
		mimic_printf("[%s (%d)] stFormat.nChannels = %lu\r\n", __func__, __LINE__, pst->nChannels);
		mimic_printf("[%s (%d)] *pu32PCMBufferSize = %lu [bytes]\r\n", __func__, __LINE__, *pu32PCMBufferSize);
	}

	pu8PCMBuffer = (uint8_t *)pvPortMalloc(*pu32PCMBufferSize);
	if(pu8PCMBuffer == NULL){
		FLAC__stream_decoder_finish(s_pstFlacDecoder);
		FLAC__stream_decoder_delete(s_pstFlacDecoder);
		s_pstFlacDecoder = NULL;
		return NULL;
	}


	pst->enSample = s_u32SampleRate;
	pst->enBitsWidth = s_u32BitPerSample;
	memset(s_u8PCMBuffer, 0, sizeof(s_u8PCMBuffer));
	return pu8PCMBuffer;
}

DefALLOCATE_ITCM uint32_t ReadFlacFile(uint8_t pu8PCMBuffer[], uint32_t u32PCMBufferSize){
	uint32_t u32ByteCnt = 0;
	uint32_t u32ByteOfPCM = s_u32BitPerSample / 8;

	memset(pu8PCMBuffer, 0, u32PCMBufferSize);
	
	while(u32ByteCnt < u32PCMBufferSize){
		if(s_u32SampleCnt != 0){
			if(u32ByteOfPCM == 3)
			{
				uint32_t u32Pos = 0;
				uint32_t u32Cnt = 0;
				uint32_t u32 = s_u32SampleCnt * s_u32Ch * u32ByteOfPCM;
				/** おのれ24bit PCM */
				while(u32Cnt < u32){
					pu8PCMBuffer[u32ByteCnt++] = s_u8PCMBuffer[u32Pos];
					pu8PCMBuffer[u32ByteCnt++] = s_u8PCMBuffer[u32Pos + 1];
					pu8PCMBuffer[u32ByteCnt++] = s_u8PCMBuffer[u32Pos + 2];
					u32Pos += 4;
					u32Cnt += 3;
				}
			}else{
				memcpy(&pu8PCMBuffer[u32ByteCnt], s_u8PCMBuffer, s_u32SampleCnt * s_u32Ch * u32ByteOfPCM);
				u32ByteCnt += s_u32SampleCnt * s_u32Ch * u32ByteOfPCM;
			}
			s_u32SampleCnt = 0;
		}

		if(FLAC__stream_decoder_process_single(s_pstFlacDecoder) == false){
			break;
		}

		if(FLAC__stream_decoder_get_state(s_pstFlacDecoder) == FLAC__STREAM_DECODER_END_OF_STREAM){
			break;
		}else if(FLAC__stream_decoder_get_state(s_pstFlacDecoder) == FLAC__STREAM_DECODER_ABORTED){
			break;
		}else{
			/* NOP */
		}
		if(s_u32SampleCnt != 0){
			uint32_t u32Tmp;
			
			/** pu8PCMBufferにのこり何倍と書ける? */
			if(u32PCMBufferSize > u32ByteCnt){
				uint32_t u32CopyBytes;
				u32Tmp = u32PCMBufferSize - u32ByteCnt;
				if(u32Tmp >= s_u32SampleCnt * s_u32Ch * u32ByteOfPCM){
					u32CopyBytes = s_u32SampleCnt * s_u32Ch * u32ByteOfPCM;
				}else{
					u32CopyBytes = u32Tmp;
				}
				if(u32ByteOfPCM == 3)
				{
					uint32_t u32Pos = 0;
					uint32_t u32Cnt = 0;
					/** おのれ24bit PCM */
					while(u32Cnt < u32CopyBytes){
						pu8PCMBuffer[u32ByteCnt++] = s_u8PCMBuffer[u32Pos];
						pu8PCMBuffer[u32ByteCnt++] = s_u8PCMBuffer[u32Pos + 1];
						pu8PCMBuffer[u32ByteCnt++] = s_u8PCMBuffer[u32Pos + 2];
						u32Pos += 4;
						u32Cnt += 3;
					}
				}
				else
				{
					memcpy(&pu8PCMBuffer[u32ByteCnt], s_u8PCMBuffer, u32CopyBytes);
					u32ByteCnt += u32CopyBytes;
				}
				

				s_u32SampleCnt -= u32CopyBytes / (s_u32Ch * u32ByteOfPCM);

				/** pu8PCMBufferに書き込めないやつはs_u8PCMBufferに残しておく */
				if(s_u32SampleCnt !=0){
					if(u32ByteOfPCM == 3)
					{
						u32CopyBytes *= 4;
						u32CopyBytes /= 3;
						memcpy(s_u8PCMBuffer, &s_u8PCMBuffer[u32CopyBytes], s_u32SampleCnt * s_u32Ch * 4);
					}
					else
					{
						memcpy(s_u8PCMBuffer, &s_u8PCMBuffer[u32CopyBytes], s_u32SampleCnt * s_u32Ch * u32ByteOfPCM);
					}
				}
			}
		}
	}
	return u32ByteCnt;
}

void CloseFlacFile(void){
	FLAC__stream_decoder_delete(s_pstFlacDecoder);
	s_pstFlacDecoder = NULL;
}

static FLAC__StreamEncoder *s_pstFlacEncoder = NULL;
static FLAC__StreamMetadata *metadata[2];
static FLAC__StreamMetadata_VorbisComment_Entry entry;
static FLAC__int32 *s_pFlac32PCMBuffer = NULL;
static uint32_t u32SizeOfFlac32PCMBuffer = 0;

void ProgressCallBack(const FLAC__StreamEncoder *encoder, FLAC__uint64 bytes_written, FLAC__uint64 samples_written, unsigned frames_written, unsigned total_frames_estimate, void *client_data)
{
	/* NOP*/
}
_Bool OpenRecFlacFile(const TCHAR szFilePath[], const stCodecCondition_t *pst){
	_Bool bret = true;
	
	s_pstFlacEncoder = FLAC__stream_encoder_new();
	if(s_pstFlacEncoder == NULL)
	{
		bret = false;
	}
	
	if(bret != false){
		s_u32SampleRate = (uint32_t)pst->enSample;
		s_u32Ch = (uint32_t)pst->nChannels;
		s_u32BitPerSample = (uint32_t)pst->enBitsWidth;
	}
	/** Set Encoder Condition */
	if(bret != false){
		bret = FLAC__stream_encoder_set_verify(s_pstFlacEncoder, false);
	}
	if(bret != false){
		bret = FLAC__stream_encoder_set_compression_level(s_pstFlacEncoder, 1);
	}
	if(bret != false){
		bret = FLAC__stream_encoder_set_channels(s_pstFlacEncoder, s_u32Ch);
	}
	if(bret != false){
		bret = FLAC__stream_encoder_set_bits_per_sample(s_pstFlacEncoder, s_u32BitPerSample);
	}
	if(bret != false){
		bret = FLAC__stream_encoder_set_sample_rate(s_pstFlacEncoder, s_u32SampleRate);
	}

	
	/** Create Meta Data */
	if(bret != false){
		metadata[0] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
		if(metadata[0] == NULL){
			bret = false;
		}
	}

	if(bret != false){
		metadata[1] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_PADDING);
		if(metadata[1] == NULL){
			bret = false;
		}
	}
	/* there are many tag (vorbiscomment) functions but these are convenient for this particular use: */

	if(bret != false){
		bret = FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "ARTIST", "Some Artist");
	}
	if(bret != false){
		bret = FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, /*copy=*/false);
	}
	if(bret != false){
		bret = FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "YEAR", "1984");
	}
	if(bret != false){
		bret = FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, /*copy=*/false);
	}
	if(bret != false){
		metadata[1]->length = 1234; /* set the padding length */
		bret = FLAC__stream_encoder_set_metadata(s_pstFlacEncoder, metadata, 2);
	}

	if(bret != false){
		FLAC__StreamEncoderInitStatus init_status = FLAC__stream_encoder_init_file(s_pstFlacEncoder, szFilePath, (FLAC__StreamEncoderProgressCallback)ProgressCallBack, NULL);
		if(init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK) {
			bret = false;
		}
	}

	if(bret != false){
		if(pst->enBitsWidth == enWordWidth24bits){
			u32SizeOfFlac32PCMBuffer = pst->nChannels * pst->enSample * 4;
		}else{
			u32SizeOfFlac32PCMBuffer = pst->nChannels * pst->enSample * pst->enBitsWidth / 8;
		}

		s_pFlac32PCMBuffer = (FLAC__int32*)pvPortMalloc(pst->nChannels * pst->enSample * pst->enBitsWidth / 8);
		if(s_pFlac32PCMBuffer == NULL){
			bret = false;
		}
	}
	if(bret == false){

		if(metadata[0] != NULL){
			FLAC__metadata_object_delete(metadata[0]);
			metadata[0] = NULL;
		}
		if(metadata[1] != NULL){
			FLAC__metadata_object_delete(metadata[1]);
			metadata[1] = NULL;
		}
		if(s_pstFlacEncoder != NULL){
			FLAC__stream_encoder_finish(s_pstFlacEncoder);
			FLAC__stream_encoder_delete(s_pstFlacEncoder);
			s_pstFlacEncoder = NULL;
		}
		if(s_pFlac32PCMBuffer != NULL){
			vPortFree(s_pFlac32PCMBuffer);
		}
	}
	
	return bret;
}

DefALLOCATE_ITCM __attribute__(( optimize("-O3") )) _Bool WriteRecFlacFile(const stCodecCondition_t *pst, const uint8_t pu8PCMBuffer[], uint32_t u32PCMBufferSize){
	_Bool bret = false;
	
	uint32_t samples = u32PCMBufferSize / (s_u32Ch * s_u32BitPerSample /8);

	if(s_u32BitPerSample == 16){
		for(uint32_t i = 0; i < samples*s_u32Ch; i++) {
			s_pFlac32PCMBuffer[i] = (FLAC__int32)(((FLAC__int16)(FLAC__int8)pu8PCMBuffer[2*i+1] << 8) | (FLAC__int16)pu8PCMBuffer[2*i]);
		}
	}else if(s_u32BitPerSample == 24){
		for(uint32_t i = 0; i < samples*s_u32Ch; i++) {
			uint32_t u32 = pu8PCMBuffer[3*i+2];
			FLAC__int32 flacint32;
			u32 <<= 8;
			u32 |= pu8PCMBuffer[3*i+1];
			u32 <<= 8;
			u32 |= pu8PCMBuffer[3*i];
			u32 <<= 8;

			flacint32 = (FLAC__int32)u32;
			flacint32 /= 256;
			s_pFlac32PCMBuffer[i] = flacint32;
		}
	}else if(s_u32BitPerSample == 32){
		memcpy(s_pFlac32PCMBuffer, pu8PCMBuffer, u32PCMBufferSize);
	}else{
		return false;
	}
	bret = FLAC__stream_encoder_process_interleaved(s_pstFlacEncoder, s_pFlac32PCMBuffer, samples);
	if(bret == false){
		FLAC__StreamEncoderState encState = FLAC__stream_encoder_get_state(s_pstFlacEncoder);
		FLAC__StreamDecoderState decState = FLAC__stream_encoder_get_verify_decoder_state(s_pstFlacEncoder);
		mimic_printf("[%s (%d)] encState = %d\r\n", __FUNCTION__, __LINE__, encState);
		mimic_printf("[%s (%d)] decState = %d\r\n", __FUNCTION__, __LINE__, decState);
	}
	return bret;
}
void CloseRecFlacFile(void){
	if(metadata[0] != NULL){
		FLAC__metadata_object_delete(metadata[0]);
		metadata[0] = NULL;
	}
	if(metadata[1] != NULL){
		FLAC__metadata_object_delete(metadata[1]);
		metadata[1] = NULL;
	}
	if(s_pstFlacEncoder != NULL){
		FLAC__stream_encoder_delete(s_pstFlacEncoder);
		s_pstFlacEncoder = NULL;
	}
	if(s_pFlac32PCMBuffer != NULL){
		vPortFree(s_pFlac32PCMBuffer);
	}

	return;
}
