#ifndef CODEC_H
#define CODEC_H

#include <QMainWindow>
#include <windows.h>
#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <iostream>
#include <thread>
#include <memory>
#include <functional>
#include <conio.h>  //判断是否有键盘输入
#include <QMessageBox>
#include <QAudioDeviceInfo>
#include <QTextCodec>
#include <QByteArray>
#include <QString>
#include <QDir>
#include <QDebug>

extern"C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavfilter/avfilter.h>
//#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/channel_layout.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <SDL2/SDL.h>
#undef main
}

using namespace std;

#define OUTPUT_YUV420P 0
#define SFM_REFRESH_EVENT (SDL_USEREVENT + 1)
#define SFM_BREAK_EVENT (SDL_USEREVENT + 2)

#define H264 0

namespace Ui {
class codec;
}

class codec : public QMainWindow
{
    Q_OBJECT

public:
    explicit codec(QWidget *parent = 0);
    ~codec();

    //写日志
    int writeLog(FILE *pFile,const char *fmt,...);

    //刷新编码器
    int flush_encoder(AVFormatContext *fctx,unsigned int astindex);
    //写ADTS文件
    void AddADTS(int pktLen);
    //AAC编码器(用户使用的接口函数)
    int openAACFile();
    //AAC解码器(用户使用的接口函数)
    int decFile();

private:
    Ui::codec *ui;
};

#endif // CODEC_H
