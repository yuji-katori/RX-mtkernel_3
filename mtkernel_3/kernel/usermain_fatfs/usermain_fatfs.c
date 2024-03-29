/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include <dev_disk.h>
#include <ff.h>
#if FF_FS_NORTC == 1
#include <dev_rtc.h>

typedef enum { SHELL, CLOCK, OBJ_KIND_NUM } OBJ_KIND;
EXPORT DATE_TIM dt;
#elif FF_FS_NORTC == 2
EXPORT INT y, m, d, h, n, s;
void DATETIMtoTRON(void)
{
D  ll_tim, x;
INT i, z;
SYSTIM tim;
	ll_tim = 1000LL * ( s + 60 * ( n + 60L * ( h + ( d - 1 ) * 24 ) ) );
	if( y >= 2000 )  {				// 2000年以降か？
		ll_tim += 946684800000;			// 30年分のカウント値を加算
		i = 2000;				// 2000年を開始年とする
	}
	else
		i = 1970;				// 1970念を開始年とする
	for(  ;  ; i++ )  {
		if( i % 4 )				// 4で割り切れないか？
			// 11 --> 31day, 10 --> 30day, 01 --> 29day, 00 --> 28day
			// Dec Nov Oct Sep Aug Jul Jun May Apr Mar Feb Jan (Dec not used)
			// 11  10  11  10  11  11  10  11  10  11  00  11
			x = 31536000000, z = 0xEEFBB3;	// うるう年ではない
		else if( i % 100 )			// 100で割り切れないか？
			// 11  10  11  10  11  11  10  11  10  11  01  11
			x = 31622400000, z = 0xEEFBB7;	// うるう年
		else if( i % 400 )			// 400で割り切れないか？
			// 11  10  11  10  11  11  10  11  10  11  00  11
			x = 31536000000, z = 0xEEFBB3;	// うるう年ではない
		else					// 400で割り切る
			// 11  10  11  10  11  11  10  11  10  11  01  11
			x = 31622400000, z = 0xEEFBB7;	// うるう年
		if( i == y )				// 今年か？
			break;
		ll_tim += x;				// １年分の秒数を加算
	}
	for( i=1 ; i!=m ; i++, z>>=2 )  {
		if( z & 2 )				// 31日か30日か？
			if( z & 1 )			// 31日か？
				x = 2678400000;		// 31日
			else
				x = 2592000000;		// 30日
		else
			if( z & 1 )			// 29日か28日か？
				x = 2505600000;		// 29日
			else
				x = 2419200000;		// 28日
		ll_tim += x;				// １ケ月分の秒数を加算
	}
	tim.hi = ll_tim >> 32;				// 上位32ビットを設定
	tim.lo = ll_tim;				// 下位32ビットを設定
	tk_set_utc( &tim );				// システム時刻を設定
}
typedef enum { SHELL, OBJ_KIND_NUM } OBJ_KIND;
#endif
EXPORT ID ObjID[OBJ_KIND_NUM];
EXPORT void shell_tsk(INT stacd, void *exinf);
EXPORT void getline(VB *buf);

EXPORT INT usermain( void )
{
T_CTSK t_ctsk;
#if FF_FS_NORTC == 1
#if defined(AP_RX63N) || defined(AP_RX65N) || defined(AP_RX72N)
LOCAL VB buf[32];
SZ  asize;
#endif

	if( rtcDrvEntry( ) < E_OK )				// RTCドライバの登録処理(サービス関数)
		goto ERROR;
#if defined(AP_RX63N) || defined(AP_RX65N) || defined(AP_RX72N)
	if( ( ObjID[CLOCK] = tk_opn_dev( RTC_DEVNM, TD_UPDATE ) ) < E_OK )
		goto ERROR;					// RTCドライバをオープンし、初期化
	tm_putstring("Input now date and time.\n"
		     "Year:Month:Day:Week:Hour:Minute:Second\n"
		     "Week is 0 -> Sunday ... 6 -> Saturday\n"
		     "Ex. 2000:1:1:6:12:34:56\n\n");
	tm_getline( buf );
	sscanf( buf, "%ld:%ld:%ld:%ld:%ld:%ld:%ld", &dt.d_year, &dt.d_month, &dt.d_day, &dt.d_wday, &dt.d_hour, &dt.d_min, &dt.d_sec );
	tm_putstring("\n");
	dt.d_year -= 1900;
	if( tk_swri_dev( ObjID[CLOCK], DN_CKDATETIME, &dt, sizeof(dt), &asize ) < E_OK || asize != sizeof(dt) )
		goto ERROR;
#endif
#elif  FF_FS_NORTC == 2
LOCAL VB buf[32];
	tm_putstring("Input now date and time.\n"
		     "Year:Month:Day:Hour:Minute:Second\n"
		     "Ex. 2000:1:1:12:34:56\n\n");
	tm_getline( buf );
	sscanf( buf, "%ld:%ld:%ld:%ld:%ld:%ld", &y, &m, &d, &h, &n, &s );
	tm_putstring("\n");
	DATETIMtoTRON( );					// 年月日時分秒をUTC時刻に変換
#endif

#if defined(AP_RX63N) || defined(AP_RX72N) || defined(EK_RX72N)
	if( rdDrvEntry( ) < E_OK )				// RAMディスクドライバの登録(サービス関数)
		goto ERROR;
#endif

#if defined(AP_RX65N) || defined(AP_RX72N) || defined(EK_RX72N)	\
 || defined(TB_RX65N) || defined(TB_RX66N) || defined(TB_RX231)
	if( sdDrvEntry( ) < E_OK )				// SDカードドライバの登録(サービス関数)
		goto ERROR;
#endif

#if defined(AP_RX63N) || defined(AP_RX65N) || defined(AP_RX72N)	\
 || defined(TB_RX65N) || defined(TB_RX66N) || defined(EK_RX72N)
	if( usbDrvEntry( ) < E_OK )				// USBドライバの登録(サービス関数)
		goto ERROR;
#endif

	t_ctsk.tskatr = TA_HLNG | TA_DSNAME;			// タスクの属性を設定
	t_ctsk.stksz = 2048;					// タスクのスタックサイズを設定
	t_ctsk.itskpri = 10;					// タスクの優先度を設定（任意）
	t_ctsk.task =  shell_tsk;				// タスクの起動番地を設定
	strcpy( t_ctsk.dsname, "shell" );			// タスクのデバッガサポート名を設定
	if( (ObjID[SHELL] = tk_cre_tsk( &t_ctsk )) <= E_OK )	// shellタスクの生成
		goto ERROR;
	if( tk_sta_tsk( ObjID[SHELL], tk_get_tid( ) ) < E_OK )	// shellタスクの起動
		goto ERROR;
	tk_slp_tsk( TMO_FEVR );					// 起床待ちとなり、shellタスクに制御を渡す
	tk_chg_pri( TSK_SELF, TK_MAX_TSKPRI );			// 自タスクの優先度を最低に変更
	
	tk_del_tsk( ObjID[SHELL] );				// shellタスクを削除
#if FF_FS_NORTC == 1
	tk_cls_dev( ObjID[CLOCK] , 0 );				// RTCドライバをクローズ
#endif

ERROR:
	return 0;
}

typedef void FUNC(INT);
EXPORT FUNC cpd, dir, cd, rd, wt, mkdir, rmdir;

EXPORT VB path[32], buf[128], ldn[3] = "0:", ldnum[DEV_TYPE_CNT];
EXPORT VB ldname[][4]= { RAM_DISK_DEVNM, SD_CARD_DEVNM, USB_MSC_DEVNM };
EXPORT BYTE work[FF_MAX_SS];
EXPORT VB * const argv[] = { &buf[64], &buf[96] };
EXPORT VB * const cmd[] = { "dir", "cd", "rd", "wt", "mkdir", "rmdir", "rm", "end" };
EXPORT void (* const func[])(INT) = { dir, cd, rd, wt, mkdir, rmdir, rmdir };
EXPORT FATFS fs;
EXPORT INT len;

EXPORT void shell_tsk(INT stacd, void *exinf)
{
INT argc, i, j;
T_LDEV t_ldev;

	for( i=0 ; tk_lst_dev( &t_ldev, i, 1 ) >= E_OK  ; i++ )
		for( j=0 ; j<DEV_TYPE_CNT ; j++ )
			if( strcmp( t_ldev.devnm, ldname[j] ) == 0 )  {
				ldnum[j] = '0' + j;
				if( ! j && FR_OK != f_mkfs( ldn, 0, work, sizeof(work) ) )
					goto ERROR;
				break;
			}
	for( i=0 ; i<DEV_TYPE_CNT ; i++ )
		if( ldnum[i] )
			break;
	if( i == DEV_TYPE_CNT )
		goto ERROR;
	ldn[0] = ldnum[i];
START:

#if defined(AP_RX65N) || defined(AP_RX72N) || defined(EK_RX72N)	\
 || defined(TB_RX65N) || defined(TB_RX66N) || defined(TB_RX231)
	if( ldn[0] == '1' && sdWaitInsertEvent( TMO_POL ) == E_TMOUT )  {
		tm_putstring("Please insert SD card.\n");
		sdWaitInsertEvent( TMO_FEVR );
	}
#endif

#if defined(AP_RX63N) || defined(AP_RX65N) || defined(AP_RX72N)	\
 || defined(TB_RX65N) || defined(TB_RX66N) || defined(EK_RX72N)
	if( ldn[0] == '2' && usbWaitAttachEvent( TMO_POL ) == E_TMOUT )  {
		tm_putstring("Please attach USB memory.\n");
		usbWaitAttachEvent( TMO_FEVR );
	}
#endif
	if( FR_OK != f_mount( &fs, ldn, 1 ) )
		goto ERROR;
	strcpy( path, ldn );
	while( 1 )  {
		tm_printf( "%s>", path );
		getline( buf );
		if( strlen( buf ) == 2 && buf[1] == ':' )  {
			f_mount( 0, ldn, 0 );
			ldn[0] = buf[0];
			goto START;
		}
		argc = sscanf( buf, "%s%s%d", argv[0], argv[1], &len );
		for( i=0 ; i<sizeof(cmd)/sizeof(cmd[0]) ; i++ )
			if( ! strcmp( argv[0], cmd[i] ) )
				break;
		if( i == sizeof(cmd)/sizeof(cmd[0]) - 1 )
			break;
		if( i < sizeof(cmd)/sizeof(cmd[0]) )
			func[i]( argc );
	}
	f_mount( 0, ldn, 0 );
ERROR:
	tk_wup_tsk( stacd );
	tk_ext_tsk( );
}

void dir(INT argc)
{
DIR dp;
FILINFO fno;

	if( argc != 1 || FR_OK != f_opendir( &dp, path ) )
		return;
	while( 1 )  {
		if( FR_OK != f_readdir( &dp, &fno ) || fno.fname[0] == '\0' )
			break;
		tm_printf("%-13s%s %10d %4d:%.02d:%.02d %.02d:%.02d:%.02d\n", fno.fname,
		fno.fattrib & AM_DIR ? "\b\b\b\b DIR" : "",
		fno.fsize, (fno.fdate>>9)+1980, (fno.fdate>>5)&0xF, fno.fdate&0xF,
		(fno.ftime>>11)&0xF, (fno.ftime>>5)&0x3F, (fno.ftime&0x1F)<<1 );
	}
	f_closedir( &dp );
}

void cd(INT argc)
{
DIR dp;
INT i;

	if( argc != 2 )
		return;
	if( ! strcmp( argv[1], ".." ) )  {
		if( path[2] != '\0' )  {
			for( i = strlen( path )-1 ; i!=2 && path[i] != '/' ; i-- )
				;
			path[i] = '\0';
		}
	}
	else  {
		strcpy( buf, path );
		if( path[2] != '\0' )
			strcat( buf, "/" );
		strcat( buf, argv[1] );
		if( FR_OK == f_opendir( &dp, buf ) )  {
			strcpy( path, buf );
			f_closedir( &dp );
		}
	}
}

void rd(INT argc)
{
FIL  fil;
UINT br;

	if( argc != 2 )
		return;
	strcpy( buf, path );
	strcat( buf, "/" );
	strcat( buf, argv[1] );
	if( FR_OK != f_open( &fil, buf, FA_OPEN_EXISTING | FA_READ) )
		return;
	while( 1 )  {
		if( FR_OK != f_read( &fil, buf, sizeof(buf)-1, &br ) )
			break;
		buf[br] = '\0';
		tm_putstring( buf );
		if( br < sizeof(buf)-1 )  {
			tm_putstring( "\n" );
			break;
		}
	}
	f_close( &fil );
}

void wt(INT argc)
{
FIL  fil;
UINT bw;
INT  i, j;

	if( argc != 3 )
		return;
	strcpy( buf, path );
	strcat( buf, "/" );
	strcat( buf, argv[1] );
	if( FR_OK != f_open( &fil, buf, FA_CREATE_NEW | FA_WRITE) )
		return;
	for( i=0x20, j=0 ; i<0x7F ; i++, j++ )  {
		buf[j] = i;
		if( ( i & 0x1F ) == 0x1F )
			buf[++j] = '\n';
	}
	buf[j] = '\n';
	while( len > 0 )  {
		if( len >= 98 )  {
			if( FR_OK != f_write( &fil, buf, 98, &bw ) )
				break;
		}
		else  {
			if( FR_OK != f_write( &fil, buf, len, &bw ) )
				break;
		}
		len -= 98;
	}
	f_close( &fil );
}

void mkdir(INT argc)
{
	if( argc != 2 )
		return;
	strcpy( buf, path );
	strcat( buf, "/" );
	strcat( buf, argv[1] );
	f_mkdir( buf );
}

void rmdir(INT argc)
{
	if( argc != 2 )
		return;
	strcpy( buf, path );
	strcat( buf, "/" );
	strcat( buf, argv[1] );
	f_unlink( buf );
}

void getline(VB *buf)
{
IMPORT void tm_rcv_dat(VB* buf, INT size);
IMPORT void tm_snd_dat(const VB* buf, INT size);
	while( 1 )  {
		tm_rcv_dat( buf, 1 );
		tm_snd_dat( buf, 1 );
		if( *buf == '\r' )  {
			*buf = '\n';
			tm_snd_dat( buf, 1 );
			*buf = '\0';
			return;
		}
		else
			buf++;
	}
}