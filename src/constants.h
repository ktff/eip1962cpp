#ifndef H_CONSTANTS
#define H_CONSTANTS

#include "types.h"

static const usize BYTES_FOR_LENGTH_ENCODING = 1;

static const usize CURVE_TYPE_LENGTH = 1;
static const u8 BLS12 = 0x01;
static const u8 BN = 0x02;
static const u8 MNT4 = 0x03;
static const u8 MNT6 = 0x04;

static const usize TWIST_TYPE_LENGTH = 1;
static const u8 TWIST_TYPE_M = 0x01;
static const u8 TWIST_TYPE_D = 0x02;

static const usize SIGN_ENCODING_LENGTH = 1;
static const u8 SIGN_PLUS = 0x00;
static const u8 SIGN_MINUS = 0x01;

static const usize EXTENSION_DEGREE_ENCODING_LENGTH = 1;
static const u8 EXTENSION_DEGREE_2 = 0x02;
static const u8 EXTENSION_DEGREE_3 = 0x03;

static const usize OPERATION_ENCODING_LENGTH = 1;

static const u8 OPERATION_G1_ADD = 0x01;
static const u8 OPERATION_G1_MUL = 0x02;
static const u8 OPERATION_G1_MULTIEXP = 0x03;

static const u8 OPERATION_G2_ADD = 0x04;
static const u8 OPERATION_G2_MUL = 0x05;
static const u8 OPERATION_G2_MULTIEXP = 0x06;

static const u8 OPERATION_PAIRING = 0x07;

#endif