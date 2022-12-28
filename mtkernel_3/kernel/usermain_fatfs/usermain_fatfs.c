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

	if( rtcDrvEntry( ) < E_OK )				// RTC�h���C�o�̓o�^����(�T�[�r�X�֐�)
		goto ERROR;
	if( ( ObjID[CLOCK] = tk_opn_dev( RTC_DEVNM, TD_UPDATE ) ) < E_OK )
		goto ERROR;					// RTC�h���C�o���I�[�v�����A������
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

#if defined(AP_RX63N) || defined(AP_RX72N)
	if( rdDrvEntry( ) < E_OK )				// RAM�f�B�X�N�h���C�o�̓o�^(�T�[�r�X�֐�)
		goto ERROR;
#endif
#if defined(AP_RX65N) || defined(AP_RX72N)
	if( sdDrvEntry( ) < E_OK )				// SD�J�[�h�h���C�o�̓o�^(�T�[�r�X�֐�)
		goto ERROR;
#endif

	tk_dly_tsk( 1000 );
	t_ctsk.tskatr = TA_HLNG | TA_DSNAME;			// �^�X�N�̑�����ݒ�
	t_ctsk.stksz = 2048;					// �^�X�N�̃X�^�b�N�T�C�Y��ݒ�
	t_ctsk.itskpri = 10;					// �^�X�N�̗D��x��ݒ�i�C�Ӂj
	t_ctsk.task =  shell_tsk;				// �^�X�N�̋N���Ԓn��ݒ�
	strcpy( t_ctsk.dsname, "shell" );			// �^�X�N�̃f�o�b�K�T�|�[�g����ݒ�
	if( (ObjID[SHELL] = tk_cre_tsk( &t_ctsk )) <= E_OK )	// shell�^�X�N�̐���
		goto ERROR;
	if( tk_sta_tsk( ObjID[SHELL], tk_get_tid( ) ) < E_OK )	// shell�^�X�N�̋N��
		goto ERROR;
	tk_slp_tsk( TMO_FEVR );					// �N���҂��ƂȂ�Ashell�^�X�N�ɐ����n��
	tk_chg_pri( TSK_SELF, TK_MAX_TSKPRI );			// ���^�X�N�̗D��x���Œ�ɕύX
	
	tk_del_tsk( ObjID[SHELL] );				// shell�^�X�N���폜
	tk_cls_dev( ObjID[CLOCK] , 0 );				// RTC�h���C�o���N���[�Y

ERROR:
	return 0;
}

typedef void FUNC(INT);
EXPORT FUNC cpd, dir, cd, rd, wt, mkdir, rmdir;

EXPORT VB path[32], buf[128], ldn[3] = "0:", ldnum[DEV_TYPE_CNT];
EXPORT VB ldname[][4]= { RAM_DISK_DEVNM, SD_CARD_DEVNM, USB_HMSC_DEVNM };
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
	if( FR_OK != f_mount( &fs, ldn, 1 ) )
		goto ERROR;
	strcpy( path, ldn );
	while( 1 )  {
		tm_printf( "%s>", path );
		tm_getline( buf );
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