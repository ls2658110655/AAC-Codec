
/*
 *Remove noise:https://blog.csdn.net/venice0708/article/details/80783870
 */

#include "codec.h"
#include "ui_codec.h"

codec::codec(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::codec)
{
    ui->setupUi(this);

#if 0
    openAACFile();
#else
    decFile();
#endif
}

codec::~codec()
{
    delete ui;
}

#define INPUT_FILE_NAME ("audio_r48000_FMT_S16_c2.pcm")
//输出文件前缀
#define OUTPUT_FILE_NAME_PREFIX	("audio")
//输出文件后缀
#define OUTPUT_FILE_NAME_SUFFIX	("aac")
//输出文件后缀
//#define OUTPUT_FILE_NAME_SUFFIX	("mp3")
//输出文件比特率 该值越大 音频质量越好 音质损失越小
#define OUTPUT_FILE_BIT_RATE	(64000)

int codec::writeLog(FILE *pFile,const char *fmt,...)
{
    va_list arg;
    int done;
    va_start (arg,fmt);
    //done = vfprintf (stdout, fmt, arg);

    time_t time_log = time(NULL);
    struct tm* tm_log = localtime(&time_log);
    fprintf(pFile, "%04d-%02d-%02d %02d:%02d:%02d\t", tm_log->tm_year + 1900,
            tm_log->tm_mon + 1, tm_log->tm_mday, tm_log->tm_hour, tm_log->tm_min, tm_log->tm_sec);

    done = vfprintf (pFile, fmt, arg);
    va_end (arg);

    fflush(pFile);
    return done;

}

void codec::AddADTS(int pktLen)
{
    //int m_PcmSampleRate,m_PcmChannel;
    AVCodecContext *aacctx;

    char m_pOutData[1024 * 10];
    int profile = 1; // AAC LC
    int freqIdx = 0xb; // 44.1KHz
    int chanCfg = aacctx->channels; //m_PcmChannel; // CPE

    if (aacctx->bit_rate == 96000)
    {
        freqIdx = 0x00;
    }
    else if(aacctx->bit_rate == 88200)
    {
        freqIdx = 0x01;
    }
    else if (aacctx->bit_rate == 64000)
    {
        freqIdx = 0x02;
    }
    else if (aacctx->bit_rate == 48000)
    {
        freqIdx = 0x03;
    }
    else if (aacctx->bit_rate == 44100)
    {
        freqIdx = 0x04;
    }
    else if (aacctx->bit_rate == 32000)
    {
        freqIdx = 0x05;
    }
    else if (aacctx->bit_rate == 24000)
    {
        freqIdx = 0x06;
    }
    else if (aacctx->bit_rate == 22050)
    {
        freqIdx = 0x07;
    }
    else if (aacctx->bit_rate == 16000)
    {
        freqIdx = 0x08;
    }
    else if (aacctx->bit_rate == 12000)
    {
        freqIdx = 0x09;
    }
    else if (aacctx->bit_rate == 11025)
    {
        freqIdx = 0x0a;
    }
    else if (aacctx->bit_rate == 8000)
    {
        freqIdx = 0x0b;
    }
    else if (aacctx->bit_rate == 7350)
    {
        freqIdx = 0xc;
    }
    // fill in ADTS data
    m_pOutData[0] = 0xFF;
    m_pOutData[1] = 0xF1;
    m_pOutData[2] = ((profile) << 6) + (freqIdx << 2) + (chanCfg >> 2);
    m_pOutData[3] = (((chanCfg & 3) << 6) + (pktLen >> 11));
    m_pOutData[4] = ((pktLen & 0x7FF) >> 3);
    m_pOutData[5] = (((pktLen & 7) << 5) + 0x1F);
    m_pOutData[6] = 0xFC;
}

int codec::flush_encoder(AVFormatContext *fctx,unsigned int astindex)
{
    FILE* pFile = fopen("AACEncoder.log", "at+");

    int ret,got_afr;
    AVPacket ec_apkt;
    if (!(fctx->streams[astindex]->codec->codec->capabilities & AV_CODEC_CAP_DELAY))
        return 0;
    while (1) {
        ec_apkt.data = nullptr;
        ec_apkt.size = 0;
        av_init_packet(&ec_apkt);
        ret = avcodec_encode_audio2 (fctx->streams[astindex]->codec, &ec_apkt,
            nullptr, &got_afr);
        if (ret < 0)
            break;
        if (!got_afr){
            ret = 0;
            break;
        }

        av_frame_free(nullptr);

        printf("Flush Encoder: Succeed to encode 1 aac frame!\taSize:%5d\n",ec_apkt.size);
        writeLog(pFile, "%s %d %f\n", "Flush Encoder: Succeed to encode 1 aac frame!\taSize:%5d!",ec_apkt.size);

//        AddADTS(ec_apkt.size + 7);//未解决咔咔声
        // Mux encoded aac frame
        ret = av_write_frame(fctx, &ec_apkt);
        if (ret < 0){
            cout << "Write 1 aac frame!" << endl;
            writeLog(pFile, "%s %d %f\n", "Write 1 aac frame!");
            break;
        }
//        av_pkt_unref(&ec_apkt);
    }
    return ret;
}

/*
int codec::openAACFile()
{
    FILE* pFile = fopen("AACEncoder.log", "at+");
    writeLog(pFile, "%s %d %f\n", "AAC Encoder start running", 10, 55.55);

    AVFormatContext* aacfctx = nullptr;
    AVOutputFormat* fmt = nullptr;
    AVStream* aacSt = nullptr;
    AVCodecContext* aacctx = nullptr;
    AVCodec* aacc;
    AVFrame* aacfr = nullptr;
    AVPacket aacpkt;
    SwrContext *swr_ctx = nullptr;
    int aacfrnum = 1000;

    int aacSize = 0,got_aframe = 0,ret = 0;
    uint8_t* aacfrBuf;
    FILE *pcmfile = NULL;

    AVSampleFormat psfmt = AV_SAMPLE_FMT_S16;

    pcmfile = fopen("audio_48k_2.pcm","rb");
    const char *aacfile = "44k1_2.aac";

    av_register_all();
    avformat_network_init();

    avformat_alloc_output_context2(&aacfctx, NULL, NULL, aacfile);
    fmt = aacfctx->oformat;

    //Open output URL
    if (avio_open(&aacfctx->pb,aacfile, AVIO_FLAG_READ_WRITE) < 0){
        cout << "Failed to open output PCM file!" << endl;
        writeLog(pFile, "%s %d %f\n", "Failed to open output PCM file!");
        return -1;
    }

    aacSt = avformat_new_stream(aacfctx, 0);
    if (aacSt == NULL){
        return -1;
    }
    aacctx = aacSt->codec;
    aacctx->codec_id = fmt->audio_codec;
    aacctx->codec_type = AVMEDIA_TYPE_AUDIO;
    aacctx->sample_fmt = AV_SAMPLE_FMT_FLTP;
    aacctx->sample_rate = 44100;
    aacctx->channel_layout = AV_CH_LAYOUT_STEREO;//立体声 //av_get_default_channel_layout(aacctx->channels);
    aacctx->channels = av_get_channel_layout_nb_channels(aacctx->channel_layout); //2;
    aacctx->bit_rate = 128000; //96000; //64000；
    aacctx->frame_size = 1024;
    //Add Item
//    aacctx->profile = FF_PROFILE_AAC_LOW;
    // Allow the use of the experimental AAC encoder.
    aacctx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

    //Show some information
    av_dump_format(aacfctx, 0, aacfile, 1);

    aacc = avcodec_find_encoder(aacctx->codec_id);
    if (!aacc) {
        cout << "Could not find AAC encoder!" << endl;
        writeLog(pFile, "%s %d %f\n", "Could not find AAC encoder!");
        return -1;
    }
    if (avcodec_open2(aacctx, aacc,nullptr) < 0){
        cout << "Failed to open aac encoder!" << endl;
        writeLog(pFile, "%s %d %f\n", "Failed to open aac encoder!");
        return -1;
    }

    aacfr = av_frame_alloc();
    aacfr->nb_samples= aacctx->frame_size;
    aacfr->format= aacctx->sample_fmt;
    aacfr->channels = 2;

    //格式转换：pcm(s16)转aac(fltp)
    //asfmt = aacc->sample_fmts;
    swr_ctx = swr_alloc_set_opts(nullptr, aacctx->channel_layout, aacctx->sample_fmt,aacctx->sample_rate,
                                 aacctx->channel_layout, psfmt, aacctx->sample_rate, 0, nullptr);
    if (swr_ctx == nullptr)
    {
        fprintf(stderr, "Could not allocate resample context!\n");
        writeLog(pFile, "%s %d %f\n", "Could not allocate resample context!");
        return -1;
    }
    if (swr_init(swr_ctx) < 0)
    {
        fprintf(stderr, "Failed to initialize the resampling context!\n");
        writeLog(pFile, "%s %d %f\n", "Failed to initialize the resampling context!");
        return -1;
    }
    // Calloc
    uint8_t **cvtData; //存储转换后的数据，再编码AAC
    cvtData = (uint8_t**)calloc(aacctx->channels,sizeof(*cvtData));
    av_samples_alloc(cvtData, nullptr,aacctx->channels, aacctx->frame_size,aacctx->sample_fmt, 0);

    aacSize = av_samples_get_buffer_size(NULL, aacctx->channels,aacctx->frame_size,aacctx->sample_fmt, 1);
    aacfrBuf = (uint8_t *)av_malloc(aacSize);
    ret = avcodec_fill_audio_frame(aacfr, aacctx->channels, aacctx->sample_fmt,(const uint8_t*)aacfrBuf, aacSize, 1);
    if (ret < 0)
    {
        cout << "Failed to avcodec_fill_audio_frame!" << endl;
        writeLog(pFile, "%s %d %f\n", "Failed to avcodec_fill_audio_frame!");
        return 0;
    }

    //Write Header
    avformat_write_header(aacfctx,NULL);

    av_new_packet(&aacpkt,aacSize);

    for (int i = 0; i < aacfrnum; i++){
    //while(true){
        //Read PCM
        int readSize = fread(aacfrBuf, 1, 4096, pcmfile);
        cout << "readSize:" << readSize << endl;
        if (readSize <= 0){
            cout << "Failed to read raw PCM data!236" << endl;
            writeLog(pFile, "%s %d %f\n", "Failed to read raw PCM data!");
            return -1;
        }else if(feof(pcmfile)){
            break;
        }

        if (swr_convert(swr_ctx, cvtData,aacctx->frame_size,
                        (const uint8_t**)aacfr->data, aacctx->frame_size) <= 0)
        {
            cout << "Audio swr_convert failed!" << endl;
            writeLog(pFile, "%s %d %f\n", "Audio swr_convert failed!");
            return -1;
        }
        //将转换后的数据复制给pFrame
  //      aacfr->data[0] = aacfrBuf;  //PCM Data
        int len = aacctx->frame_size * av_get_bytes_per_sample(aacctx->sample_fmt);

        memcpy(aacfr->data[0], cvtData[0], len);
        memcpy(aacfr->data[1], cvtData[1], len);

        aacfr->pts = i*100;
        got_aframe = 0;

        //Encode
        ret = avcodec_send_frame(aacctx,aacfr);
        while(ret >= 0){
            ret = avcodec_receive_pkt(aacctx, &aacpkt);
            if (ret == 0){
                printf("Succeed to encode 1 frame! \tsize:%5d\n",aacpkt.size);
                writeLog(pFile, "%s %d %f\n", "Succeed to encode 1 frame! \tsize:%5d",aacpkt.size);
                AddADTS(aacpkt.size + 7);//未解决咔咔声
                aacpkt.stream_index = aacSt->index;
                ret = av_write_frame(aacfctx, &aacpkt);
                av_pkt_unref(&aacpkt);
            }
        }
    }

    //Flush Encoder
    ret = flush_encoder(aacfctx,0);
    if (ret < 0) {
        cout << "Flushing encoder failed!" << endl;
        writeLog(pFile, "%s %d %f\n", "Flushing encoder failed!");
        return -1;
    }

    //Write Trailer
    av_write_trailer(aacfctx);

    //Clean
    if (aacSt){
        avcodec_close(aacSt->codec);
        av_frame_free(&aacfr);
        av_free(aacfrBuf);
    }
//    swr_free(&swr_ctx);
    avio_close(aacfctx->pb);
    avformat_free_context(aacfctx);
//    avcodec_free_context(&aacctx);

    av_freep(&cvtData[0]);
    free(cvtData);
    fclose(pcmfile);
//    DeleteFileA(pcmfile);

    writeLog(pFile, "%s %d %f\n", "AAC Encoder stop running!");
    fclose(pFile);//close logFile

    return 0;
}*/

int codec::openAACFile()
{
    FILE* pFile = fopen("AACEncoder.log", "at+");
    writeLog(pFile, "%s %d %f\n", "AAC Encoder start running", 10, 55.55);

    static char*fmtne[]=
    {
        "FMT_U8","FMT_S16","FMT_S32","FMT_FLT","FMT_DBL",
        "FMT_U8P","FMT_S16P","FMT_S32P","FMT_FLTP","FMT_DBLP"
    };

    //各种不同格式对应字节数
    static int mapSampleBytes[AV_SAMPLE_FMT_NB]
            ={1,2,4,4,8,1,2,4,4,8};

    //PCM原始数据格式
    uint64_t iLayout = AV_CH_LAYOUT_STEREO;
    int iChans = av_get_channel_layout_nb_channels(iLayout);
    AVSampleFormat eiSmeFmt = AV_SAMPLE_FMT_S16;
    int iSmeRate = 48000;
    //不同样本格式长度
    int iSampleBytes = mapSampleBytes[eiSmeFmt];

    //PCM需要重采样的格式 部分编码器不支持原始PCM的数据格式如MP3
    uint64_t oLayout = AV_CH_LAYOUT_STEREO;
    int oChans = av_get_channel_layout_nb_channels(oLayout);
    AVSampleFormat oSpeFmt ;
    int oSpeRate = 48000;

    if(strcmp(OUTPUT_FILE_NAME_SUFFIX,"aac") == 0)
    {
        oSpeFmt = AV_SAMPLE_FMT_FLTP;
    }
    else if(strcmp(OUTPUT_FILE_NAME_SUFFIX,"mp3") == 0)
    {
        //MP3不支持AV_SAMPLE_FMT_S16这种格式
        oSpeFmt = AV_SAMPLE_FMT_S16P;
    }
    else
    {

    }
    //编码样本长度
    int oSampleBytes = mapSampleBytes[oSpeFmt];

    //是否需要重采样
    bool bNeedResample = false;
    if(eiSmeFmt != oSpeFmt)
    {
        bNeedResample = true;
    }

    //是否平面存储结构
    bool bPlanner = false;
    if((oSpeFmt>=AV_SAMPLE_FMT_U8P) &&(oSpeFmt<=AV_SAMPLE_FMT_DBLP))
    {
        bPlanner = true;
    }

    //pcmfile = fopen("audio_48k_2.pcm","rb");
    //const char *aacfile = "44k1_2.aac";
    //打开输入文件
    FILE *ife = fopen("audio_r48000_FMT_S16_c2.pcm", "rb");
    if(ife == NULL)
    {

    }

    //打开输出文件
    char ofene[256]={0};
    sprintf(ofene,"%s_br%d_sr%d.%s",OUTPUT_FILE_NAME_PREFIX,OUTPUT_FILE_BIT_RATE,oSpeRate,OUTPUT_FILE_NAME_SUFFIX);
    FILE *ofe = fopen(ofene, "wb");

    //打开中间测试文件
    char szTempfene[256]={0};
    sprintf(szTempfene,"%s_sr%d_c1.pcm",OUTPUT_FILE_NAME_PREFIX,oSpeRate);
    FILE *pTempFile = fopen(szTempfene, "wb");

    int iret;

    ///////////编码器操作////////////////////////////////
    //注册所有编解码器
    av_register_all();

    //封装格式上下文 AVFormatContext中有AVInputFormat和AVOutputFormat
    //解复用时avformat_open_input()初始化AVInputFormat，复用时用户自己初始化AVOutputFormat
    AVFormatContext* afmtCtx;
    AVOutputFormat * fmt;

    //Method 2.
    avformat_alloc_output_context2(&afmtCtx, NULL, NULL, ofene);
    fmt = afmtCtx->oformat;

    //添加一个流
    AVStream  *audio_st = avformat_new_stream(afmtCtx, 0);
    if (audio_st==NULL)
    {
        return -1;
    }

    //添加一个输出路径
    if (avio_open(&afmtCtx->pb,ofene, AVIO_FLAG_READ_WRITE) < 0)
    {
        printf("Failed to open output file!\n");
        return -1;
    }

    //Show some information 日志信息
    av_dump_format(afmtCtx, 0, ofene, 1);

    //初始化编码器相关结构体 获取输出流中的编码上下文
    AVCodecContext* acCtx = audio_st->codec;

    acCtx->codec_id		  = fmt->audio_codec	;
    acCtx->codec_type	  = AVMEDIA_TYPE_AUDIO	;
    //立体声
    acCtx->channel_layout = oLayout	;
    acCtx->channels		  = oChans    ;
    //编码比特率 AAC支持多种比特率 一般比特率越高 视频质量越好 需要传输带宽越大
    acCtx->bit_rate		  = OUTPUT_FILE_BIT_RATE;
    acCtx->sample_rate    = oSpeRate;
    //PCM样本深度为AV_SAMPLE_FMT_S16 但不是所有格式的编码都支持这种格式
    acCtx->sample_fmt	  = oSpeFmt;

    //编码器
    AVCodec* ac = avcodec_find_encoder(acCtx->codec_id);
    if (!ac)
    {
        printf("Can not find encoder!\n");
        return -1;
    }

    //打开解码器 有可能失败 -22 错误，原因不同的编码格式支持的样本格式不一样
    //如封装AAC格式样本格式是AV_SAMPLE_FMT_FLT，打开就出错
    if ((iret = avcodec_open2(acCtx, ac,NULL)) < 0)
    {
        printf("Failed to open encoder :[%d]!\n",iret);
        return -1;
    }

    //重采用上下文
    SwrContext *pSwrCtx = NULL;

    //原始数据帧
    AVFrame* pRawfr  = NULL;
    //原始帧一Planer的大小 非平面分布的情况就是缓存总大小
    int iRawLineSize = 0;
    //原始帧缓存大小
    int iRawBuffSize = 0;
    //原始帧缓存
    uint8_t *pRawBuff= NULL;

    //重采样后数据帧
    AVFrame* pConvertframe = NULL;
    //重采样后一Planer的大小
    int iConvertLineSize  = 0;
    //重采样后缓存大小
    int iConvertBuffSize  = 0;
    //重采样后帧缓存
    uint8_t *pConvertBuff = NULL;

    //1帧数据样本数
    int ifrSamples = acCtx->frame_size;

    // 存储原始数据
    iRawLineSize = 0;
    iRawBuffSize  = av_samples_get_buffer_size(&iRawLineSize, iChans, ifrSamples, eiSmeFmt, 0);
    pRawBuff = (uint8_t *)av_malloc(iRawBuffSize);

    //原始数据保存在AVFrame结构体中
    pRawfr = av_frame_alloc();

    pRawfr->nb_samples	= ifrSamples;
    pRawfr->format		= eiSmeFmt;
    pRawfr->channels     = iChans;

    iret = avcodec_fill_audio_frame(pRawfr, iChans, eiSmeFmt, (const uint8_t*)pRawBuff, iRawBuffSize, 0);
    if(iret<0)
    {
        return -1;
    }

    if(bNeedResample)
    {
        pSwrCtx = swr_alloc_set_opts(NULL,oLayout, oSpeFmt, oSpeRate,
                                     iLayout,eiSmeFmt , iSmeRate,0, NULL);
        swr_init(pSwrCtx);

        // 存储转换后数据
        iConvertLineSize  = 0;
        iConvertBuffSize  = av_samples_get_buffer_size(&iConvertLineSize, oChans, ifrSamples, oSpeFmt, 0);
        pConvertBuff      = (uint8_t *)av_malloc(iConvertBuffSize);

        //转换后数据保存在AVFrame结构体中
        pConvertframe = av_frame_alloc();
        pConvertframe->nb_samples	= ifrSamples;
        pConvertframe->format		= oSpeFmt;
        pConvertframe->channels     = oChans;

        iret = avcodec_fill_audio_frame(pConvertframe, oChans, oSpeFmt, (const uint8_t*)pConvertBuff, iConvertBuffSize, 0);
        if(iret<0)
        {
            return -1;
        }
    }

    //编码以后的数据是AVPacket
    AVPacket pkt;
    if(!bNeedResample)
    {
        av_new_packet(&pkt,iRawBuffSize);
    }
    else
    {
        av_new_packet(&pkt,iConvertBuffSize);
    }

    //Write Header
    avformat_write_header(afmtCtx,NULL);

    //统计读取样本数
    long long lReadTotalSamples = 0;
    //每次读取样本数
    int iReadSamples;
    //统计所有的帧数
    int iFrameNum =0;
    //是否编码成功
    int got_frame =0;
    //临时
    AVFrame* pTempFrame=NULL;
    //读取数据 保存在pConvertframe->data
    int iRealRead = fread(pRawBuff, 1, iRawBuffSize, ife);
    while(iRealRead>0)
    {
        iReadSamples = iRealRead/(iSampleBytes*iChans);
        if(bNeedResample)
        {
            swr_convert(pSwrCtx, (uint8_t**)pConvertframe->data, ifrSamples ,(const uint8_t**)pRawfr->data, ifrSamples );

            if(bPlanner)
            {
                //只保存一个通道 因为保存多个通道测试工具 audacity看不了
                fwrite(pConvertframe->data[0],pConvertframe->linesize[0],1,pTempFile);
            }
            printf("Convert Frame :%d\n",++iFrameNum);
            pTempFrame = pConvertframe;
        }
        else
        {
            pTempFrame = pRawfr;
        }



        pTempFrame->pts = lReadTotalSamples;
        got_frame = 0;
        //Encode
        if(avcodec_encode_audio2(acCtx, &pkt,pTempFrame, &got_frame)<0)
        {
            printf("Failed to encode!\n");
            return -1;
        }
        if (got_frame==1)
        {
            printf("Succeed to encode 1 frame! \tsize:%5d\n",pkt.size);
            pkt.stream_index = audio_st->index;
            av_write_frame(afmtCtx, &pkt);
            av_free_packet(&pkt);
        }

        //统计样本数以转换前为准 转换前后样本数是一样的
        lReadTotalSamples += (iReadSamples);

        iRealRead = fread(pRawBuff, 1, iRawBuffSize, ife);
    }

    //刷新编码器
    if(flush_encoder(afmtCtx,0)<0)
    {
        printf("Flushing encoder failed\n");
        return -1;
    }

    fclose(ife);
    fclose(ofe);
    fclose(pTempFile);

    av_free(pRawBuff);

    if(bNeedResample)
    {
        av_free(pConvertBuff);
        swr_free(&pSwrCtx);
    }
    printf("Convert Success!!\n");
    //getchar();

    writeLog(pFile, "%s %d %f\n", "AAC Encoder stop running!");
    fclose(pFile);//close logFile

    return 0;
}

#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio
//Output PCM
#define OUTPUT_PCM 1
//Use SDL
#define USE_SDL 1

//Buffer:
//|-----------|-------------|
//chunk-------pos---len-----|
static  Uint8  *achunk;
static  Uint32  alen;
static  Uint8  *apos;

void  fill_audio(void *udata,Uint8 *astr,int len)
{
    //SDL 2.0
    SDL_memset(astr, 0, len);
    if(alen == 0)
        return;

    len = (len>alen?alen:len); //Mix as much data as possible

    SDL_MixAudio(astr,apos,len,SDL_MIX_MAXVOLUME);
    apos += len;
    alen -= len;
}

int codec::decFile()
{
    SDL_AudioSpec wanted_spec;

    FILE *oFe = NULL;

    oFe = fopen("opt.pcm", "wb");
    if(oFe == NULL)
    {
        printf("Can't to open output pcm stream!\n");
        return -1;
    }
    //注册解码器
    av_register_all();
    avformat_network_init();

    AVFormatContext	*afmtCtx = avformat_alloc_context();
    //打开封装格式
    if(avformat_open_input(&afmtCtx,"audio_br64000_sr48000.aac",NULL,NULL) != 0)
    {
        return -1;
    }
    //
    if(avformat_find_stream_info(afmtCtx,NULL) < 0)
    {
        printf("封装格式查找流失败.\n");
        return -1;
    }
    // Dump valid information onto standard error
    av_dump_format(afmtCtx, 0, "audio_br64000_sr48000.aac", false);

    // Find the first audio stream
    int aSt = -1;
    for(int i=0; i < afmtCtx->nb_streams; i++)
    {
        if(afmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            aSt=i;
            break;
        }
    }
    if(aSt == -1)
    {
        printf("Didn't find a audio stream!\n");
        return -1;
    }

    //Get a pointer to the codec context for the audio stream
    AVCodecContext *acCtx = afmtCtx->streams[aSt]->codec;

    //Find the decoder for the audio stream
    AVCodec *ac = avcodec_find_decoder(acCtx->codec_id);
    if(ac == NULL)
    {
        printf("Codec not found.\n");
        return -1;
    }

    // Open codec
    if(avcodec_open2(acCtx, ac,NULL)<0)
    {
        printf("Could not open codec!\n");
        return -1;
    }

    //对AAC编码文件来说，编码根据音频参数编码，解码根据音频参数重新构建声波，FFMPEG构建的
    //音频存储方式不一定支持播放，所以需要重采样样本 如AAC解码的样本格式是AV_SAMPLE_FMT_FLTP

    uint64_t iLayout = av_get_default_channel_layout(acCtx->channels);
    int      iChans = acCtx->channels;
    AVSampleFormat eiSmeFmt = acCtx->sample_fmt;
    int	     iSmeRate = acCtx->sample_rate;

    uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
    int ochannels = av_get_channel_layout_nb_channels(out_channel_layout);
    uint64_t oLayout = av_get_default_channel_layout(acCtx->channels);
    int out_nb_samples = acCtx->frame_size;
    int      oChans = acCtx->channels;
    AVSampleFormat oSpeFmt = AV_SAMPLE_FMT_FLTP;
    int	     oSpeRate = acCtx->sample_rate;
    //Out Buffer Size
    int obufsize = av_samples_get_buffer_size(NULL,ochannels, out_nb_samples,oSpeFmt, 1);
    uint8_t* obuf = (uint8_t *)av_malloc(MAX_AUDIO_FRAME_SIZE*2);

    SwrContext *pSwrCtx = swr_alloc_set_opts(NULL,oLayout, oSpeFmt, oSpeRate,
                                             iLayout,eiSmeFmt , iSmeRate,0, NULL);
    swr_init(pSwrCtx);

    //AVPacket读取原始解码前的数据
    AVPacket *pkt=(AVPacket *)malloc(sizeof(AVPacket));
    av_init_packet(pkt);

    //1帧数据样本数
    int ifrSamples = acCtx->frame_size;

    // 存储原始数据
    int iRawLineSize = 0;
    int iRawBuffSize  = av_samples_get_buffer_size(&iRawLineSize, iChans, ifrSamples, eiSmeFmt, 0);
    uint8_t *pRawBuff = (uint8_t *)av_malloc(iRawBuffSize);

    //原始数据保存在AVFrame结构体中
    AVFrame* pRawfr = av_frame_alloc();

    pRawfr->nb_samples	= ifrSamples;
    pRawfr->format		= eiSmeFmt;
    pRawfr->channels     = iChans;

    int iret = avcodec_fill_audio_frame(pRawfr, iChans, eiSmeFmt, (const uint8_t*)pRawBuff, iRawBuffSize, 0);
    if(iret<0)
    {
        printf("Failed to fill input aac frame!\n");
        return -1;
    }


    // 存储转换后数据
    int iConvertLineSize = 0;
    int iConvertBuffSize  = av_samples_get_buffer_size(&iConvertLineSize, oChans, ifrSamples, oSpeFmt, 0);
    uint8_t *pConvertBuff = (uint8_t *)av_malloc(iConvertBuffSize);

    //转换后数据保存在AVFrame结构体中
    AVFrame* pConvertframe = av_frame_alloc();
    pConvertframe->nb_samples	= ifrSamples;
    pConvertframe->format		= oSpeFmt;
    pConvertframe->channels     = oChans;

    iret = avcodec_fill_audio_frame(pConvertframe, oChans, oSpeFmt, (const uint8_t*)pConvertBuff, iConvertBuffSize, 0);
    if(iret<0)
    {
        printf("Failed to fill output pcm frame!\n");
        return -1;
    }
    int iGetPic;
    int iDecodeRet;
    int frNo = 0;

//    write_wav_header(16,oChans,oSpeFmt,oSpeRate,0,oFe);

#if USE_SDL
    //Init
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        printf( "Could not initialize SDL - %s\n", SDL_GetError());
        return -1;
    }
    //SDL_AudioSpec
    wanted_spec.freq = 44100; //out_sample_rate;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.channels = av_get_channel_layout_nb_channels(out_channel_layout); //ochannels;
    wanted_spec.silence = 0;
    wanted_spec.samples = acCtx->frame_size;//out_nb_samples;
    wanted_spec.callback = fill_audio;
    wanted_spec.userdata = acCtx;

    if (SDL_OpenAudio(&wanted_spec, NULL)<0){
        printf("can't open audio.\n");
        return -1;
    }
#endif

    while(av_read_frame(afmtCtx, pkt)>=0)
    {
        if(pkt->stream_index == aSt)
        {

            iDecodeRet = avcodec_decode_audio4(acCtx, pRawfr,&iGetPic, pkt);
            if (iDecodeRet < 0 )
            {
                printf("Error in decoding audio frame.\n");
                return -1;
            }
            if ( iGetPic > 0 )
            {
                printf("FrameNo:%5d\n",frNo);
                swr_convert(pSwrCtx, (uint8_t**)pConvertframe->data, ifrSamples ,(const uint8_t**)pRawfr->data, ifrSamples );

                fwrite(pConvertframe->data[0],pConvertframe->linesize[0],1,oFe);
                frNo++;
            }
#if USE_SDL
            while(alen>0)//Wait until finish
                SDL_Delay(1);

            //Set audio buffer (PCM data)
            achunk = (Uint8 *)obuf;
            //Audio buffer length
            alen = obufsize;
            apos = achunk;
#endif

        }
        av_free_packet(pkt);
    }

    av_free(pRawBuff);
    av_free(pConvertBuff);
    swr_free(&pSwrCtx);
#if USE_SDL
    SDL_CloseAudio();//Close SDL
    SDL_Quit();
#endif
    avcodec_close(acCtx);
    avformat_close_input(&afmtCtx);

    fclose(oFe);
    printf("Aac encode Success!!\n");
//    getchar();

    return 0;
}
