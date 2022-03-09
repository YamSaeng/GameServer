/**
@file KISA_SHA_256.h
@brief SHA256 ��ȣ �˰���
@author Copyright (c) 2013 by KISA
@remarks http://seed.kisa.or.kr/
*/

#ifndef SHA256_H
#define SHA256_H

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef OUT
#define OUT
#endif

#ifndef IN
#define IN
#endif

#ifndef INOUT
#define INOUT
#endif

#undef BIG_ENDIAN
#undef LITTLE_ENDIAN

#if defined(USER_BIG_ENDIAN)
	#define BIG_ENDIAN
#elif defined(USER_LITTLE_ENDIAN)
	#define LITTLE_ENDIAN
#else
	#if 0
		#define BIG_ENDIAN
	#elif defined(_MSC_VER)
		#define LITTLE_ENDIAN
	#else
		#error
	#endif
#endif

#define SHA256_DIGEST_BLOCKLEN	64
#define SHA256_DIGEST_VALUELEN	32

typedef struct{
	unsigned int uChainVar[SHA256_DIGEST_VALUELEN / 4];
	unsigned int uHighLength;
	unsigned int uLowLength;
	unsigned int remain_num;
	unsigned char szBuffer[SHA256_DIGEST_BLOCKLEN];
} SHA256_INFO;

/**
@brief ���⺯���� ���̺����� �ʱ�ȭ�ϴ� �Լ�
@param Info : SHA256_Process ȣ�� �� ���Ǵ� ����ü
*/
void SHA256_Init( OUT SHA256_INFO *Info );

/**
@brief ���⺯���� ���̺����� �ʱ�ȭ�ϴ� �Լ�
@param Info : SHA256_Init ȣ���Ͽ� �ʱ�ȭ�� ����ü(���������� ���ȴ�.)
@param pszMessage : ����� �Է� ��
@param inLen : ����� �Է� �� ����
*/
void SHA256_Process( OUT SHA256_INFO *Info, IN const unsigned char*pszMessage, IN unsigned int uDataLen );

/**
@brief �޽��� �����̱�� ���� �����̱⸦ ������ �� ������ �޽��� ����� ������ �����Լ��� ȣ���ϴ� �Լ�
@param Info : SHA256_Init ȣ���Ͽ� �ʱ�ȭ�� ����ü(���������� ���ȴ�.)
@param pszDigest : ��ȣ��
*/
void SHA256_Close( OUT SHA256_INFO *Info, OUT unsigned char*pszDigest );

/**
@brief ����� �Է� ���� �ѹ��� ó��
@param pszMessage : ����� �Է� ��
@param pszDigest : ��ȣ��
@remarks ���������� SHA256_Init, SHA256_Process, SHA256_Close�� ȣ���Ѵ�.
*/
void SHA256_Encrpyt( IN const unsigned char* pszMessage, IN unsigned int uPlainTextLen, OUT unsigned char* pszDigest );

#ifdef  __cplusplus
}
#endif

#endif