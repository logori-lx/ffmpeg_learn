


static AVFormatContext* open_dev(const char* device_name) {
	//注册所有设备
	avdevice_register_all();
	//获得格式
	const AVInputFormat* iformat = av_find_input_format("video4linux2");
 
	//打开设备
	AVFormatContext* fmt_ctx = NULL;
	//const char* device_name = "hw:0,0";//这个是音频
	//const char* device_name = "/dev/video0";
	AVDictionary* options = NULL;
 
	//视频就需要做options了
	av_dict_set(&options, "video_size", "640*480", 0);
	av_dict_set(&options, "framerate", "30", 0);
 
	int ret = avformat_open_input(&fmt_ctx, device_name, iformat, &options);
	
	//判断是否成功
	char errors[1024] = {0, };
	if(ret < 0) {
		av_strerror(ret, errors, 1024);
		std::cout << "Faile to open Device : " <<errors<<std::endl;
		return NULL;
	}
	std::cout<<"Success open Device"<<std::endl;
	return fmt_ctx;
}



#define __STDC_CONSTANT_MACROS
#include <string.h>
 
extern "C"
{
#include "libavutil/avutil.h"
#include "libavdevice/avdevice.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
#include "libavdevice/avdevice.h"
}
 
 
#define WIDTH  640
#define HEIGHT 480
 
 
static
AVFormatContext* open_dev(){
 
    int ret = 0;
    char errors[1024] = { 0, };
 
    //ctx
    AVFormatContext *fmt_ctx = NULL;
    AVDictionary *options = NULL;
    //[[video device]:[audio device]]
    const char *device_name = "/dev/video0";
    av_dict_set(&options, "video_size", "640x480",0);
 
    //get format
    AVInputFormat *iformat = av_find_input_format("video4linux2");
 
    //open device
    if ((ret = avformat_open_input(&fmt_ctx, device_name, iformat, &options)) < 0){
        av_strerror(ret, errors, 1024);
        fprintf(stderr, "Failed to open audio device, [%d]%s\n", ret, errors);
        return NULL;
    }
 
    return fmt_ctx;
}
 
void open_encodec(int width, int height, AVCodecContext ** enc_ctx)
{
    AVCodec * codec = avcodec_find_encoder_by_name("libx264");
    if(codec == NULL)
    {
        av_log(NULL, AV_LOG_INFO, "Codec libx264 not found!\n");
        return;
    }
 
 
    *enc_ctx = avcodec_alloc_context3(codec);
    if(enc_ctx == NULL)
    {
        av_log(NULL, AV_LOG_INFO, "alloc context3 failed!\n");
        return;
    }
    //sps pps
    (*enc_ctx)->profile = FF_PROFILE_H264_HIGH_444;
    (*enc_ctx)->level = 50;//level 5.0 version
 
    //resolution
    (*enc_ctx)->width = width;
    (*enc_ctx)->height = height;
    //GOP
    (*enc_ctx)->gop_size = 250;
    (*enc_ctx)->keyint_min = 25;
 
    (*enc_ctx)->max_b_frames = 3;
    (*enc_ctx)->has_b_frames = 1;
 
    (*enc_ctx)->refs = 3;
    (*enc_ctx)->pix_fmt = AV_PIX_FMT_YUV420P;
 
    (*enc_ctx)->bit_rate = 60000000;// 6000kbps
 
    (*enc_ctx)->time_base = (AVRational){1,30};
    (*enc_ctx)->framerate = (AVRational){30,1};
 
 
    int ret = avcodec_open2((*enc_ctx), codec, NULL);
    if(ret < 0)
    {
        av_log(NULL, AV_LOG_INFO, "avcodec_open2 failed!\n");
        return;
    }
}
 
AVFrame* create_frame(int width, int height)
{
    AVFrame *frame = av_frame_alloc();
    if(frame == NULL)
    {
        av_log(NULL, AV_LOG_INFO, "av_frame_alloc failed!\n");
        return NULL;
    }
    frame->width = width;
    frame->height = height;
    frame->format = AV_PIX_FMT_YUV420P;
 
    int ret = av_frame_get_buffer(frame, 32);
    if(ret < 0)
    {
        av_log(NULL, AV_LOG_INFO, "av_frame_get_buffer failed!\n");
        return NULL;
    }
    return frame;
}
 
 
static void encode(AVCodecContext *ctx, AVFrame *frame, AVPacket *newpkt, FILE * outfile)
{
#if 0
    int ret = avcodec_send_frame(ctx, frame);
    if(ret < 0)
    {
        av_log(NULL, AV_LOG_INFO, "avcodec_send_frame failed!\n");
        exit(-1);
    }
    while(ret >= 0)
    {
        ret = avcodec_receive_packet(ctx,newpkt);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            //av_log(NULL, AV_LOG_INFO, "avcodec_receive_packet no more data!\n");
            return;
        }
        else if(ret < 0)
        {
            av_log(NULL, AV_LOG_INFO, "avcodec_receive_packet failed!\n");
            exit(-1);
        }
        fwrite(newpkt->data, 1, newpkt->size, outfile);
        av_packet_unref(newpkt);
    }
 
#endif
}
 
void rec_video() {
 
    //context
    AVFormatContext *fmt_ctx = NULL;
    //设置log的级别
    av_log_set_level(AV_LOG_DEBUG);
 
    //注册所有的音频设备
    avdevice_register_all();
 
 
    //create file
    const char *out = "/home/lili/Videos/video.yuv";
    FILE *outfile = fopen(out, "wb+");
    if (!outfile)
    {daddr_t
        printf("Error, Failed to open file!\n");
        return;
    }
 
    AVCodecContext *codec_ctx = NULL;
    open_encodec(WIDTH, HEIGHT, &codec_ctx);
 
    //编码的输入数据
    AVFrame*  avframe = create_frame(WIDTH, HEIGHT);
    //创建编码后输出的数据
    AVPacket* newpkt = av_packet_alloc();
    if (!newpkt){
        printf("Error, av_packet_alloc!\n");
        return;
    }
    //打开设备
    fmt_ctx = open_dev();
    if (!fmt_ctx){
        printf("Error, Failed to open device!\n");
        return;
    }
    AVPacket pkt;
    int ret = -1, count = 1;
 
 
    const char *encode_file_path = "/home/lili/Videos/video.h264";
    FILE *encode_file = fopen(encode_file_path, "wb+");
    if (!encode_file)
    {
        printf("Error, Failed to open file!\n");
        return;
    }
 
    int base = 0;int got_output = 0;
    while((ret = av_read_frame(fmt_ctx, &pkt)) == 0 && count++ < 200)
    {
        av_log(NULL, AV_LOG_INFO, "packet size is %d\n", pkt.size);
#define FORMAT_YUYV
 
 
#ifdef FORMAT_YUV
        //ffplay -s 640x480 -pixel_format yuyv422 ~/Videos/video.yuv
       fwrite(pkt.data, 1, pkt.size, outfile);//yuyv422 format
       fflush(outfile);
#else
    /*
           Y U Y V Y U Y V
           Y U Y V Y U Y V
           Y U Y V Y U Y V
           Y U Y V Y U Y V
              - yuyv422 -
       Y Y Y Y Y Y      Y Y Y Y Y Y      Y Y Y Y Y Y      Y Y Y Y Y Y
       Y Y Y Y Y Y      Y Y Y Y Y Y      Y Y Y Y Y Y      Y Y Y Y Y Y
       Y Y Y Y Y Y      Y Y Y Y Y Y      Y Y Y Y Y Y      Y Y Y Y Y Y
       Y Y Y Y Y Y      Y Y Y Y Y Y      Y Y Y Y Y Y      Y Y Y Y Y Y
       U U U U U U      V V V V V V      U V U V U V      V U V U V U
       V V V V V V      U U U U U U      U V U V U V      V U V U V U
        - I420 -          - YV12 -         - NV12 -         - NV21 -
    */
        //ffplay -video_size 640x480 -pixel_format yuv420p ~/Videos/video.yuv
 
        int len = WIDTH * HEIGHT;
        for(int i = 0;i < len; i++){
            avframe->data[0][i] = pkt.data[i*2];//Y
        }
 
        //yuyv 序列为YU YV YU YV，一个yuv422帧的长度 width * height * 2 个字节
        //yuyv --- >yuv420p丢弃偶数行 u v
        int cnt = 0;
        for(int i = 0; i < HEIGHT; i += 2)
        {
            for(int j = 0; j < WIDTH/2; j++)
            {
                avframe->data[1][cnt] = pkt.data[i * WIDTH * 2 + j * 4 + 1];//Cb
                avframe->data[2][cnt++] = pkt.data[i * WIDTH * 2 + j * 4 + 3];//Cr
            }
        }
        fwrite(avframe->data[0], 1, WIDTH * HEIGHT, outfile);
        fwrite(avframe->data[1], 1, WIDTH * HEIGHT/4, outfile);
        fwrite(avframe->data[2], 1, WIDTH * HEIGHT/4, outfile);
        fflush(outfile);
        //important!!!
        avframe->pts = base++;
 
        //encode(codec_ctx, avframe, newpkt, encode_file);
 
 
        ret = avcodec_encode_video2(codec_ctx, newpkt, avframe, &got_output);
        if (ret < 0)
        {
            fprintf(stderr, "Error encoding frame\n");
            exit(1);
        }
 
        if (got_output)
        {
            printf("Write frame (size=%5d)\n", newpkt->size);
            fwrite(newpkt->data, 1, newpkt->size, encode_file);
            fflush(encode_file);
            av_packet_unref(newpkt);
        }
 
#endif
 
    }
 
    /* get the delayed frames */
    for (got_output = 1; got_output;)
    {
        fflush(stdout);
        ret = avcodec_encode_video2(codec_ctx, newpkt, NULL, &got_output);
        if (ret < 0) {
            fprintf(stderr, "Error encoding frame\n");
            exit(1);
        }
 
        if (got_output)
        {
            printf("Write frame (size=%5d)\n", newpkt->size);
            fwrite(newpkt->data, 1, newpkt->size, encode_file);
            av_packet_unref(newpkt);
        }
    }
 
 
    fclose(outfile);
    fclose(encode_file);
    av_log(NULL, AV_LOG_DEBUG, "finish!\n");
 
    return;
}
 
 
int main()
{
    rec_video();
    return 0;
}
