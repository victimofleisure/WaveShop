// this file was generated by lameExpHdr; don't edit it

typedef lame_global_flags * (*plame_init)(void);
typedef int (*plame_set_num_samples)(lame_global_flags *, unsigned long);
typedef int (*plame_set_in_samplerate)(lame_global_flags *, int);
typedef int (*plame_set_num_channels)(lame_global_flags *, int);
typedef int (*plame_set_quality)(lame_global_flags *, int);
typedef int (*plame_set_brate)(lame_global_flags *, int);
typedef int (*plame_set_VBR)(lame_global_flags *, vbr_mode);
typedef int (*plame_set_VBR_q)(lame_global_flags *, int);
typedef int (*plame_set_VBR_mean_bitrate_kbps)(lame_global_flags *, int);
typedef int (*plame_get_frameNum)(const lame_global_flags *);
typedef int (*plame_get_totalframes)(const lame_global_flags *);
typedef int (*plame_init_params)(lame_global_flags *);
typedef int (*plame_encode_buffer) (lame_global_flags* gfp, const short int buffer_l [], const short int buffer_r [], const int nsamples, unsigned char* mp3buf, const int mp3buf_size ); 
typedef int (*plame_encode_buffer_interleaved)(lame_global_flags* gfp, short int pcm[], int num_samples, unsigned char* mp3buf, int mp3buf_size ); 
typedef int (*plame_encode_buffer_ieee_double)(lame_t gfp,const double pcm_l [], const double pcm_r [], const int nsamples,unsigned char * mp3buf,const int mp3buf_size);
typedef int (*plame_encode_buffer_interleaved_ieee_double)(lame_t gfp,const double pcm[], const int nsamples,unsigned char * mp3buf,const int mp3buf_size);
typedef int (*plame_encode_flush)(lame_global_flags * gfp, unsigned char* mp3buf, int size); 
typedef int  (*plame_close) (lame_global_flags *);
typedef void (*pid3tag_add_v2)   (lame_t gfp);
typedef void (*pid3tag_set_title)(lame_t gfp, const char* title);
typedef void (*pid3tag_set_artist)(lame_t gfp, const char* artist);
typedef void (*pid3tag_set_album)(lame_t gfp, const char* album);
typedef void (*pid3tag_set_year)(lame_t gfp, const char* year);
typedef void (*pid3tag_set_comment)(lame_t gfp, const char* comment);
typedef int (*pid3tag_set_track)(lame_t gfp, const char* track);
typedef int (*pid3tag_set_genre)(lame_t gfp, const char* genre);
