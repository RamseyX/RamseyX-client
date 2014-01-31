#ifndef RX_RAMSEYXHEADER_H
#define RX_RAMSEYXHEADER_H

// Version
#define RX_VER_MAJOR        5
#define RX_VER_MINOR        0
#define RX_VER_PATCHLEVEL   0

// Data
#define RX_N    3U
#define RX_P    67U
#define RX_MN   33U
#define RX_G    2U
#define RX_Q1   3U
#define RX_Q2   3U
#define RX_Q3   6U

#define RX_APPROXIMATE_BLOCK_LENGTH 50000000ULL
#define RX_LAYER1_BLOCKS_PER_TASK   8192U

#define RX_MAX_DAYS 5

// Error codes
#define RX_ERR_SUCCESS                  0x0000
#define RX_ERR_INVALID_ARGUMENTS        0x0001
#define RX_ERR_CONNECTION_FAILED        0x0002
#define RX_ERR_LOGIN_FAILED             0x0003
#define RX_ERR_WRONG_USR_PWD            0x0004
#define RX_ERR_GET_FAILED               0x0005
#define RX_ERR_NO_NEW_TASK              0x0006
#define RX_ERR_UPLOAD_FAILED            0x0007
#define RX_ERR_USER_INFO_UNAVAILABLE    0x0008
#define RX_ERR_USER_ALREADY_EXIST       0x0009
#define RX_ERR_EMAIL_ALREADY_EXIST      0x000a
#define RX_ERR_INVALID_RECOMMENDER      0x000b
#define RX_ERR_SIGNUP_FAILED            0x000c
#define RX_ERR_TASK_OUTDATED            0x000d
#define RX_ERR_TASK_LISTS_FULL          0x000e
#define RX_ERR_NO_COMPLETED_TASK        0x0010

#define RX_TASKINFO_BLOCKNUM    0
#define RX_TASKINFO_W1SIZE      1
#define RX_TASKINFO_BLOCKLENGTH 2

#include <ctime>

struct RXPRINT
{
	unsigned long long id;
	unsigned int layer;
	double progress;
	std::time_t deadline;
};

// Determine OS
#if defined(_WIN64)
    #define RX_OS "win64"
#elif defined(_WIN32)
    #define RX_OS "win32"
#elif defined(__linux__)
    #define RX_OS "linux"
#elif defined(__APPLE__) && defined(__MACH__)
    #define RX_OS "apple"
#else
    #error Unsupported OS
#endif

// Determine compiler
#if defined(__GNUG__) && !(defined(__clang__) || defined(__INTEL_COMPILER))
    /* GNU GCC/G++. --------------------------------------------- */
    #define RX_COMPILER "g++" __VERSION__
#elif defined(_MSC_VER)
    /* Microsoft Visual Studio. --------------------------------- */
    #include <string>
    #define RX_COMPILER "msvc" + std::to_string(_MSC_FULL_VER)
#else
    #error Unsupported compiler
#endif

// Combined build info
#define RX_BUILD (RX_OS "-" RX_COMPILER)

#endif
