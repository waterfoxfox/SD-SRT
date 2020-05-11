//***************************************************************************//
//* 版权所有  www.mediapro.cc
//*
//* 内容摘要：SDSrt-AVCom内部传输模式对外DLL接口
//*	
//* 当前版本：V1.0		
//* 作    者：mediapro
//* 完成日期：2020-3-6
//**************************************************************************//

#ifndef _SD_SRT_AVCOM_SDK_H_
#define _SD_SRT_AVCOM_SDK_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#ifndef BUILDING_DLL
#define DLLIMPORT __declspec (dllexport)
#else /* Not BUILDING_DLL */
#define DLLIMPORT __declspec (dllimport)
#endif /* Not BUILDING_DLL */
#else
#define DLLIMPORT
#endif

#ifdef __APPLE__
#ifndef OBJC_BOOL_DEFINED
typedef int BOOL;
#endif 
#else
#ifndef BOOL
typedef int BOOL;
#endif
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

//最大允许的JITTER 缓存时间
#define MAX_SUPPORT_JITTER_MS		3000

//日志输出的级别
typedef enum LOG_OUTPUT_LEVEL
{
	LOG_OUTPUT_LEVEL_DEBUG = 1,
	LOG_OUTPUT_LEVEL_INFO,
	LOG_OUTPUT_LEVEL_WARNING,
	LOG_OUTPUT_LEVEL_ERROR,
	LOG_OUTPUT_LEVEL_ALARM,
	LOG_OUTPUT_LEVEL_FATAL,
	LOG_OUTPUT_LEVEL_NONE
} LOG_OUTPUT_LEVEL;

//2D FEC布局模式
typedef enum E_SRT_FEC_LAYOUT {
	//普通连续模式
	e_SRT_FEC_LAYOUT_EVEN = 0,
	//阶梯模式，可以降低一定码率波动（冗余包分散发送）
	e_SRT_FEC_LAYOUT_STAIR
}E_SRT_FEC_LAYOUT;


//FEC-ARQ配合模式
typedef enum E_SRT_FEC_ARQ {
	//只要丢包均会发起ARQ，不管FEC能否恢复
	e_SRT_FEC_ARQ_ALWAYS = 0,
	//仅在FEC失败时发起ARQ
	e_SRT_FEC_ARQ_ONREQ,
	//关闭ARQ
	e_SRT_FEC_ARQ_NEVER,
}E_SRT_FEC_ARQ;


//输出接收到的音视频数据 回调函数
//bComplete用来表示当前帧数据是否完整（无局部丢包）
//bPrevTotalFrameLost用来表示当前帧与上一次输出帧之间无整帧丢失的情况，即本帧序号与上一帧序号是否连续。
//通过以上两个标志，结合关键帧判定标志，外层可以很方便的实现丢帧冻结机制
typedef void (*CallBackFuncRecvVideoData)(void* pObj, int nLen, unsigned char *byBuf, unsigned int unPTS, BOOL bComplete, BOOL bPrevTotalFrameLost);

typedef void (*CallBackFuncRecvAudioData)(void* pObj, int nLen, unsigned char *byBuf, unsigned int unPTS);


//////////////////////////////////////////////////////////////////////////
// SrtAvCom接口

/***
* 环境初始化，系统只需调用一次，主要用于SRT环境以及日志模块的初始化
* @param:outputPath表示日志存放路径，支持相对路径和绝对路径，若目录不存在将自动创建
* @param:outputPath表示日志输出的级别，只有等于或者高于该级别的日志输出到文件，取值范围参考LOG_OUTPUT_LEVEL
* @return: 
*/
DLLIMPORT void  SDSrtAvCom_Enviroment_Init(const char* outputPath, int outputLevel);

DLLIMPORT void  SDSrtAvCom_Enviroment_Free();

/***
* 创建SrtAvCom
* @param unLogId: 日志ID，仅用于日志输出时的对象标识。
* @return: 返回模块指针，为NULL则失败
*/
DLLIMPORT void*  SDSrtAvCom_Create(UINT unLogId);

/***
* 销毁SrtAvCom，使用者应该做好与其他API之间的互斥保护
* @param pRtp_avcom: 模块指针
* @return:
*/
DLLIMPORT void  SDSrtAvCom_Delete(void* pRtp_avcom);


/***
* 开始工作
* @param strLocalIP: 本地IP地址
* @param shLocalPort: 本地收发端口（该端口用于音频，视频端口号在此基础上加1）
* @param strRemoteIP: 对方IP地址
* @param shRemotePort: 对方收发端口（该端口用于音频，视频端口号在此基础上加1）
* @param pfVideoRecvCallBack: 接收到对方发送的视频数据后的回调函数
* @param pfAudioRecvCallBack: 接收到对方发送的音频数据后的回调函数
* @param pObject: 调用上述两个回调函数时的附带透传形参，模块内部不会解析本参数仅做透传处理
* @return: TRUE FALSE
*/
DLLIMPORT BOOL  SDSrtAvCom_Start(
	void* pRtp_avcom,
	const char *strLocalIP, 
	USHORT shLocalPort, 
	const char *strRemoteIP, 
	USHORT shRemotePort, 
	CallBackFuncRecvVideoData pfVideoRecvCallBack,
	CallBackFuncRecvAudioData pfAudioRecvCallBack,
	void *pObject);


/***
* 停止SrtAvCom工作
* @param pRtp_avcom: 模块指针
* @return:
*/
DLLIMPORT void  SDSrtAvCom_Stop(void* pRtp_avcom);

/***
* 发送音频数据
* @param pRtp_avcom: 模块指针
* @param byBuf: 传入一帧音频裸码流，可以是ADTS，内部无拆包透传
* @param nLen: 数据长度
* @return: 
*/
DLLIMPORT BOOL  SDSrtAvCom_SendAudioData(void* pRtp_avcom, unsigned char *byBuf, int nLen);

/***
* 发送视频数据
* @param pRtp_avcom: 模块指针
* @param byBuf: 传入一帧带起始码的裸码流，内部自行拆分拼接
* @param nLen: 数据长度
* @return: 
*/
DLLIMPORT BOOL  SDSrtAvCom_SendVideoData(void* pRtp_avcom, unsigned char *byBuf, int nLen);


/***
* 设置基础传输参数，请在Start接口之前调用
* @param pRtp_avcom: 模块指针
* @param nRecvDelayMs: 接收缓存时间，建议4*RTT，单位ms。可在发送端或接收端设置，将取其中较大的值
* @param nMaxBitrateKbps：最大传输码率，建议2*VideoEncBitrate，单位kbps。需要在发送端设置，当设置为0时表示不受限
* @return:
*/
DLLIMPORT BOOL  SDSrtAvCom_SetBaseTransParams(void* pRtp_avcom, int nRecvDelayMs, int nMaxBitrateKbps);


/***
* 设置视频通道FEC传输参数，请在Start接口之前调用
* @param pRtp_avcom: 模块指针
* @param bEnable: 是否启用FEC，收发双方需保持一致
* @param nCols: FEC Group列数
* @param nRows: FEC Group行数
* @param eLayoutMode：2D FEC布局模式
* @param eArqMode：FEC-ARQ配合模式
* @return:
*/
DLLIMPORT BOOL  SDSrtAvCom_SetVideoFecParams(void* pRtp_avcom, BOOL bEnable, int nCols, int nRows, E_SRT_FEC_LAYOUT eLayoutMode, E_SRT_FEC_ARQ eArqMode);



/***
* 设置音频通道FEC传输参数，请在Start接口之前调用
* @param pRtp_avcom: 模块指针
* @param bEnable: 是否启用FEC，收发双方需保持一致
* @param nCols: FEC Group列数
* @param nRows: FEC Group行数
* @param eLayoutMode：2D FEC布局模式
* @param eArqMode：FEC-ARQ配合模式
* @return:
*/
DLLIMPORT BOOL  SDSrtAvCom_SetAudioFecParams(void* pRtp_avcom, BOOL bEnable, int nCols, int nRows, E_SRT_FEC_LAYOUT eLayoutMode, E_SRT_FEC_ARQ eArqMode);



/***
* 获取视频通道统计信息
* @param pRtp_avcom: 模块指针
* @param pfRttMs: RTT，单位毫秒
* @param pfUpLossRate: 上行丢包率.内部已经乘100
* @param pfDownLossRate: 下行丢包率.内部已经乘100
* @param pfEstimatedUpBitrate：上行带宽估算.Kbps
* @param pfUpBitrate：上行码率.Kbps
* @param pfDownBitrate：下行码率.Kbps
* @return:
*/
DLLIMPORT BOOL  SDSrtAvCom_GetVideoTransStatis(void* pRtp_avcom, double *pfRttMs, double *pfUpLossRate, double *pfDownLossRate, 
											double *pfEstimatedUpBitrate, double *pfUpBitrate, double *pfDownBitrate);


/***
* 获取音频通道统计信息
* @param pRtp_avcom: 模块指针
* @param pfRttMs: RTT，单位毫秒
* @param pfUpLossRate: 上行丢包率.内部已经乘100
* @param pfDownLossRate: 下行丢包率.内部已经乘100
* @param pfEstimatedUpBitrate：上行带宽估算.Kbps
* @param pfUpBitrate：上行码率.Kbps
* @param pfDownBitrate：下行码率.Kbps
* @return:
*/
DLLIMPORT BOOL  SDSrtAvCom_GetAudioTransStatis(void* pRtp_avcom, double *pfRttMs, double *pfUpLossRate, double *pfDownLossRate,
											double *pfEstimatedUpBitrate, double *pfUpBitrate, double *pfDownBitrate);
#ifdef __cplusplus
}
#endif

#endif // _SD_SRT_AVCOM_SDK_H_
