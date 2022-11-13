/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include <dev_disk.h>
#include <dev_rtc.h>
#include <ff.h>

typedef enum { SHELL, CLOCK, OBJ_KIND_NUM } OBJ_KIND;
EXPORT ID ObjID[OBJ_KIND_NUM];
EXPORT void shell_tsk(INT stacd, void *exinf);
EXPORT DATE_TIM dt;

EXPORT INT usermain( void )
{
T_CTSK t_ctsk;
LOCAL VB buf[32];
SZ  asize;

	if( rdDrvEntry( ) < E_OK )				// RAMディスクドライバの登録(サービス関数)
		goto ERROR;
	if( rtcDrvEntry( ) < E_OK )				// RTCドライバの登録処理(サービス関数)
		goto ERROR;
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

	t_ctsk.tskatr = TA_HLNG | TA_DSNAME;			// タスクの属性を設定
	t_ctsk.stksz = 2048;					// タスクのスタックサイズを設定
	t_ctsk.itskpri = 5;					// タスクの優先度を設定（任意）
	t_ctsk.task =  shell_tsk;				// タスクの起動番地を設定
	strcpy( t_ctsk.dsname, "shell" );			// タスクのデバッガサポート名を設定
	if( (ObjID[SHELL] = tk_cre_tsk( &t_ctsk )) <= E_OK )	// shellタスクの生成
		goto ERROR;
	if( tk_sta_tsk( ObjID[SHELL], tk_get_tid( ) ) < E_OK )	// shellタスクの起動
		goto ERROR;
	tk_slp_tsk( TMO_FEVR );					// 起床待ちとなり、shellタスクに制御を渡す
	tk_chg_pri( TSK_SELF, TK_MAX_TSKPRI );			// 自タスクの優先度を最低に変更
	
	tk_del_tsk( ObjID[SHELL] );				// shellタスクを削除
	tk_cls_dev( ObjID[CLOCK] , 0 );				// RTCドライバをクローズ

ERROR:
	return 0;
}

typedef void FUNC(INT);
EXPORT FUNC dir, cd, rd, wt, mkdir, rmdir;

EXPORT VB path[32], buf[128], ldn[4];
EXPORT BYTE work[FF_MAX_SS];
EXPORT VB * const argv[] = { &buf[64], &buf[96] };
EXPORT VB * const cmd[] = { "dir", "cd", "rd", "wt", "mkdir", "rmdir", "rm", "end" };
EXPORT void (* const func[])(INT) = { dir, cd, rd, wt, mkdir, rmdir, rmdir };
EXPORT FATFS fs;
EXPORT INT len;

EXPORT void shell_tsk(INT stacd, void *exinf)
{
INT argc, i;

	ldn[0] = '0' + RAMDISK;    ldn[1] = ':';
	if( FR_OK != f_mkfs( ldn, 0, work, sizeof(work) ) )
		goto ERROR;
	if( FR_OK != f_mount( &fs, ldn, 1 ) )
		goto ERROR;

	while( 1 )  {
		tm_printf( "%d:%s>", RAMDISK , path );
		tm_getline( buf );
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
		if( path[0] != '\0' )  {
			for( i = strlen( path )-1 ; i && path[i] != '/' ; i-- )
				;
			path[i] = '\0';
		}
	}
	else  {
		if( path[0] != '\0' )  {
			strcpy( buf, path );
			strcat( buf, "/" );
			strcat( buf, argv[1] );
		}
		else
			strcpy( buf, argv[1] );
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