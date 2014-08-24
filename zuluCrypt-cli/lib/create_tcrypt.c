
 /*
  *
  *  Copyright (c) 2013
  *  name : mhogo mchungu
  *  email: mhogomchungu@gmail.com
  *  This program is free software: you can redistribute it and/or modify
  *  it under the terms of the GNU General Public License as published by
  *  the Free Software Foundation, either version 2 of the License, or
  *  (at your option) any later version.
  *
  *  This program is distributed in the hope that it will be useful,
  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  *  GNU General Public License for more details.
  *
  *  You should have received a copy of the GNU General Public License
  *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
  */

#include "includes.h"
#include <sys/syscall.h>
#include <sys/syscall.h>
#include <libcryptsetup.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
 * this header is created at config time
 */
#include "truecrypt_support_1.h"

#if TRUECRYPT_CREATE

static int _create_file_system( const create_tcrypt_t * e )
{
	string_t m = StringVoid ;

	int r ;

	const char * device_mapper ;
	const char * mapper ;

	size_t len ;

	open_struct_t opts ;

	memset( &opts,'\0',sizeof( open_struct_t ) ) ;

	m = String( crypt_get_dir() ) ;
	len = StringLength( m )   ;

	StringAppend( m,"/zuluCrypt-" ) ;
	device_mapper = StringAppendInt( m,syscall( SYS_gettid ) ) ;
	mapper = device_mapper + len + 1 ;

	opts.volume_type = TCRYPT_NORMAL ;
	opts.device = e->device ;
	opts.mapper_name = mapper ;
	opts.m_opts = "rw" ;

	opts.key     = e->passphrase ;
	opts.key_len = e->passphrase_size ;

	if( e->keyfiles != NULL ){
		opts.tcrypt_keyfiles       = e->keyfiles ;
		opts.tcrypt_keyfiles_count = e->keyfiles_number ;
	}

	/*
	 * zuluCryptOpenTcrypt_1 is defined in open_tcrypt.c
	 */
	r = zuluCryptOpenTcrypt_1( &opts ) ;

	if( r == 0 ){
		/*
		 * zuluCryptCreateFileSystemInAVolume() is defined in create_volume.c
		 */
		if( zuluCryptCreateFileSystemInAVolume( e->fs,device_mapper ) == 0 ){

			r = 0 ;
		}else{
			r = 3 ;
		}
		/*
		 * zuluCryptCloseMapper() is defined in close_mapper.c
		 */
		zuluCryptCloseMapper( device_mapper ) ;
	}

	if( r == 0 && e->hidden_volume_size > 0 ){

		opts.volume_type = TCRYPT_HIDDEN ;

		opts.key     = e->passphrase_h ;
		opts.key_len = e->passphrase_h_size ;

		if( e->keyfiles_h != NULL ){
			opts.tcrypt_keyfiles       = e->keyfiles_h ;
			opts.tcrypt_keyfiles_count = e->keyfiles_h_number ;
		}

		r = zuluCryptOpenTcrypt_1( &opts ) ;

		if( r == 0 ){

			if( zuluCryptCreateFileSystemInAVolume( e->fs_h,device_mapper ) == 0 ){

				r = 0 ;
			}else{
				r = 3 ;
			}
		}
		/*
		 * zuluCryptCloseMapper() is defined in close_mapper.c
		 */
		zuluCryptCloseMapper( device_mapper ) ;
	}

	StringDelete( &m ) ;
	return r ;
}

#define TRUE   ( int )1
#define FALSE  ( int )0

static string_t _root_device( const char * device,const char ** sys_device )
{
	size_t e ;
	ssize_t r ;
	string_t st = String( device ) ;
	if( StringStartsWithAtLeastOne( st,"/dev/sd","/dev/hd",NULL ) ){
		/*
		 * this path will convert something like: "/dev/sdc12" to "/dev/sdc".
		 * basically,it removes digits from the end of the string to give the root device
		 * required by tcplay's system volume or fde volume
		 */
		*sys_device = StringRemoveDigits( st ) ;
	}else if( StringStartsWith( st,"/dev/mmc" ) ){
		/*
		 * device path will be something like "/dev/mmcblk0p2" and what we want to do
		 * is cut off the string from p to end iwth "/dev/mmcblk0"
		 */
		r = StringIndexOfChar( st,0,'p' ) ;
		if( r != -1 ){
			e = StringLength( st ) - ( size_t )r ;
			*sys_device = StringRemoveRight( st,e ) ;
		}else{
			*sys_device = StringContent( st ) ;
		}
	}else{
		*sys_device = StringContent( st ) ;
	}
	return st ;
}

int zuluCryptModifyTcryptHeader( const info_t * info )
{
	tc_api_task task ;
	int r = !TC_OK ;
	const char * sys_device = NULL ;
	string_t st = StringVoid ;

	if( info->device == NULL ){
		return r ;
	}
	if( tc_api_init( 0 ) == TC_OK ){
		task = tc_api_task_init( "modify" ) ;
		if( task != 0 ){
			if( StringsAreEqual( info->opt,"sys" ) ){
				tc_api_task_set( task,"dev",info->device ) ;
				st = _root_device( info->device,&sys_device ) ;
				tc_api_task_set( task,"sys",sys_device ) ;
			}else if( StringsAreEqual( info->opt,"fde" ) ){
				st = _root_device( info->device,&sys_device ) ;
				tc_api_task_set( task,"dev",sys_device ) ;
				tc_api_task_set( task,"fde",TRUE ) ;
			}else{
				tc_api_task_set( task,"dev",info->device ) ;
			}

			tc_api_task_set( task,"hidden_size_bytes",( u_int64_t )0 ) ;

			if( info->header_source && info->tmp_path ){
				/*
				* below line may look like one of the following two lines:
				* tc_api_task_set( task,"header_from_file","/home/ink/tc.headerbackup" ) ;
				* tc_api_task_set( task,"save_header_to_file","/home/ink/tc.headerbackup" ) ;
				*/
				tc_api_task_set( task,info->header_source,info->tmp_path ) ;
			}

			if( info->header_key_source && info->header_key ){
				/*
				* below line may look like one of the following two lines:
				* tc_api_task_set( task,"passphrase","xxx" ) ;
				* tc_api_task_set( task,"keyfiles","/home/ink/keyfile" ) ;
				*/
				tc_api_task_set( task,info->header_key_source,info->header_key ) ;
			}

			if( info->header_new_key_source && info->header_new_key ){
				/*
				* below line may look like one of the following two lines:
				* tc_api_task_set( task,"new_passphrase","xxx" ) ;
				* tc_api_task_set( task,"new_keyfiles","/home/ink/keyfile" ) ;
				*/
				tc_api_task_set( task,info->header_new_key_source,info->header_new_key ) ;
			}

			if( StringsAreEqual( info->rng,"/dev/urandom" ) ){
				tc_api_task_set( task,"weak_keys_and_salt",TRUE ) ;
			}else{
				tc_api_task_set( task,"weak_keys_and_salt",FALSE ) ;
			}

			r = tc_api_task_do( task ) ;
			tc_api_task_uninit( task ) ;
		}
		tc_api_uninit() ;
	}
	StringDelete( &st ) ;
	return r ;
}

int zuluCryptCreateTCryptVolume( const create_tcrypt_t * e )
{
	tc_api_task task ;
	int r = !TC_OK ;
	size_t i ;
	size_t k ;
	const char ** z ;

	if( tc_api_init( 0 ) == TC_OK ){
		task = tc_api_task_init( "create" ) ;
		if( task != 0 ){

			tc_api_task_set( task,"dev",e->device ) ;
			tc_api_task_set( task,"secure_erase",FALSE ) ;
			tc_api_task_set( task,"prf_algo","RIPEMD160" ) ;
			tc_api_task_set( task,"cipher_chain","AES-256-XTS" ) ;

			if( StringsAreNotEqual( e->passphrase,"" ) ){
				tc_api_task_set( task,"passphrase",e->passphrase ) ;
			}

			z = e->keyfiles ;
			k = e->keyfiles_number ;

			for( i = 0 ; i < k ; i++ ){
				tc_api_task_set( task,"keyfiles",*( z + i ) ) ;
			}

			if( e->weak_keys_and_salt ){
				tc_api_task_set( task,"weak_keys_and_salt",TRUE ) ;
			}else{
				tc_api_task_set( task,"weak_keys_and_salt",FALSE ) ;
			}

			if( e->hidden_volume_size > 0 ){

				tc_api_task_set( task,"hidden_size_bytes",e->hidden_volume_size ) ;
				tc_api_task_set( task,"h_prf_algo","RIPEMD160" ) ;
				tc_api_task_set( task,"h_cipher_chain","AES-256-XTS" ) ;

				if( StringsAreNotEqual( e->passphrase_h,"" ) ){
					tc_api_task_set( task,"h_passphrase",e->passphrase_h ) ;
				}

				z = e->keyfiles_h ;
				k = e->keyfiles_h_number ;

				for( i = 0 ; i < k ; i++ ){
					tc_api_task_set( task,"h_keyfiles",*( z + i ) ) ;
				}
			}

			r = tc_api_task_do( task ) ;

			tc_api_task_uninit( task ) ;

			if( r == TC_OK ){
				r = _create_file_system( e ) ;
			}
		}

		tc_api_uninit() ;
	}

	return r ;
}

int zuluCryptCreateTCrypt( const char * device,const char * file_system,const char * rng,
			   const char * key,size_t key_len,int key_source,
			   u_int64_t hidden_volume_size,
			   const char * file_system_h,const char * key_h,size_t key_len_h,int key_source_h )
{
	int fd ;
	string_t q = StringVoid ;
	string_t st = StringVoid ;
	string_t xt = StringVoid ;

	const char * keyFile ;
	const char * keyFile_h ;

	int r ;

	create_tcrypt_t tcrypt ;

	tcrypt.device = device ;
	tcrypt.fs = file_system ;
	tcrypt.fs_h = file_system_h ;
	tcrypt.hidden_volume_size = hidden_volume_size ;
	tcrypt.weak_keys_and_salt = StringsAreEqual( rng,"/dev/urandom" ) ;

	if( key_source == TCRYPT_PASSPHRASE ){
		tcrypt.passphrase = key ;
	}else{
		/*
		 * zuluCryptCreateKeyFile() is defined in open_tcrypt.c
		 */
		st = zuluCryptCreateKeyFile( key,key_len,"create_tcrypt-1-" ) ;
		keyFile = StringContent( st ) ;
		tcrypt.keyfiles = &keyFile ;
		tcrypt.keyfiles_number = 1 ;
	}

	if( tcrypt.hidden_volume_size > 0 ){
		if( key_source_h == TCRYPT_PASSPHRASE ){
			tcrypt.passphrase_h = key_h ;
		}else{
			xt = zuluCryptCreateKeyFile( key_h,key_len_h,"create_tcrypt-2-" ) ;
			keyFile_h = StringContent( xt ) ;
			tcrypt.keyfiles_h = &keyFile_h ;
			tcrypt.keyfiles_h_number = 1 ;
		}
	}

	if( StringPrefixEqual( device,"/dev/" ) ){

		r = zuluCryptCreateTCryptVolume( &tcrypt ) ;
	}else{
		/*
		 * zuluCryptAttachLoopDeviceToFile() is defined in create_loop_device.c
		 */
		if( zuluCryptAttachLoopDeviceToFile( device,O_RDWR,&fd,&q ) ){

			tcrypt.device = StringContent( q ) ;
			r = zuluCryptCreateTCryptVolume( &tcrypt ) ;
			StringDelete( &q ) ;
		}else{
			r = 3 ;
		}
	}

	/*
	 * zuluCryptDeleteFile_1() is defined in file_path_security.c
	 */
	if( st != StringVoid ){
		zuluCryptDeleteFile_1( st ) ;
		StringDelete( &st ) ;
	}
	if( xt != StringVoid ){
		zuluCryptDeleteFile_1( xt ) ;
		StringDelete( &xt ) ;
	}

	return r ;
}

#else

/*
 * tcplay < 2.0.0 or tcplay not found
 */
int zuluCryptCreateTCryptVolume( const create_tcrypt_t * e )
{
	if( e ){;}
	return 1 ;
}

int zuluCryptModifyTcryptHeader( const info_t * info )
{
	if( info ){;}
	return 1 ;
}

int zuluCryptCreateTCrypt( const char * device,const char * file_system,const char * rng,
			   const char * key,size_t key_len,int key_source,
			   u_int64_t hidden_volume_size,const char * file_system_h,
			   const char * key_h,size_t key_len_h,int key_source_h )
{
	if( 0 && device && file_system && rng && key_len && key_source && key
		&& hidden_volume_size && key_len_h && key_h && key_source_h && file_system_h ){;}
	return 3 ;
}
#endif