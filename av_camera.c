#include<unistd.h>
#include<libavcodec/avcodec.h>
#include<libavdevice/avdevice.h>
#include<libavutil/channel_layout.h>
#include<libavutil/common.h>
#include<libavutil/frame.h>
#include<libavutil/samplefmt.h>
#include<libavutil/opt.h>
#include<libavutil/imgutils.h>
#include<libavutil/parseutils.h>
#include<libavutil/mem.h>
#include<libswscale/swscale.h>
#include<libavformat/avformat.h>


int flush_encoder(AVFormatContext *fmtCtx, AVCodecContext *codecCtx, int vStreamIndex){
    int      ret=0;
    AVPacket enc_pkt;
    enc_pkt.data = NULL;
    enc_pkt.size = 0;
    av_init_packet(&enc_pkt);

    if (!(codecCtx->codec->capabilities & AV_CODEC_CAP_DELAY))
        return 0;

    printf("Flushing stream #%u encoder\n",vStreamIndex);
    if(avcodec_send_frame(codecCtx,0)>=0){
        while(avcodec_receive_packet(codecCtx,&enc_pkt)>=0){
            printf("success encoder 1 frame.\n");

            // parpare packet for muxing
            enc_pkt.stream_index = vStreamIndex;
            av_packet_rescale_ts(&enc_pkt,codecCtx->time_base,
                                 fmtCtx->streams[ vStreamIndex ]->time_base);
            ret = av_interleaved_write_frame(fmtCtx, &enc_pkt);
            if(ret<0){
                break;
            }
        }
    }

    return ret;
}

int main()
{
    int ret = 0;
    //注册设备
    avdevice_register_all();
    //为相应的相关结构体分配内存
    AVFormatContext *inFmtCtx = avformat_alloc_context();
    AVCodecContext *inCodecCtx = NULL;
    AVCodec *inCodec = NULL;
    AVPacket *inPkt = av_packet_alloc();
    AVFrame *yuvFrame = av_frame_alloc();
    AVFrame *srcFrame = av_frame_alloc();

    AVFormatContext *outFmtCtx = avformat_alloc_context();
    AVOutputFormat *outFmt = NULL;
    AVCodecContext *outCodecCtx = NULL;
    AVCodec *outCodec = NULL;
    AVStream *outVStream = NULL;


    AVPacket *outPkt = av_packet_alloc();

    struct SwsContext *img_ctx = NULL;

    int inVideoStreamIndex = -1;
    do{
        //
        AVInputFormat *inFmt = av_find_input_format("v4l2");
        if(avformat_open_input(&inFmtCtx, "/dev/video0",NULL, NULL) < 0){
            printf("Cannot open camera.\n");
            return -1;
        }

        if(avformat_find_stream_info(inFmtCtx, NULL) < 0){
            printf("Cannot find any stream in file!\n");
            return -1;
        }

        for(size_t i = 0; i < inFmtCtx->nb_streams; i++){
            if(inFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
                inVideoStreamIndex = i;
                break;
            }
        }

        if(inVideoStreamIndex == -1){
            printf("Cannot find video stream in file.\n");
            return -1;
        }
        //AVCodecParameters 定义了一个已编码的流的相应属性
        AVCodecParameters *inVideoCodecPara = inFmtCtx->streams[inVideoStreamIndex]->codecpar;
        if(!(inCodec = avcodec_find_decoder(inVideoCodecPara->codec_id))){
            printf("Cannot find valid video decoder.\n");
            return -1;
        }
        
        if(!(inCodecCtx = avcodec_alloc_context3(inCodec))){
            printf("Cannot alloc valid video decoder.\n");
            return -1;
        }
        //利用相应的CodecPara的相应字段填充相应的编码器CodecContext内容
        if(avcodec_parameters_to_context(inCodecCtx, inVideoCodecPara) < 0){
            printf("Cannot initialize parameters.\n");
            return -1;
        }

        if(avcodec_open2(inCodecCtx, inCodec, NULL) < 0){
            printf("Cannot open codec.\n");
            return -1;
        }

        img_ctx = sws_getContext(inCodecCtx->width,   //输入图像的宽度
                                 inCodecCtx->height,  //输入图像的高度
                                 inCodecCtx->pix_fmt, //输入图像的像素格式
                                 inCodecCtx->width,   //输出图像的宽度
                                 inCodecCtx->height,  //输出图像的高度
                                 AV_PIX_FMT_YUV420P,  //输出图像的像素格式
                                 SWS_BICUBIC,         //缩放算法的选择，一般只有输入与输出图像的宽和高不同时才会有效
                                 NULL,                //输入图像的滤波器信息，若不需要传NULL
                                 NULL,                //输出图像的滤波器信息，若不需要传NULL
                                 NULL);               //特定缩放算法所需要的参数，默认为NULL
        
        int numBytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P,
                                                inCodecCtx->width,
                                                inCodecCtx->height,1);
        uint8_t *out_buffer = (unsigned char *)av_malloc(numBytes * sizeof(unsigned char));

        ret = av_image_fill_arrays(yuvFrame->data,
                                   yuvFrame->linesize,
                                   out_buffer,
                                   AV_PIX_FMT_YUV420P,
                                   inCodecCtx->width,
                                   inCodecCtx->height,
                                   1);
        if(ret < 0){
            printf("Fill arrays failed!\n");
            return -1;
        }

        const char * outfile = "result.h264";
        if(avformat_alloc_output_context2(&outFmtCtx,NULL,NULL,outfile) < 0){
            printf("Cannot alloc output file context.\n");
            return -1;
        }
        outFmt = outFmtCtx->oformat;
        
        if(avio_open(&outFmtCtx->pb, outfile, AVIO_FLAG_READ_WRITE) < 0){
            printf("out put file open failed!\n");
            return -1;
        }

        outVStream = avformat_new_stream(outFmtCtx, outCodec);
        if(outVStream == NULL){
            printf("create new video stream failed!\n");
            return -1;
        }
        outVStream->time_base.den = 30;
        outVStream->time_base.num = 1;
        
        AVCodecParameters *outCodecPara = outFmtCtx->streams[outVStream->index]->codecpar;
        outCodecPara->codec_type = AVMEDIA_TYPE_VIDEO;
        outCodecPara->codec_id = outFmt->video_codec;
        outCodecPara->width = 480;
        outCodecPara->height = 360;
        outCodecPara->bit_rate = 110000;

        outCodec = avcodec_find_encoder(outFmt->video_codec);
        if(outCodec == NULL){
            printf("Cannot find any encoder.\n");
            return -1;
        }

        outCodecCtx = avcodec_alloc_context3(outCodec);
        avcodec_parameters_to_context(outCodecCtx, outCodecPara);
        if(outCodecCtx == NULL){
            printf("Cannot alloc output codec content.\n");
            return -1;
        }

        outCodecCtx->codec_id = outFmt->video_codec;
        outCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
        outCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
        outCodecCtx->width = inCodecCtx->width;
        outCodecCtx->height = inCodecCtx->height;
        outCodecCtx->time_base.num = 1;
        outCodecCtx->time_base.den = 30;
        outCodecCtx->bit_rate = 110000;
        //定义了两个关键帧之间的帧数
        outCodecCtx->gop_size = 10;

        //如果编码器不同，需要额外设置一些参数，这样编码器才能正确编码
        //如果编码器是h264，那么需要设置qmin、qmax以及qcompress才能正确编码
        if(outCodecCtx->codec_id == AV_CODEC_ID_H264){
            outCodecCtx->qmin = 10;
            outCodecCtx->qmax = 51;
            outCodecCtx->qcompress = (float) 0.6;
        //如果编码器时采用MPEG-2格式来进行编码压缩的，则设置最大b帧数量
        }else if(outCodecCtx->codec_id == AV_CODEC_ID_MPEG2VIDEO){
            outCodecCtx->max_b_frames = 2;
        //如果编码器采用MPEG-1格式来进行编码压缩，则设置mb_decision
        }else if(outCodecCtx->codec_id == AV_CODEC_ID_MPEG1VIDEO){
            outCodecCtx->mb_decision = 2;
        }

        //根据编码器参数打开相应的编码器
        if(avcodec_open2(outCodecCtx,outCodec,NULL) < 0){
            printf("Open encoder failed!\n");
            return -1;
        }


        yuvFrame->format = outCodecCtx->pix_fmt;
        yuvFrame->width = outCodecCtx->width;
        yuvFrame->height = outCodecCtx->height;

        ret = avformat_write_header(outFmtCtx,NULL);

        int count = 0;
        while(av_read_frame(inFmtCtx, inPkt) >= 0 && count < 50){
            if(inPkt->stream_index == inVideoStreamIndex){
                if(avcodec_send_packet(inCodecCtx,inPkt) >= 0){
                    while((ret = avcodec_receive_frame(inCodecCtx,srcFrame)) >= 0){
                        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
                            return -1;
                        }else if(ret < 0){
                            fprintf(stderr, "Error during decoding\n");
                            exit(1);
                        }
                        sws_scale(img_ctx,
                                  (const uint8_t * const *) srcFrame->data,
                                  srcFrame->linesize,
                                  0,
                                  inCodecCtx->height,
                                  yuvFrame->data,
                                  yuvFrame->linesize);
                        yuvFrame->pts = srcFrame->pts;

                        if(avcodec_send_frame(outCodecCtx,yuvFrame) >= 0){
                            if(avcodec_receive_packet(outCodecCtx,outPkt) >= 0){
                                printf("encode one frame.\n");
                                ++count;
                                outPkt->stream_index = outVStream->index;
                                av_packet_rescale_ts(outPkt, outCodecCtx->time_base,outVStream->time_base);
                                outPkt->pos = -1;
                                av_interleaved_write_frame(outFmtCtx,outPkt);
                                av_packet_unref(outPkt);
                            }
                        }
                        usleep(1000*24);
                    }
                }
                av_packet_unref(inPkt);
            }
        }
        ret = flush_encoder(outFmtCtx,outCodecCtx,outVStream->index);
        if(ret<0){
            printf("flushing encoder failed.\n");
            return -1;
        }

        av_write_trailer(outFmtCtx);
    }while(0);
    av_packet_free(&inPkt);
    avcodec_free_context(&inCodecCtx);
    avcodec_close(inCodecCtx);
    avformat_close_input(&inFmtCtx);
    av_frame_free(&srcFrame);
    av_frame_free(&yuvFrame);

    av_packet_free(&outPkt);
    avcodec_free_context(&outCodecCtx);
    avcodec_close(outCodecCtx);
    avformat_close_input(&outFmtCtx);

    return 0;
} 